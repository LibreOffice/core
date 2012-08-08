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
