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


#include "informationdialog.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/URLTransformer.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>

#define DIALOG_WIDTH    240
#define DIALOG_HEIGHT   80
#define PAGE_POS_X      35
#define PAGE_WIDTH      ( DIALOG_WIDTH - PAGE_POS_X ) - 6


// ---------------------
// - INFORMATIONDIALOG -
// ---------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;



// -----------------------------------------------------------------------------

rtl::OUString InsertFixedText( InformationDialog& rInformationDialog, const rtl::OUString& rControlName, const OUString& rLabel,
                                sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Bool bMultiLine, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_MultiLine ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( nHeight ),
        Any( rLabel ),
        Any( bMultiLine ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertFixedText( rControlName, aNames, aValues );
    return rControlName;
}

rtl::OUString InsertImage( InformationDialog& rInformationDialog, const OUString& rControlName, const OUString& rURL,
                        sal_Int32 nPosX, sal_Int32 nPosY, sal_Int32 nWidth, sal_Int32 nHeight )
{
    OUString pNames[] = {
        TKGet( TK_Border ),
        TKGet( TK_Height ),
        TKGet( TK_ImageURL ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_ScaleImage ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_Int16( 0 ) ),
        Any( nHeight ),
        Any( rURL ),
        Any( nPosX ),
        Any( nPosY ),
        Any( sal_True ),
        Any( nWidth ) };
    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertImage( rControlName, aNames, aValues );
    return rControlName;
}

rtl::OUString InsertCheckBox( InformationDialog& rInformationDialog, const OUString& rControlName,
    const Reference< XItemListener > xItemListener, const OUString& rLabel,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nHeight ),
        Any( rLabel ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XCheckBox > xCheckBox( rInformationDialog.insertCheckBox( rControlName, aNames, aValues ) );
    if ( xItemListener.is() )
        xCheckBox->addItemListener( xItemListener );
    return rControlName;
}

rtl::OUString InsertButton( InformationDialog& rInformationDialog, const OUString& rControlName, Reference< XActionListener >& xActionListener,
    sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex, PPPOptimizerTokenEnum nResID )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_PushButtonType ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nHeight ),
        Any( rInformationDialog.getString( nResID ) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( static_cast< sal_Int16 >( PushButtonType_OK ) ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };


    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertButton( rControlName, xActionListener, aNames, aValues );
    return rControlName;
}


