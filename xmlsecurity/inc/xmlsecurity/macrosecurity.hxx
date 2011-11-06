/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    SvxSimpleTable      maTrustCertLB;  // PB 2006/02/02 #i48648 now SvHeaderTabListBox
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

