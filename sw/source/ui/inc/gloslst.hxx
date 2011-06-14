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
    sal_uInt16      nCount;
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
    sal_Bool            bFilled;

    AutoTextGroup*  FindGroup(const String& rGroupName);
    void            FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGloss);

public:
        SwGlossaryList();
        ~SwGlossaryList();

    sal_Bool            HasLongName(const String& rBegin, SvStringsISortDtor* pLongNames );
    sal_Bool            GetShortName(const String& rLongName,
                                        String& rShortName, String& rGroupName );

    sal_uInt16          GetGroupCount();
    String          GetGroupName(sal_uInt16 nPos, sal_Bool bNoPath = sal_True, String* pTitle = 0);
    sal_uInt16          GetBlockCount(sal_uInt16 nGroup);
    String          GetBlockName(sal_uInt16 nGroup, sal_uInt16 nBlock, String& rShortName);

    void            Update();

    virtual void    Timeout();

    void            ClearGroups();
};

#endif






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
