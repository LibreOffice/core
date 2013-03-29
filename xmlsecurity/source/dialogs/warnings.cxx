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


#include <xmlsecurity/warnings.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <comphelper/sequence.hxx>

// added for password exception
#include <vcl/msgbox.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>
using namespace ::com::sun::star::security;


#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;


MacroWarning::MacroWarning( Window* _pParent, uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, css::uno::Reference< css::security::XCertificate >& _rxCert )
    :ModalDialog        ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROWARN ) )
    ,maDocNameFI        ( this, ResId( FI_DOCNAME ) )
    ,maDescr1aFI        ( this, ResId( FI_DESCR1A ) )
    ,maDescr1bFI        ( this, ResId( FI_DESCR1B ) )
    ,maSignsFI          ( this, ResId( FI_SIGNS ) )
    ,maViewSignsBtn     ( this, ResId( PB_VIEWSIGNS ) )
    ,maDescr2FI         ( this, ResId( FI_DESCR2 ) )
    ,maAlwaysTrustCB    ( this, ResId( CB_ALWAYSTRUST ) )
    ,maBottomSepFL      ( this, ResId( FL_BOTTOM_SEP ) )
    ,maEnableBtn        ( this, ResId( PB_DISABLE ) )
    ,maDisableBtn       ( this, ResId( PB_DISABLE ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
    ,mbSignedMode       ( true )
{
    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rxCert;

    // hide unused parts
    maDescr1bFI.Hide();

    maViewSignsBtn.SetClickHdl( LINK( this, MacroWarning, ViewSignsBtnHdl ) );
    maEnableBtn.SetClickHdl( LINK( this, MacroWarning, EnableBtnHdl ) );

    if( mxCert.is() )
        maSignsFI.SetText( XmlSec::GetContentPart( mxCert->getSubjectName() ) );
    else
        // nothing to view!
        maViewSignsBtn.Disable();
}

MacroWarning::MacroWarning( Window* _pParent )
    :ModalDialog        ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROWARN ) )
    ,maDocNameFI        ( this, ResId( FI_DOCNAME ) )
    ,maDescr1aFI        ( this, ResId( FI_DESCR1A ) )
    ,maDescr1bFI        ( this, ResId( FI_DESCR1B ) )
    ,maSignsFI          ( this, ResId( FI_SIGNS ) )
    ,maViewSignsBtn     ( this, ResId( PB_VIEWSIGNS ) )
    ,maDescr2FI         ( this, ResId( FI_DESCR2 ) )
    ,maAlwaysTrustCB    ( this, ResId( CB_ALWAYSTRUST ) )
    ,maBottomSepFL      ( this, ResId( FL_BOTTOM_SEP ) )
    ,maEnableBtn        ( this, ResId( PB_DISABLE ) )
    ,maDisableBtn       ( this, ResId( PB_DISABLE ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
    ,mbSignedMode       ( false )
{
    FreeResource();

    // hide unused parts
    maDescr1aFI.Hide();
    maSignsFI.Hide();
    maViewSignsBtn.Hide();
    maAlwaysTrustCB.Hide();
    maDescr2FI.Hide();

    // move hint up to position of signer list
    maDescr1bFI.SetPosPixel( maSignsFI.GetPosPixel() );
}

MacroWarning::~MacroWarning()
{
}

IMPL_LINK_NOARG(MacroWarning, ViewSignsBtnHdl)
{
    DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    CertificateViewer   aViewer( this, mxSecurityEnvironment, mxCert );
    aViewer.Execute();

    return 0;
}

IMPL_LINK_NOARG(MacroWarning, EnableBtnHdl)
{
    if( mbSignedMode && maAlwaysTrustCB.IsChecked() )
    {   // insert path into trusted path list

    }

    EndDialog( RET_OK );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
