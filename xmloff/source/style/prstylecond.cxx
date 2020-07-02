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

#include <rtl/ustring.hxx>
#include <prstylecond.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::xmloff::token;

// note: keep this in sync with the list of conditions in sw/source/uibase/chrdlg/ccoll.cxx

namespace {

struct ConditionMap
{
        char const* aInternal;
        XMLTokenEnum nExternal;
        int          aValue;
};

}

const ConditionMap g_ConditionMap[] =
{
    { "TableHeader",            XML_TABLE_HEADER,   -1 },
    { "Table",                  XML_TABLE,          -1 },
    { "Frame",                  XML_TEXT_BOX,       -1 }, // FIXME - Not in ODF spec
    { "Section",                XML_SECTION,        -1 },
    { "Footnote",               XML_FOOTNOTE,       -1 },
    { "Endnote",                XML_ENDNOTE,        -1 },
    { "Header",                 XML_HEADER,         -1 },
    { "Footer",                 XML_FOOTER,         -1 },
    { "OutlineLevel1",          XML_OUTLINE_LEVEL,   1 },
    { "OutlineLevel2",          XML_OUTLINE_LEVEL,   2 },
    { "OutlineLevel3",          XML_OUTLINE_LEVEL,   3 },
    { "OutlineLevel4",          XML_OUTLINE_LEVEL,   4 },
    { "OutlineLevel5",          XML_OUTLINE_LEVEL,   5 },
    { "OutlineLevel6",          XML_OUTLINE_LEVEL,   6 },
    { "OutlineLevel7",          XML_OUTLINE_LEVEL,   7 },
    { "OutlineLevel8",          XML_OUTLINE_LEVEL,   8 },
    { "OutlineLevel9",          XML_OUTLINE_LEVEL,   9 },
    { "OutlineLevel10",         XML_OUTLINE_LEVEL,  10 },
    { "NumberingLevel1",        XML_LIST_LEVEL,      1 },
    { "NumberingLevel2",        XML_LIST_LEVEL,      2 },
    { "NumberingLevel3",        XML_LIST_LEVEL,      3 },
    { "NumberingLevel4",        XML_LIST_LEVEL,      4 },
    { "NumberingLevel5",        XML_LIST_LEVEL,      5 },
    { "NumberingLevel6",        XML_LIST_LEVEL,      6 },
    { "NumberingLevel7",        XML_LIST_LEVEL,      7 },
    { "NumberingLevel8",        XML_LIST_LEVEL,      8 },
    { "NumberingLevel9",        XML_LIST_LEVEL,      9 },
    { "NumberingLevel10",       XML_LIST_LEVEL,     10 }
};

OUString GetParaStyleCondExternal( OUString const &internal)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(g_ConditionMap); ++i)
    {
        if (internal.compareToAscii( g_ConditionMap[i].aInternal ) == 0)
        {
            OUString aResult = GetXMLToken( g_ConditionMap[i].nExternal ) +
                    "()";
            if (g_ConditionMap[i].aValue != -1)
            {
                aResult += "=" +
                    OUString::number( g_ConditionMap[i].aValue );
            }
            return aResult;
        }
    }
    assert(!"GetParaStyleCondExternal: model has unknown style condition");
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
