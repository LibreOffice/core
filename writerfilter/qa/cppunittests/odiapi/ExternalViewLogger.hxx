/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_EXTERNALVIEWLOGGER_HXX
#define INCLUDED_EXTERNALVIEWLOGGER_HXX

#ifndef INCLUDED_LOGGER_HXX
#include <odiapi/props/Logger.hxx>
#endif

#include <fstream>
#include <stack>
#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>

namespace util {
 
  struct NodeDescription
  {
    typedef boost::shared_ptr<NodeDescription> Pointer_t;
    
    NodeDescription(const std::string& parent, const std::string& refersTo, const std::string& value, bool inUse);

    std::string mParentNodeId;
    std::string mRefersToNodeId;
    std::string mNodeValue;
    bool mInUse;
  };
  
  /** A file logger
   */
  class ExternalViewLoggerImpl : public Logger
  {
  public:
    ExternalViewLoggerImpl(const std::string& fileName);
  
    virtual void beginTree();
    virtual void endTree();

    virtual void beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse);
    virtual void endNode(const std::string& nodeId);

  private:
    bool isLeaf(const std::string& nodeId);
    bool isUnreferencedLeaf(const std::string& nodeId);
    bool isReferenced(const std::string& nodeId);
    bool isReferingToOtherNode(const std::string& nodeId);
    bool hasParent(const std::string& nodeId);
    void dumpTree(const std::string& nodeId);
    std::string getValue(const std::string& nodeId);
    std::string getNewStyleName();
    void dumpNodeContainer(const std::string& fileName);

  private:
    typedef std::map<std::string, NodeDescription::Pointer_t> NodeContainer_t;
    
    std::string mFileName;
    NodeContainer_t mNodeContainer;
    std::ofstream mFile;  
    std::stack<std::string> mParentNodeStack;
  };

} // namespace util

#endif // INCLUDED_LOGGER_HXX
