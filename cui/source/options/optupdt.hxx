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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTUPDT_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTUPDT_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/container/XNameReplace.hpp>

// class SvxPathTabPage --------------------------------------------------

class SvxOnlineUpdateTabPage : public SfxTabPage
{
private:
    CheckBox*           m_pAutoCheckCheckBox;
    RadioButton*        m_pEveryDayButton;
    RadioButton*        m_pEveryWeekButton;
    RadioButton*        m_pEveryMonthButton;
    PushButton*         m_pCheckNowButton;
    CheckBox*           m_pAutoDownloadCheckBox;
    FixedText*          m_pDestPathLabel;
    FixedText*          m_pDestPath;
    PushButton*         m_pChangePathButton;
    FixedText*          m_pLastChecked;
    OUString       m_aNeverChecked;
    OUString       m_aLastCheckedTemplate;

    DECL_LINK(FileDialogHdl_Impl, void *) ;
    DECL_LINK(CheckNowHdl_Impl, void *) ;
    DECL_LINK(  AutoCheckHdl_Impl, CheckBox* ) ;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xUpdateAccess;

    void                    UpdateLastCheckedText();

public:
    SvxOnlineUpdateTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxOnlineUpdateTabPage();

    static SfxTabPage*      Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual bool            FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void            Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void            FillUserData() SAL_OVERRIDE;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
