/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ExternalViewLogger.hxx"
#include <iostream>
#include <boost/assert.hpp>
#include <stdio.h>

#ifdef WNT
    #define SNPRINTF(buffer, size, format, args) _snprintf(buffer, size, format, args)
#else
    #define SNPRINTF(buffer, size, format, args) snprintf(buffer, size, format, args)
#endif

using namespace std;

namespace util
{

  NodeDescription::NodeDescription(const string& parent, const string& refersTo, const string& value, bool inUse) :
    mParentNodeId(parent),
    mRefersToNodeId(refersTo),
    mNodeValue(value),
    mInUse(inUse)
  {}

  ExternalViewLoggerImpl::ExternalViewLoggerImpl(const string& fileName) :
    mFileName(fileName),
    mFile(fileName.c_str())
  {
    if (!mFile)
      throw "Cannot open file";
  }

  string ExternalViewLoggerImpl::getNewStyleName()
  {
    static int i = 0;
    char buff[20];
    SNPRINTF(buff, sizeof(buff), "Style_%d", i++);
    return string(buff);
  }

  void ExternalViewLoggerImpl::beginTree()
  {
    mParentNodeStack.push("");
  }

  void ExternalViewLoggerImpl::dumpNodeContainer(const std::string& fileName)
  {
    std::ofstream file(fileName.c_str());
    NodeContainer_t::iterator iter = mNodeContainer.begin();
    NodeContainer_t::iterator iter_end = mNodeContainer.end();
    for (; iter != iter_end; ++iter)
    {
        file << iter->first << string(" ") << iter->second->mParentNodeId << string(" ") << iter->second->mRefersToNodeId << string(" ") << iter->second->mNodeValue << endl;
    }
  }

  void ExternalViewLoggerImpl::endTree()
  {
    mFile << "digraph {" << endl;
    mFile << "Root [shape=box, color=grey];" << endl;

    while (!mParentNodeStack.empty())
      mParentNodeStack.pop();

    mParentNodeStack.push("Root");

    NodeContainer_t::iterator iter = mNodeContainer.begin();
    NodeContainer_t::iterator iter_end = mNodeContainer.end();
    for (; iter != iter_end; ++iter)
    {
      if (isUnreferencedLeaf(iter->first))
      {
        string newStyleName = getNewStyleName();
        mFile << newStyleName << " [shape=box];" << endl;
        mFile << mParentNodeStack.top() << " -> " << newStyleName << endl;
        mParentNodeStack.push(newStyleName);
        dumpTree(iter->first);
        mParentNodeStack.pop();
      }
    }

    mFile << "}" << endl;
  }

  void ExternalViewLoggerImpl::beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse)
  {
    mNodeContainer.insert(
        NodeContainer_t::value_type(nodeId,
        NodeDescription::Pointer_t(new NodeDescription(mParentNodeStack.top(), refersToNodeId, value, inUse))));
    mParentNodeStack.push(nodeId);
  }

  void ExternalViewLoggerImpl::endNode(const std::string& nodeId)
  {
    mParentNodeStack.pop();
  }

  bool ExternalViewLoggerImpl::isLeaf(const std::string& nodeId)
  {
    bool isLeaf = true;

    NodeContainer_t::const_iterator iter = mNodeContainer.begin();
    NodeContainer_t::const_iterator iter_end = mNodeContainer.end();
    for (; iter != iter_end; ++iter)
    {
      if (iter->second->mParentNodeId == nodeId)
      {
        isLeaf = false;
        break;
      }
    }
    return isLeaf;
  }

  bool ExternalViewLoggerImpl::isUnreferencedLeaf(const string& nodeId)
  {
    return isLeaf(nodeId) && !isReferenced(nodeId);
  }

  bool ExternalViewLoggerImpl::isReferenced(const string& nodeId)
  {
    bool isReferenced = false;

    NodeContainer_t::const_iterator iter = mNodeContainer.begin();
    NodeContainer_t::const_iterator iter_end = mNodeContainer.end();
    for (; iter != iter_end; ++iter)
    {
      if (iter->second->mRefersToNodeId == nodeId)
      {
        isReferenced = true;
        break;
      }
    }
    return isReferenced;
  }

  bool ExternalViewLoggerImpl::isReferingToOtherNode(const string& nodeId)
  {
    NodeContainer_t::const_iterator iter = mNodeContainer.find(nodeId);
    BOOST_ASSERT(iter != mNodeContainer.end());
    return !iter->second->mRefersToNodeId.empty();
  }

  bool ExternalViewLoggerImpl::hasParent(const string& nodeId)
  {
    NodeContainer_t::const_iterator iter = mNodeContainer.find(nodeId);
    BOOST_ASSERT(iter != mNodeContainer.end());
    return iter->second->mParentNodeId != "Root" && iter->second->mParentNodeId != "";
  }

  string ExternalViewLoggerImpl::getValue(const string& nodeId)
  {
    return mNodeContainer.find(nodeId)->second->mNodeValue;
  }

  void ExternalViewLoggerImpl::dumpTree(const string& nodeId)
  {
    if (nodeId != "Root")
    {
      mFile << nodeId << " [label=\"(" << getValue(nodeId) << ")\",shape=box];" << endl;
      mFile << mParentNodeStack.top() << " -> " << nodeId << ";" << endl;
      if (isReferingToOtherNode(nodeId))
      {
        mParentNodeStack.push(nodeId);
        dumpTree(mNodeContainer.find(nodeId)->second->mRefersToNodeId);
        mParentNodeStack.pop();
      }
    }

    if (hasParent(nodeId))
      dumpTree(mNodeContainer.find(nodeId)->second->mParentNodeId);
  }

} // namespace util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
