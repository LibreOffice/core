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
#ifndef _GLOSHDL_HXX
#define _GLOSHDL_HXX

#include <tools/string.hxx>
#include "swdllapi.h"

class SwWrtShell;
class SwTextBlocks;
class SvxMacro;
class SwGlossaries;
class SfxViewFrame;

// CLASS -----------------------------------------------------------------
class SW_DLLPUBLIC SwGlossaryHdl
{

    SwGlossaries&   rStatGlossaries;
    String          aCurGrp;
    SfxViewFrame*   pViewFrame;
    SwWrtShell*     pWrtShell;
    SwTextBlocks*   pCurGrp;

    SW_DLLPRIVATE void  _SetMacros(const String &rName,
                       const SvxMacro *pStart,
                       const SvxMacro *pEnd);

    SW_DLLPRIVATE BOOL  Expand( const String& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary );

public:
    BOOL    ConvertToNew(SwTextBlocks& rOld);
    void    GlossaryDlg();

    USHORT  GetGroupCnt() const;
    String  GetGroupName( USHORT, String* pTitle = 0 );
    BOOL    NewGroup(String & rGroupName, const String& rTitle);
    BOOL    DelGroup(const String &);
    BOOL    RenameGroup(const String & rOld, String& rNew, const String& rNewTitle);
    void    SetCurGroup(const String &aGrp, BOOL bApi = FALSE, BOOL bAlwaysCreateNew = FALSE);

    const String &GetCurGroup() const { return aCurGrp; }

    USHORT  GetGlossaryCnt();
    String  GetGlossaryName(USHORT);
    String  GetGlossaryShortName(const String &rName);
    String  GetGlossaryShortName(USHORT);

    BOOL    Rename( const String& rOldShortName,  const String& rNewShortName,
                        const String& rNewName);
    BOOL    CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, BOOL bMove );
    BOOL    HasShortName(const String &rShortName) const;
    //wird NewGlossary vom Basic gerufen, dann soll dir zuvor eingestellte
    //Gruppe ggf. neu erstellt werden
    BOOL    NewGlossary(const String &rName, const String &rShortName,
                BOOL bApiCall = FALSE, BOOL bNoAttr = FALSE );
    BOOL    DelGlossary(const String&);
    BOOL    CopyToClipboard(SwWrtShell& rSh, const String& rShortName);

    BOOL    ExpandGlossary();
    BOOL    InsertGlossary(const String &rName);

    void    SetMacros(const String& rName,
                      const SvxMacro* pStart,
                      const SvxMacro* pEnd,
                      SwTextBlocks *pGlossary = 0 );
    void    GetMacros(const String& rShortName,
                      SvxMacro& rStart,
                      SvxMacro& rEnd,
                      SwTextBlocks* pGlossary = 0 );

    BOOL    IsReadOnly( const String* = 0 ) const;
    BOOL    IsOld() const;

    BOOL    FindGroupName(String & rGroup); // Gruppe ohne Pfadindex finden

    BOOL    ImportGlossaries( const String& rName );

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // _GLOSHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
