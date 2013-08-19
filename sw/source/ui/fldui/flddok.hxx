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
#ifndef _SWFLDDOK_HXX
#define _SWFLDDOK_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>

#include "numfmtlb.hxx"
#include "fldpage.hxx"

class SwFldDokPage : public SwFldPage
{
    ListBox*            m_pTypeLB;
    VclContainer*       m_pSelection;
    ListBox*            m_pSelectionLB;
    FixedText*          m_pValueFT;
    Edit*               m_pValueED;
    FixedText*          m_pLevelFT;
    NumericField*       m_pLevelED;
    FixedText*          m_pDateFT;
    FixedText*          m_pTimeFT;
    NumericField*       m_pDateOffsetED;
    VclContainer*       m_pFormat;
    ListBox*            m_pFormatLB;
    NumFormatListBox*   m_pNumFormatLB;
    CheckBox*           m_pFixedCB;

    sal_uInt16              nOldSel;
    sal_uLong               nOldFormat;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK(FormatHdl, void * = 0);
    DECL_LINK(SubTypeHdl, void *);

    void                AddSubType(sal_uInt16 nTypeId);
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDokPage(Window* pWindow, const SfxItemSet& rSet);

                        ~SwFldDokPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
