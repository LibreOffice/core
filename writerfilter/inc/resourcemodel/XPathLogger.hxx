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
#ifndef INCLUDED_XPATH_LOGGER_HXX
#define INCLUDED_XPATH_LOGGER_HXX

#include <hash_map>
#include <stack>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <WriterFilterDllApi.hxx>

namespace writerfilter
{
using ::std::hash_map;
using ::std::stack;
using ::std::string;
using ::std::vector;

class WRITERFILTER_DLLPUBLIC XPathLogger
{
    typedef hash_map<string, unsigned int> TokenMap_t;
    typedef boost::shared_ptr<TokenMap_t> TokenMapPointer_t;

    TokenMapPointer_t mp_tokenMap;
    stack<TokenMapPointer_t> m_tokenMapStack;
    vector<string> m_path;
    string m_currentPath;

    void updateCurrentPath();

public:
    explicit XPathLogger();
    virtual ~XPathLogger();

    string getXPath() const;
    void startElement(string _token);
    void endElement();
};
}
#endif // INCLUDED_XPATH_LOGGER_HXX
