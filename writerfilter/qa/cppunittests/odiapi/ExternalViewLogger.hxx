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

#ifndef INCLUDED_EXTERNALVIEWLOGGER_HXX
#define INCLUDED_EXTERNALVIEWLOGGER_HXX

#include <odiapi/props/Logger.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
