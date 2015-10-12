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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSLST_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSLST_HXX

#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
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

    AutoTextGroup()
        : nCount(0)
        , aDateModified(DateTime::EMPTY)
    {
    }
};

class SwGlossaryList : public AutoTimer
{
    std::vector<AutoTextGroup*> aGroupArr;
    OUString        sPath;
    bool            bFilled;

    AutoTextGroup*  FindGroup(const OUString& rGroupName);
    static void     FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGloss);

public:
        SwGlossaryList();
        virtual ~SwGlossaryList();

    bool            HasLongName(const OUString& rBegin, std::vector<OUString> *pLongNames);
    bool            GetShortName(const OUString& rLongName,
                                       OUString& rShortName, OUString& rGroupName );

    size_t          GetGroupCount();
    OUString        GetGroupName(size_t nPos, bool bNoPath = true);
    OUString        GetGroupTitle(size_t nPos);

    sal_uInt16      GetBlockCount(size_t nGroup);
    OUString        GetBlockLongName(size_t nGroup, sal_uInt16 nBlock);
    OUString        GetBlockShortName(size_t nGroup, sal_uInt16 nBlock);

    void            Update();

    virtual void    Invoke() override;

    void            ClearGroups();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
