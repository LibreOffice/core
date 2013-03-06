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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "informationdialog.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <rtl/ustrbuf.hxx>
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"

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

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertFixedText( rControlName, aNames, aValues );
    return rControlName;
}

rtl::OUString InsertImage(
    InformationDialog& rInformationDialog,
    const OUString& rControlName,
    const OUString& rURL,
    sal_Int32 nPosX,
    sal_Int32 nPosY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Bool bScale )
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
        Any( bScale ),
        Any( nWidth ) };
    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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


    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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

void InformationDialog::InitDialog()
{
    sal_Int32 nDialogHeight = DIALOG_HEIGHT;
    if ( !maSaveAsURL.getLength() )
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

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

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
    if ( maSaveAsURL.getLength() )
    {
        Reference< XURLTransformer > xURLTransformer( mxMSF->getServiceManager()->createInstanceWithContext(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ), mxMSF ), UNO_QUERY );
        if ( xURLTransformer.is() )
        {
            util::URL aURL, aPresentationURL;
            aURL.Complete = maSaveAsURL;
            xURLTransformer->parseSmart( aURL, rtl::OUString() );

            const OUString sFileProtocol( RTL_CONSTASCII_USTRINGPARAM( "file:///" ) );
            aPresentationURL.Complete = sFileProtocol.concat( aURL.Name );
            aTitle = xURLTransformer->getPresentation( aPresentationURL, sal_False );

            if ( aTitle.match( sFileProtocol, 0 ) )
                aTitle = aTitle.replaceAt( 0, sFileProtocol.getLength(), rtl::OUString() );
        }
    }

    OUString aInfoString( getString( eInfoString ) );
    const OUString aOldSizePlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%OLDFILESIZE" ) );
    const OUString aNewSizePlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%NEWFILESIZE" ) );
    const OUString aTitlePlaceholder( aTitle.getLength() ? OUString::createFromAscii( "%TITLE" ) : OUString::createFromAscii( "'%TITLE'" ) );

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
    InsertImage( *this,
                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "aboutimage" ) ),
                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:standardimage/query" ) ),
                 5, 5, 25, 25, sal_False );
    InsertFixedText( *this, rtl::OUString( rtl::OUString::createFromAscii( "fixedtext" ) ), aInfoString, PAGE_POS_X, 6, PAGE_WIDTH, 24, sal_True, 0 );
    if ( maSaveAsURL.getLength() )
        InsertCheckBox(  *this, TKGet( TK_OpenNewDocument ), xItemListener, getString( STR_AUTOMATICALLY_OPEN ), PAGE_POS_X, 42, PAGE_WIDTH, 8, 1 );
    InsertButton( *this, rtl::OUString( rtl::OUString::createFromAscii( "button" ) ), mxActionListener, DIALOG_WIDTH / 2 - 25, nDialogHeight - 20, 50, 14, 2, STR_OK );

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

    if ( maSaveAsURL.getLength() )
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
    if ( rEvent.ActionCommand == rtl::OUString( rtl::OUString::createFromAscii( "button" ) ) )
    {
        mrInformationDialog.endExecute( sal_True );
    }
}
void OKActionListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw ( com::sun::star::uno::RuntimeException )
{
}
