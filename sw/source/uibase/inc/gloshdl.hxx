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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSHDL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSHDL_HXX

#include <rtl/ustring.hxx>
#include "swdllapi.h"

class SwWrtShell;
class SwTextBlocks;
class SvxMacro;
class SwGlossaries;
class SfxViewFrame;

class SW_DLLPUBLIC SwGlossaryHdl
{

    SwGlossaries&   rStatGlossaries;
    OUString        aCurGrp;
    SfxViewFrame*   pViewFrame;
    SwWrtShell*     pWrtShell;
    SwTextBlocks*   pCurGrp;

    SAL_DLLPRIVATE bool  Expand( const OUString& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary );

public:
    void        GlossaryDlg();

    size_t      GetGroupCnt() const;
    OUString    GetGroupName( size_t, OUString* pTitle = nullptr );
    bool        NewGroup(OUString & rGroupName, const OUString& rTitle);
    bool        DelGroup(const OUString &);
    bool        RenameGroup(const OUString& rOld, OUString& rNew, const OUString& rNewTitle);
    void        SetCurGroup(const OUString &aGrp, bool bApi = false, bool bAlwaysCreateNew = false);

    sal_uInt16  GetGlossaryCnt();
    OUString    GetGlossaryName(sal_uInt16);
    OUString    GetGlossaryShortName(const OUString &rName);
    OUString    GetGlossaryShortName(sal_uInt16);

    bool    Rename( const OUString& rOldShortName, const OUString& rNewShortName,
                        const OUString& rNewName);
    bool    CopyOrMove( const OUString& rSourceGroupName, OUString& rSourceShortName,
                        const OUString& rDestGroupName, const OUString& rLongName, bool bMove );
    bool    HasShortName(const OUString &rShortName) const;
    // when NewGlossary is called from Basic then the previously set group should
    // be newly created if applicable.
    bool    NewGlossary(const OUString &rName, const OUString &rShortName,
                bool bApiCall = false, bool bNoAttr = false );
    bool    DelGlossary(const OUString&);
    bool    CopyToClipboard(SwWrtShell& rSh, const OUString& rShortName);

    bool    ExpandGlossary();
    bool    InsertGlossary(const OUString &rName);

    void    SetMacros(const OUString& rName,
                      const SvxMacro* pStart,
                      const SvxMacro* pEnd,
                      SwTextBlocks *pGlossary = nullptr );
    void    GetMacros(const OUString& rShortName,
                      SvxMacro& rStart,
                      SvxMacro& rEnd,
                      SwTextBlocks* pGlossary = nullptr );

    bool    IsReadOnly( const OUString* = nullptr ) const;
    bool    IsOld() const;

    bool    FindGroupName(OUString& rGroup); // find group without path index

    bool    ImportGlossaries( const OUString& rName );

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_GLOSHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
