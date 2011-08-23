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

#ifndef INCLUDED_FILELOGGERIMPL_HXX
#define INCLUDED_FILELOGGERIMPL_HXX

#include <odiapi/props/Logger.hxx>
#include <fstream>
#include <stack>

namespace util {

/** A file logger
 */
class FileLoggerImpl : public util::Logger
{
public:
    FileLoggerImpl(const std::string& fileName);
  
    virtual void beginTree();
    virtual void endTree();

    virtual void beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse);
    virtual void endNode(const std::string& nodeId);

private:
    std::ofstream file_;  
    std::stack<std::string> nodeStack_;
};

} // namespace util

#endif // INCLUDED_LOGGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
