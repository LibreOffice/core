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

    SW_DLLPRIVATE sal_Bool  Expand( const String& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary );

public:
    sal_Bool    ConvertToNew(SwTextBlocks& rOld);
    void    GlossaryDlg();

    sal_uInt16  GetGroupCnt() const;
    String  GetGroupName( sal_uInt16, String* pTitle = 0 );
    sal_Bool    NewGroup(String & rGroupName, const String& rTitle);
    sal_Bool    DelGroup(const String &);
    sal_Bool    RenameGroup(const String & rOld, String& rNew, const String& rNewTitle);
    void    SetCurGroup(const String &aGrp, sal_Bool bApi = sal_False, sal_Bool bAlwaysCreateNew = sal_False);

    const String &GetCurGroup() const { return aCurGrp; }

    sal_uInt16  GetGlossaryCnt();
    String  GetGlossaryName(sal_uInt16);
    String  GetGlossaryShortName(const String &rName);
    String  GetGlossaryShortName(sal_uInt16);

    sal_Bool    Rename( const String& rOldShortName,  const String& rNewShortName,
                        const String& rNewName);
    sal_Bool    CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, sal_Bool bMove );
    sal_Bool    HasShortName(const String &rShortName) const;
    // when NewGlossary is called from Basic then the previously set group should
    // be newly created if applicable.
    sal_Bool    NewGlossary(const String &rName, const String &rShortName,
                sal_Bool bApiCall = sal_False, sal_Bool bNoAttr = sal_False );
    sal_Bool    DelGlossary(const String&);
    sal_Bool    CopyToClipboard(SwWrtShell& rSh, const String& rShortName);

    sal_Bool    ExpandGlossary();
    bool    InsertGlossary(const String &rName);

    void    SetMacros(const String& rName,
                      const SvxMacro* pStart,
                      const SvxMacro* pEnd,
                      SwTextBlocks *pGlossary = 0 );
    void    GetMacros(const String& rShortName,
                      SvxMacro& rStart,
                      SvxMacro& rEnd,
                      SwTextBlocks* pGlossary = 0 );

    sal_Bool    IsReadOnly( const String* = 0 ) const;
    sal_Bool    IsOld() const;

    sal_Bool    FindGroupName(String & rGroup); // find group without path index

    sal_Bool    ImportGlossaries( const String& rName );

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // _GLOSHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
