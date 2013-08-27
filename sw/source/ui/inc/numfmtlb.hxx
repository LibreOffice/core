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
#ifndef _SWNUMFMTLB_HXX
#define _SWNUMFMTLB_HXX

#include <vcl/lstbox.hxx>
#include <svl/zforlist.hxx>
#include "swdllapi.h"

class SwView;

class SW_DLLPUBLIC NumFormatListBox : public ListBox
{
    short               nCurrFormatType;
    sal_uInt16              nStdEntry;
    sal_Bool                bOneArea;
    sal_uLong               nDefFormat;
    SwView*             pVw;
    SvNumberFormatter*  pOwnFormatter;
    LanguageType        eCurLanguage;
    sal_Bool                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    sal_Bool                bUseAutomaticLanguage;//determine whether language is automatically assigned

    SW_DLLPRIVATE DECL_LINK( SelectHdl, ListBox * );

    SW_DLLPRIVATE double          GetDefValue(const short nFormatType) const;
    SW_DLLPRIVATE void            Init(short nFormatType, sal_Bool bUsrFmts);
    SW_DLLPRIVATE SwView*           GetView();

public:
    NumFormatListBox(Window* pWin, WinBits nStyle);

    NumFormatListBox( Window* pWin, SwView* pView, const ResId& rResId,
        short nFormatType = NUMBERFORMAT_NUMBER, sal_uLong nDefFmt = 0,
        sal_Bool bUsrFmts = sal_True );

    ~NumFormatListBox();

    void            Clear();

    inline void     SetOneArea(sal_Bool bOnlyOne = sal_True) { bOneArea = bOnlyOne; }

    void            SetFormatType(const short nFormatType);
    inline short    GetFormatType() const { return nCurrFormatType; }
    void            SetDefFormat(const sal_uLong nDefFmt);
    sal_uLong           GetFormat() const;

    inline LanguageType GetCurLanguage() const { return eCurLanguage;}
    void                SetLanguage(LanguageType eSet)  { eCurLanguage = eSet;}

    void            SetAutomaticLanguage(sal_Bool bSet){bUseAutomaticLanguage = bSet;}
    sal_Bool            IsAutomaticLanguage()const {return bUseAutomaticLanguage;}

    void            SetShowLanguageControl(sal_Bool bSet){bShowLanguageControl = bSet;}

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
