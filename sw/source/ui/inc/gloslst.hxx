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

#ifndef _GLOSLST_HXX
#define _GLOSLST_HXX


#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>
#include <svl/svarray.hxx>

class SwGlossaries;
class SvStringsISortDtor;

struct AutoTextGroup
{
    USHORT      nCount;
    String      sName;
    String      sTitle;
    String      sLongNames;   // durch 0x0A getrennte Langnamen
    String      sShortNames;  // durch 0x0A getrennte Kurznamen
    DateTime    aDateModified;
};


typedef AutoTextGroup* AutoTextGroupPtr;
SV_DECL_PTRARR(AutoTextGroups, AutoTextGroupPtr, 4, 4)

class SwGlossaryList : public AutoTimer
{
    AutoTextGroups  aGroupArr;
    String          sPath;
    BOOL            bFilled;

    AutoTextGroup*  FindGroup(const String& rGroupName);
    void            FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGloss);

public:
        SwGlossaryList();
        ~SwGlossaryList();

    BOOL            HasLongName(const String& rBegin, SvStringsISortDtor* pLongNames );
    BOOL            GetShortName(const String& rLongName,
                                        String& rShortName, String& rGroupName );

    USHORT          GetGroupCount();
    String          GetGroupName(USHORT nPos, BOOL bNoPath = TRUE, String* pTitle = 0);
    USHORT          GetBlockCount(USHORT nGroup);
    String          GetBlockName(USHORT nGroup, USHORT nBlock, String& rShortName);

    void            Update();

    virtual void    Timeout();

    void            ClearGroups();
};

#endif






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
