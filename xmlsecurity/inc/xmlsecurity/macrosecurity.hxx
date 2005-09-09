/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macrosecurity.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:07:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include <svtools/securityoptions.hxx>

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

    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >  mxSecurityEnvironment;
    SvtSecurityOptions                                          maSecOptions;

    MacroSecurityTP*    mpLevelTP;
    MacroSecurityTP*    mpTrustSrcTP;

    DECL_LINK(          OkBtnHdl, void* );
public:
    MacroSecurity( Window* pParent, const cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment );
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

    USHORT              mnCurLevel;

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
    SvxSimpleTable      maTrustCertLB;
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

