/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: newerverwarn.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "newerverwarn.hxx"
#include "sfxresid.hxx"
#include "sfx2/app.hxx"
#include "appdata.hxx"

#include "newerverwarn.hrc"
#include "dialog.hrc"

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>
#include <vcl/msgbox.hxx>

namespace beans = ::com::sun::star::beans;
namespace frame = ::com::sun::star::frame;
namespace lang  = ::com::sun::star::lang;
namespace uno   = ::com::sun::star::uno;
namespace util  = ::com::sun::star::util;

namespace sfx2
{

NewerVersionWarningDialog::NewerVersionWarningDialog( Window* pParent ) :

    SfxModalDialog( pParent, SfxResId( RID_DLG_NEWER_VERSION_WARNING ) ),

    m_aImage        ( this, SfxResId( FI_IMAGE ) ),
    m_aInfoText     ( this, SfxResId( FT_INFO ) ),
    m_aButtonLine   ( this, SfxResId( FL_BUTTON ) ),
    m_aUpdateBtn    ( this, SfxResId( PB_UPDATE ) ),
    m_aLaterBtn     ( this, SfxResId( PB_LATER ) )

{
    FreeResource();

    m_aUpdateBtn.SetClickHdl( LINK( this, NewerVersionWarningDialog, UpdateHdl ) );
    m_aLaterBtn.SetClickHdl( LINK( this, NewerVersionWarningDialog, LaterHdl ) );

    InitButtonWidth();
}

NewerVersionWarningDialog::~NewerVersionWarningDialog()
{
}

IMPL_LINK( NewerVersionWarningDialog, UpdateHdl, PushButton*, EMPTYARG )
{
    uno::Reference< lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    try
    {
        uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                xSMGR,
                                DEFINE_CONST_UNICODE("org.openoffice.Office.Addons/"),
                                DEFINE_CONST_UNICODE("AddonUI/OfficeHelp/UpdateCheckJob"),
                                DEFINE_CONST_UNICODE("URL"),
                                ::comphelper::ConfigurationHelper::E_READONLY );
        util::URL aURL;
        if ( aVal >>= aURL.Complete )
        {
            uno::Reference< util::XURLTransformer > xTransformer(
                xSMGR->createInstance( DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer") ),
                uno::UNO_QUERY_THROW );
            xTransformer->parseStrict( aURL );
            uno::Reference < frame::XDesktop > xDesktop(
                xSMGR->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ),
                uno::UNO_QUERY_THROW );
            uno::Reference< frame::XDispatchProvider > xDispatchProvider(
                xDesktop->getCurrentFrame(), uno::UNO_QUERY );
            if ( !xDispatchProvider.is() )
                xDispatchProvider = uno::Reference < frame::XDispatchProvider > ( xDesktop, uno::UNO_QUERY );

            uno::Reference< frame::XDispatch > xDispatch =
                xDispatchProvider->queryDispatch( aURL, rtl::OUString(), 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( aURL, uno::Sequence< beans::PropertyValue >() );
        }
    }
    catch( const uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK( NewerVersionWarningDialog, LaterHdl, CancelButton*, EMPTYARG )
{
    SFX_APP()->Get_Impl()->bODFVersionWarningLater = sal_True;
    EndDialog( RET_CANCEL );
    return 0;
}

void NewerVersionWarningDialog::InitButtonWidth()
{
    // one button too small for its text?
    long nBtnTextWidth = m_aUpdateBtn.GetCtrlTextWidth( m_aUpdateBtn.GetText() );
    long nTemp = m_aLaterBtn.GetCtrlTextWidth( m_aLaterBtn.GetText() );
    if ( nTemp > nBtnTextWidth )
        nBtnTextWidth = nTemp;
    nBtnTextWidth = nBtnTextWidth * 115 / 100; // a little offset
    long nMaxBtnWidth = LogicToPixel( Size( MAX_BUTTON_WIDTH, 0 ), MAP_APPFONT ).Width();
    nBtnTextWidth = std::min( nBtnTextWidth, nMaxBtnWidth );
    long nButtonWidth = m_aUpdateBtn .GetSizePixel().Width();

    if ( nBtnTextWidth > nButtonWidth )
    {
        long nDelta = nBtnTextWidth - nButtonWidth;
        Point aNewPos = m_aUpdateBtn.GetPosPixel();
        aNewPos.X() -= 2*nDelta;
        Size aNewSize = m_aUpdateBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        m_aUpdateBtn.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos = m_aLaterBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        m_aLaterBtn.SetPosSizePixel( aNewPos, aNewSize );
    }
}

} // end of namespace sfx2

