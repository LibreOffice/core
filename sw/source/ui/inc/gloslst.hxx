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

#ifndef _GLOSLST_HXX
#define _GLOSLST_HXX


#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>

class SwGlossaries;
class vector;

struct AutoTextGroup
{
    sal_uInt16  nCount;
    OUString    sName;
    OUString    sTitle;
    OUString    sLongNames;   // by 0x0A separated long names
    OUString    sShortNames;  // by 0x0A separated short names
    DateTime    aDateModified;

    AutoTextGroup() : aDateModified( DateTime::EMPTY ) {}
};


typedef std::vector<AutoTextGroup*> AutoTextGroups;

class SwGlossaryList : public AutoTimer
{
    AutoTextGroups  aGroupArr;
    OUString        sPath;
    bool            bFilled;

    AutoTextGroup*  FindGroup(const OUString& rGroupName);
    void            FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGloss);

public:
        SwGlossaryList();
        ~SwGlossaryList();

    bool HasLongName(const OUString& rBegin, std::vector<OUString> *pLongNames);
    sal_Bool            GetShortName(const String& rLongName,
                                        String& rShortName, String& rGroupName );

    sal_uInt16      GetGroupCount();
    OUString        GetGroupName(sal_uInt16 nPos, bool bNoPath = true);
    OUString        GetGroupTitle(sal_uInt16 nPos);

    sal_uInt16      GetBlockCount(sal_uInt16 nGroup);
    OUString        GetBlockLongName(sal_uInt16 nGroup, sal_uInt16 nBlock);
    OUString        GetBlockShortName(sal_uInt16 nGroup, sal_uInt16 nBlock);

    void            Update();

    virtual void    Timeout();

    void            ClearGroups();
};

#endif






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
