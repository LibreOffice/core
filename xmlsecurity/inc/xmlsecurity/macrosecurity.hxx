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

#ifndef _XMLSECURITY_MACROSECURITY_HXX
#define _XMLSECURITY_MACROSECURITY_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>
#include <unotools/securityoptions.hxx>

namespace com {
namespace sun {
namespace star {
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class MacroSecurityTP;

class ReadOnlyImage : public FixedImage
{
public:
    ReadOnlyImage(Window* pParent, const ResId rResId);
    ~ReadOnlyImage();

    virtual void        RequestHelp( const HelpEvent& rHEvt );
    static const String& GetHelpTip();
};

class MacroSecurity : public TabDialog
{
private:
    friend class MacroSecurityLevelTP;
    friend class MacroSecurityTrustedSourcesTP;

    TabControl          maTabCtrl;
    OKButton            maOkBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;
    PushButton          maResetBtn;

    cssu::Reference< cssu::XComponentContext >  mxCtx;
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >  mxSecurityEnvironment;
    SvtSecurityOptions                                          maSecOptions;

    MacroSecurityTP*    mpLevelTP;
    MacroSecurityTP*    mpTrustSrcTP;

    DECL_LINK(          OkBtnHdl, void* );
public:
    MacroSecurity( Window* pParent, const cssu::Reference< cssu::XComponentContext>& rxCtx, const cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment );
    virtual             ~MacroSecurity();

    inline void     EnableReset( bool _bEnable = true );
};

inline void MacroSecurity::EnableReset( bool _bEnable )
{
    maResetBtn.Enable ( _bEnable );
}

class MacroSecurityTP : public TabPage
{
protected:
    MacroSecurity*      mpDlg;
public:
                        MacroSecurityTP( Window* _pParent, const ResId& _rResId, MacroSecurity* _pDlg );
    inline void         SetTabDlg( MacroSecurity* pTabDlg );

    virtual void        ClosePage( void ) = 0;
};

inline void MacroSecurityTP::SetTabDlg( MacroSecurity* _pTabDlg )
{
    mpDlg = _pTabDlg;
}


class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    FixedLine           maSecLevelFL;
    ReadOnlyImage       maSecReadonlyFI;
    RadioButton         maVeryHighRB;
    RadioButton         maHighRB;
    RadioButton         maMediumRB;
    RadioButton         maLowRB;

    sal_uInt16              mnCurLevel;

protected:
    DECL_LINK(          RadioButtonHdl, RadioButton* );

public:
                        MacroSecurityLevelTP( Window* pParent, MacroSecurity* _pDlg );

    virtual void        ClosePage( void );
};


class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    FixedLine           maTrustCertFL;
    ReadOnlyImage       maTrustCertROFI;
    SvxSimpleTable      maTrustCertLB;  // #i48648 now SvHeaderTabListBox
    PushButton          maAddCertPB;
    PushButton          maViewCertPB;
    PushButton          maRemoveCertPB;
    FixedLine           maTrustFileLocFL;
    ReadOnlyImage       maTrustFileROFI;
    FixedInfo           maTrustFileLocFI;
    ListBox             maTrustFileLocLB;
    PushButton          maAddLocPB;
    PushButton          maRemoveLocPB;

    cssu::Sequence< SvtSecurityOptions::Certificate > maTrustedAuthors;

    sal_Bool            mbAuthorsReadonly;
    sal_Bool            mbURLsReadonly;

    DECL_LINK(          ViewCertPBHdl, void* );
    DECL_LINK(          RemoveCertPBHdl, void* );
    DECL_LINK(          AddLocPBHdl, void* );
    DECL_LINK(          RemoveLocPBHdl, void* );
    DECL_LINK(          TrustCertLBSelectHdl, void* );
    DECL_LINK(          TrustFileLocLBSelectHdl, void* );

    void                FillCertLB( void );
    void                ImplCheckButtons();

public:
                        MacroSecurityTrustedSourcesTP( Window* pParent, MacroSecurity* _pDlg );

    virtual void        ActivatePage();
    virtual void        ClosePage( void );
};


#endif // _XMLSECURITY_MACROSECURITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
