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

#ifndef _SVX_OPTUPDT_HXX
#define _SVX_OPTUPDT_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/container/XNameReplace.hpp>

// class SvxPathTabPage --------------------------------------------------

class SvxOnlineUpdateTabPage : public SfxTabPage
{
private:
    FixedLine           m_aOptionsLine;
    CheckBox            m_aAutoCheckCheckBox;
    RadioButton         m_aEveryDayButton;
    RadioButton         m_aEveryWeekButton;
    RadioButton         m_aEveryMonthButton;
    PushButton          m_aCheckNowButton;
    CheckBox            m_aAutoDownloadCheckBox;
    FixedText           m_aDestPathLabel;
    FixedText           m_aDestPath;
    PushButton          m_aChangePathButton;
    FixedText           m_aLastChecked;
    rtl::OUString       m_aNeverChecked;
    rtl::OUString       m_aLastCheckedTemplate;

    DECL_LINK(FileDialogHdl_Impl, void *) ;
    DECL_LINK(CheckNowHdl_Impl, void *) ;
    DECL_LINK(  AutoCheckHdl_Impl, CheckBox* ) ;

    void        CalcButtonWidth();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xUpdateAccess;

    void                    UpdateLastCheckedText();

public:
    SvxOnlineUpdateTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxOnlineUpdateTabPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual void            FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
