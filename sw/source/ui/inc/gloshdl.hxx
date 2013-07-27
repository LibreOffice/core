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
#ifndef _GLOSHDL_HXX
#define _GLOSHDL_HXX

#include <rtl/ustring.hxx>
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
    OUString        aCurGrp;
    SfxViewFrame*   pViewFrame;
    SwWrtShell*     pWrtShell;
    SwTextBlocks*   pCurGrp;

    SW_DLLPRIVATE void  _SetMacros(const OUString &rName,
                       const SvxMacro *pStart,
                       const SvxMacro *pEnd);

    SW_DLLPRIVATE sal_Bool  Expand( const OUString& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary );

public:
    sal_Bool    ConvertToNew(SwTextBlocks& rOld);
    void    GlossaryDlg();

    sal_uInt16  GetGroupCnt() const;
    OUString    GetGroupName( sal_uInt16, OUString* pTitle = 0 );
    sal_Bool    NewGroup(OUString & rGroupName, const OUString& rTitle);
    sal_Bool    DelGroup(const OUString &);
    sal_Bool    RenameGroup(const OUString& rOld, OUString& rNew, const OUString& rNewTitle);
    void        SetCurGroup(const OUString &aGrp, sal_Bool bApi = sal_False, sal_Bool bAlwaysCreateNew = sal_False);

    OUString GetCurGroup() const { return aCurGrp; }

    sal_uInt16  GetGlossaryCnt();
    OUString  GetGlossaryName(sal_uInt16);
    OUString  GetGlossaryShortName(const OUString &rName);
    OUString  GetGlossaryShortName(sal_uInt16);

    sal_Bool    Rename( const OUString& rOldShortName, const OUString& rNewShortName,
                        const OUString& rNewName);
    sal_Bool    CopyOrMove( const OUString& rSourceGroupName, OUString& rSourceShortName,
                        const OUString& rDestGroupName, const OUString& rLongName, sal_Bool bMove );
    sal_Bool    HasShortName(const OUString &rShortName) const;
    // when NewGlossary is called from Basic then the previously set group should
    // be newly created if applicable.
    sal_Bool    NewGlossary(const OUString &rName, const OUString &rShortName,
                sal_Bool bApiCall = sal_False, sal_Bool bNoAttr = sal_False );
    sal_Bool    DelGlossary(const OUString&);
    sal_Bool    CopyToClipboard(SwWrtShell& rSh, const OUString& rShortName);

    sal_Bool    ExpandGlossary();
    bool    InsertGlossary(const OUString &rName);

    void    SetMacros(const OUString& rName,
                      const SvxMacro* pStart,
                      const SvxMacro* pEnd,
                      SwTextBlocks *pGlossary = 0 );
    void    GetMacros(const OUString& rShortName,
                      SvxMacro& rStart,
                      SvxMacro& rEnd,
                      SwTextBlocks* pGlossary = 0 );

    sal_Bool    IsReadOnly( const OUString* = 0 ) const;
    sal_Bool    IsOld() const;

    sal_Bool    FindGroupName(OUString& rGroup); // find group without path index

    sal_Bool    ImportGlossaries( const OUString& rName );

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // _GLOSHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
