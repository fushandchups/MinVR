/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#ifndef VRCONSOLENODE_H_
#define VRCONSOLENODE_H_

#include "display/VRDisplayNode.h"
#include <iostream>

namespace MinVR {

class VRConsoleNode : public VRDisplayNode {
public:
	VRConsoleNode(const std::string &name, std::ostream *stream = &std::cout);
	virtual ~VRConsoleNode();

	virtual std::string getType() { return "VRConsoleNode"; }

	void render(VRDataIndex *renderState, VRRenderHandler* renderHandler);
	void displayTheFinishedRendering(VRDataIndex *renderState);

	/// Overridden here in order to generate an error - a console node cannot have children.
	void addChild(VRDisplayNode* child);

	void println(const std::string &output);

private:
	std::ostream* m_stream;
};


/** Small factory for creating this specific type of display node.  To be registered as a 
    "sub-factory" with the main VRFactory. 
 */
class VRConsoleNodeFactory : public VRDisplayNodeFactory {
public:
	virtual ~VRConsoleNodeFactory() {}

	VRDisplayNode* create(VRMain *vrMain, VRDataIndex *config, const std::string &valName, const std::string &nameSpace);
};

} /* namespace MinVR */

#endif /* VRCONSOLENODE_H_ */