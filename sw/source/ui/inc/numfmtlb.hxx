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
    NumFormatListBox( Window* pWin, const ResId& rResId,
        short nFormatType = NUMBERFORMAT_NUMBER, sal_uLong nDefFmt = 0,
        sal_Bool bUsrFmts = sal_True );

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
