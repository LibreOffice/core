/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "FileLoggerImpl.hxx"
#include <iostream>

using namespace std;

namespace util
{

    FileLoggerImpl::FileLoggerImpl(const string& fileName) :
        file_(fileName.c_str())
    {
        if (!file_)
            throw "Cannot open file";
    }

    void FileLoggerImpl::beginTree()
    {
        file_ << "digraph {" << endl;
    }

    void FileLoggerImpl::endTree()
    {
        file_ << "}" << endl;
    }

    void FileLoggerImpl::beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse)
    {
        if (!nodeStack_.empty())
        {
          if (inUse)
            file_ << nodeId << " [ label=\"(" << value << ")\", shape=box, color=grey, style=filled ];"<< endl;
          else
            file_ << nodeId << " [ label=\"(" << value << ")\" ];"<< endl;

          file_ << nodeStack_.top() << " -> " << nodeId << ";" << endl;

          if (!refersToNodeId.empty())
            file_ << nodeId << " -> " << refersToNodeId << " [ color=grey, weight=0 ];" << endl;
        }
        else
        {
            file_ << nodeId << " [ label=\"(" << value << ")\", shape=diamond ];"<< endl;            
        }
        nodeStack_.push(nodeId);
    }

    void FileLoggerImpl::endNode(const std::string& nodeId)
    {
        nodeStack_.pop();
    }

} // namespace util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
