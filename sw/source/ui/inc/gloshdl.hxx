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