static OUString ImpValueOfInMB( const sal_Int64& rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    rtl::OUStringBuffer aVal( OUString::valueOf( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.', 0 ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

OUString InformationDialog::ImpGetStandardImage( const OUString& sPrivateURL )
{
    rtl::OUString sURL;
    try
    {
        mxTempFile = Reference< XStream >( io::TempFile::create(mxMSF), UNO_QUERY_THROW );
        Reference< XPropertySet > xPropSet( mxTempFile, UNO_QUERY );
        Reference< XOutputStream > xOutputStream( mxTempFile->getOutputStream() );
        if ( xOutputStream.is() && xPropSet.is() )
        {
            Reference< graphic::XGraphicProvider > xGraphicProvider( graphic::GraphicProvider::create( mxMSF ) );
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[ 0 ].Name = OUString("URL");
            aArgs[ 0 ].Value <<= sPrivateURL;
            Reference< graphic::XGraphic > xGraphic( xGraphicProvider->queryGraphic( aArgs ) );
            if ( xGraphic.is() )
            {
                OUString aDestMimeType( "image/png"  );
                Sequence< PropertyValue > aArgs2( 2 );
                aArgs2[ 0 ].Name = TKGet( TK_MimeType );                // the GraphicProvider is using "MimeType", the GraphicExporter "MediaType"...
                aArgs2[ 0 ].Value <<= aDestMimeType;
                aArgs2[ 1 ].Name = TKGet( TK_OutputStream );
                aArgs2[ 1 ].Value <<= xOutputStream;
                xGraphicProvider->storeGraphic( xGraphic, aArgs2 );
            }
            xPropSet->getPropertyValue( OUString("Uri") ) >>= sURL;
        }
    }
    catch( Exception& )
    {
    }
    return sURL;
}

void InformationDialog::InitDialog()
{
    sal_Int32 nDialogHeight = DIALOG_HEIGHT;
    if ( maSaveAsURL.isEmpty() )
        nDialogHeight -= 22;

   // setting the dialog properties
    OUString pNames[] = {
        TKGet( TK_Closeable ),
        TKGet( TK_Height ),
        TKGet( TK_Moveable ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Title ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nDialogHeight ),
        Any( sal_True ),
        Any( sal_Int32( 245 ) ),
        Any( sal_Int32( 115 ) ),
        Any( getString( STR_SUN_OPTIMIZATION_WIZARD2 ) ),
        Any( sal_Int32( DIALOG_WIDTH ) ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    mxDialogModelMultiPropertySet->setPropertyValues( aNames, aValues );

    sal_Int64 nSource = mnSourceSize;
    sal_Int64 nDest   = mnDestSize;

    PPPOptimizerTokenEnum eInfoString( STR_INFO_1 );
    if ( mnSourceSize )
    {
        if ( mnDestSize )
            eInfoString = STR_INFO_1;
        else
        {
            eInfoString = STR_INFO_2;
            nDest = mnApproxSize;
        }
    }
    else if ( mnDestSize )
        eInfoString = STR_INFO_3;
    else
    {
        eInfoString = STR_INFO_4;
        nDest = mnApproxSize;
    }

    rtl::OUString aTitle;
    if ( !maSaveAsURL.isEmpty() )
    {
        Reference< XURLTransformer > xURLTransformer( URLTransformer::create(mxMSF) );
        util::URL aURL, aPresentationURL;
        aURL.Complete = maSaveAsURL;
        xURLTransformer->parseSmart( aURL, rtl::OUString() );

        const OUString sFileProtocol( "file:///" );
        aPresentationURL.Complete = sFileProtocol.concat( aURL.Name );
        aTitle = xURLTransformer->getPresentation( aPresentationURL, sal_False );

        if ( aTitle.match( sFileProtocol, 0 ) )
            aTitle = aTitle.replaceAt( 0, sFileProtocol.getLength(), rtl::OUString() );
    }

    OUString aInfoString( getString( eInfoString ) );
    const OUString aOldSizePlaceholder( "%OLDFILESIZE"  );
    const OUString aNewSizePlaceholder( "%NEWFILESIZE"  );
    const OUString aTitlePlaceholder( !aTitle.isEmpty() ? OUString("%TITLE"  )
                                                         : OUString("'%TITLE'") );

    sal_Int32 i = aInfoString.indexOf( aOldSizePlaceholder, 0 );
    if ( i >= 0 )
        aInfoString = aInfoString.replaceAt( i, aOldSizePlaceholder.getLength(), ImpValueOfInMB( nSource ) );

    sal_Int32 j = aInfoString.indexOf( aNewSizePlaceholder, 0 );
    if ( j >= 0 )
        aInfoString = aInfoString.replaceAt( j, aNewSizePlaceholder.getLength(), ImpValueOfInMB( nDest ) );

    sal_Int32 k = aInfoString.indexOf( aTitlePlaceholder, 0 );
    if ( k >= 0 )
        aInfoString = aInfoString.replaceAt( k, aTitlePlaceholder.getLength(), aTitle );

    com::sun::star::uno::Reference< com::sun::star::awt::XItemListener > xItemListener;
    InsertImage( *this, rtl::OUString("aboutimage"), ImpGetStandardImage( rtl::OUString("private:standardimage/query") ), 5, 5, 25, 25 );
    InsertFixedText( *this, rtl::OUString("fixedtext"), aInfoString, PAGE_POS_X, 6, PAGE_WIDTH, 24, sal_True, 0 );
    if ( !maSaveAsURL.isEmpty() )
        InsertCheckBox(  *this, TKGet( TK_OpenNewDocument ), xItemListener, getString( STR_AUTOMATICALLY_OPEN ), PAGE_POS_X, 42, PAGE_WIDTH, 8, 1 );
    InsertButton( *this, rtl::OUString("button"), mxActionListener, DIALOG_WIDTH / 2 - 25, nDialogHeight - 20, 50, 14, 2, STR_OK );

    sal_Bool bOpenNewDocument = mrbOpenNewDocument;
    setControlProperty( TKGet( TK_OpenNewDocument ), TKGet( TK_State ), Any( (sal_Int16)bOpenNewDocument ) );
}

// -----------------------------------------------------------------------------

InformationDialog::InformationDialog( const Reference< XComponentContext > &rxMSF, Reference< XFrame >& rxFrame, const rtl::OUString& rSaveAsURL, sal_Bool& rbOpenNewDocument, const sal_Int64& rSourceSize, const sal_Int64& rDestSize, const sal_Int64& rApproxSize ) :
    UnoDialog( rxMSF, rxFrame ),
    ConfigurationAccess( rxMSF, NULL ),
    mxMSF( rxMSF ),
    mxFrame( rxFrame ),
    mxActionListener( new OKActionListener( *this ) ),
    mnSourceSize( rSourceSize ),
    mnDestSize( rDestSize ),
    mnApproxSize( rApproxSize ),
    mrbOpenNewDocument( rbOpenNewDocument ),
    maSaveAsURL( rSaveAsURL )
{
    Reference< XFrame > xFrame( mxController->getFrame() );
    Reference< XWindow > xContainerWindow( xFrame->getContainerWindow() );
    Reference< XWindowPeer > xWindowPeer( xContainerWindow, UNO_QUERY_THROW );
    createWindowPeer( xWindowPeer );

    InitDialog();
}

// -----------------------------------------------------------------------------

InformationDialog::~InformationDialog()
{
}

// -----------------------------------------------------------------------------

sal_Bool InformationDialog::execute()
{
    UnoDialog::execute();

    if ( !maSaveAsURL.isEmpty() )
    {
        sal_Int16 nInt16 = 0;
        Any aAny( getControlProperty( TKGet( TK_OpenNewDocument ), TKGet( TK_State ) ) );
        if ( aAny >>= nInt16 )
        {
            sal_Bool bOpenNewDocument = static_cast< sal_Bool >( nInt16 );
            mrbOpenNewDocument = bOpenNewDocument;
        }
    }
    return mbStatus;
}

// -----------------------------------------------------------------------------

void OKActionListener::actionPerformed( const ActionEvent& rEvent )
    throw ( com::sun::star::uno::RuntimeException )
{
    if ( rEvent.ActionCommand == "button" )
    {
        mrInformationDialog.endExecute( sal_True );
    }
}
void OKActionListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
