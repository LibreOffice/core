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

#include "XMLColumnRowGroupExport.hxx"
#include "xmlexprt.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <algorithm>

using namespace xmloff::token;

ScMyColumnRowGroup::ScMyColumnRowGroup()
    : nField(0)
    , nLevel(0)
    , bDisplay(false)
{
}

bool ScMyColumnRowGroup::operator<(const ScMyColumnRowGroup& rGroup) const
{
    if (rGroup.nField > nField)
        return true;
    else
        if (rGroup.nField == nField && rGroup.nLevel > nLevel)
            return true;
        else
            return false;
}

ScMyOpenCloseColumnRowGroup::ScMyOpenCloseColumnRowGroup(ScXMLExport& rTempExport, sal_uInt32 nToken)
    : rExport(rTempExport),
    rName(rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XMLTokenEnum(nToken)))),
    aTableStart(),
    aTableEnd()
{
}

ScMyOpenCloseColumnRowGroup::~ScMyOpenCloseColumnRowGroup()
{
}

void ScMyOpenCloseColumnRowGroup::NewTable()
{
    aTableStart.clear();
    aTableEnd.clear();
}

void ScMyOpenCloseColumnRowGroup::AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField)
{
    aTableStart.push_back(aGroup);
    aTableEnd.push_back(nEndField);
}

bool ScMyOpenCloseColumnRowGroup::IsGroupStart(const sal_Int32 nField)
{
    bool bGroupStart(false);
    if (!aTableStart.empty())
    {
        //  when used to find repeated rows at the beginning of a group,
        //  aTableStart may contain entries before nField. They must be skipped here
        //  (they will be used for OpenGroups later in the right order).

        ScMyColumnRowGroupVec::iterator aItr = std::find_if_not(aTableStart.begin(), aTableStart.end(),
            [&nField](const ScMyColumnRowGroup& rGroup) { return rGroup.nField < nField; });
        bGroupStart = (aItr != aTableStart.end()) && (aItr->nField == nField);
    }
    return bGroupStart;
}

void ScMyOpenCloseColumnRowGroup::OpenGroup(const ScMyColumnRowGroup& rGroup)
{
    if (!rGroup.bDisplay)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, XML_FALSE);
    rExport.StartElement( rName, true);
}

void ScMyOpenCloseColumnRowGroup::OpenGroups(const sal_Int32 nField)
{
    ScMyColumnRowGroupVec::iterator aItr(aTableStart.begin());
    ScMyColumnRowGroupVec::iterator aEndItr(aTableStart.end());
    bool bReady(false);
    while(!bReady && aItr != aEndItr)
    {
        if (aItr->nField == nField)
        {
            OpenGroup(*aItr);
            aItr = aTableStart.erase(aItr);
        }
        else
            bReady = true;
    }
}

bool ScMyOpenCloseColumnRowGroup::IsGroupEnd(const sal_Int32 nField)
{
    return (!aTableEnd.empty()) && (aTableEnd.front() == nField);
}

void ScMyOpenCloseColumnRowGroup::CloseGroups(const sal_Int32 nField)
{
    ScMyFieldGroupVec::iterator aItr(aTableEnd.begin());
    bool bReady(false);
    while(!bReady && aItr != aTableEnd.end())
    {
        if (*aItr == nField)
        {
            rExport.EndElement( rName, true );
            aItr = aTableEnd.erase(aItr);
        }
        else
            bReady = true;
    }
}

sal_Int32 ScMyOpenCloseColumnRowGroup::GetLast()
{
    sal_Int32 maximum(-1);
    if (!aTableEnd.empty())
    {
        ScMyFieldGroupVec::iterator i(std::max_element(aTableEnd.begin(), aTableEnd.end()));
        if (*i > maximum)
            maximum = *i;
    }
    return maximum;
}

void ScMyOpenCloseColumnRowGroup::Sort()
{
    aTableStart.sort();
    std::sort(aTableEnd.begin(), aTableEnd.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
