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
#include "minimizer.hrc"
#include "helpid.hrc"

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <rtl/ustrbuf.hxx>
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#define DIALOG_WIDTH    240
#define DIALOG_HEIGHT   80
#define PAGE_POS_X      35
#define PAGE_WIDTH      ( DIALOG_WIDTH - PAGE_POS_X ) - 6


// ---------------------
// - INFORMATIONDIALOG -
// ---------------------

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

using ::rtl::OUString;

// -----------------------------------------------------------------------------

Reference< XFixedText > InformationDialog::InsertFixedText(
    const rtl::OUString& rControlName,
    const OUString& rLabel,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Bool bMultiLine,
    sal_Int16 nTabIndex )
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

    return insertFixedText( rControlName, aNames, aValues );
}

Reference< XControl > InformationDialog::InsertImage(
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

    return insertImage( rControlName, aNames, aValues );
}

Reference< XCheckBox > InformationDialog::InsertCheckBox(
    const OUString& rControlName,
    const OUString& rLabel,
    const OUString& rHelpURL,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        TKGet( TK_Enabled ),
        TKGet( TK_Height ),
        TKGet( TK_HelpURL ),
        TKGet( TK_Label ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Step ),
        TKGet( TK_TabIndex ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nHeight ),
        Any( rHelpURL ),
        Any( rLabel ),
        Any( nXPos ),
        Any( nYPos ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    return insertCheckBox( rControlName, aNames, aValues );
}

Reference< XButton > InformationDialog::InsertButton(
    const OUString& rControlName,
    sal_Int32 nXPos,
    sal_Int32 nYPos,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nTabIndex,
    sal_Int32 nResID )
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
        Any( getString( nResID ) ),
        Any( nXPos ),
        Any( nYPos ),
        Any( static_cast< sal_Int16 >( PushButtonType_OK ) ),
        Any( (sal_Int16)0 ),
        Any( nTabIndex ),
        Any( nWidth ) };


    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    return insertButton( rControlName, Reference< XActionListener >(), aNames, aValues );
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
        TKGet( TK_HelpURL ),
        TKGet( TK_Moveable ),
        TKGet( TK_PositionX ),
        TKGet( TK_PositionY ),
        TKGet( TK_Title ),
        TKGet( TK_Width ) };

    Any pValues[] = {
        Any( sal_True ),
        Any( nDialogHeight ),
        Any( HID( HID_SDEXT_MINIMIZER_INFOBOX_DLG ) ),
        Any( sal_True ),
        Any( sal_Int32( 245 ) ),
        Any( sal_Int32( 115 ) ),
        Any( getString( STR_INFO_DIALOG ) ),
        Any( sal_Int32( DIALOG_WIDTH ) ) };

    sal_Int32 nCount = sizeof( pNames ) / sizeof( OUString );

    Sequence< rtl::OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    mxDialogModelMultiPropertySet->setPropertyValues( aNames, aValues );

    sal_Int64 nSource = mnSourceSize;
    sal_Int64 nDest   = mnDestSize;

    sal_Int32 nInfoStrResId( STR_INFO_1 );
    if ( mnSourceSize )
    {
        if ( mnDestSize )
            nInfoStrResId = STR_INFO_1;
        else
        {
            nInfoStrResId = STR_INFO_2;
            nDest = mnApproxSize;
        }
    }
    else if ( mnDestSize )
        nInfoStrResId = STR_INFO_3;
    else
    {
        nInfoStrResId = STR_INFO_4;
        nDest = mnApproxSize;
    }

    rtl::OUString aTitle;
    if ( maSaveAsURL.getLength() )
    {
        Reference< XURLTransformer > xURLTransformer( mxContext->getServiceManager()->createInstanceWithContext(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ), mxContext ), UNO_QUERY );
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

    OUString aInfoString( getString( nInfoStrResId ) );
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


    Reference< XControl > xImageCtrl = InsertImage( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "aboutimage" ) ),
                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:standardimage/query" ) ),
                 3, 3, 25, 25, sal_False );
    Reference< XControl > xFixedText( InsertFixedText( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "fixedtext" ) ), aInfoString, PAGE_POS_X, 6, PAGE_WIDTH, 24, sal_True, 0 ), UNO_QUERY);

    if ( maSaveAsURL.getLength() )
        mxCheckBox = InsertCheckBox( TKGet( TK_OpenNewDocument ),
                                     getString( STR_AUTOMATICALLY_OPEN ),
                                     HID( HID_SDEXT_MINIMIZER_INFOBOX_CB_OPENNEW ),
                                     PAGE_POS_X, 42,
                                     PAGE_WIDTH, 8, 1 );
    Reference< XControl> xButtonCtrl( InsertButton( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "button" ) ), DIALOG_WIDTH / 2 - 15, nDialogHeight - 20, 30, 14, 2, STR_OK ), UNO_QUERY );

    sal_Bool bOpenNewDocument = mrbOpenNewDocument;
    setControlProperty( TKGet( TK_OpenNewDocument ), TKGet( TK_State ), Any( (sal_Int16)bOpenNewDocument ) );

    sal_Int32 nX, nY, nDlgWidth;
    Size aSize3( 3, 3 );
    Reference< XUnitConversion > xUnitConverter( mxDialogWindow, UNO_QUERY_THROW );
    aSize3 = xUnitConverter->convertSizeToPixel( aSize3, util::MeasureUnit::APPFONT );
    nX = nY = aSize3.Width;

    Reference< XPropertySet > xImageProps (xImageCtrl->getModel(), UNO_QUERY_THROW );
    Reference< graphic::XGraphic > xGraphic(
        xImageProps->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic") ) ), UNO_QUERY_THROW );
    if ( xGraphic.is() && xGraphic->getType() != graphic::GraphicType::EMPTY )
    {
        Size aSizePixel;
        Rectangle aPosSizePixel;
        Reference< XPropertySet > xGraphicProps( xGraphic, UNO_QUERY_THROW );
        xGraphicProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("SizePixel"))) >>= aSizePixel;
        Reference< XWindow > xImage (xImageCtrl->getPeer(), UNO_QUERY_THROW );

        xImage->setPosSize( nX, nY,
                            aSizePixel.Width,
                            aSizePixel.Height,
                            awt::PosSize::POSSIZE );

        nX += aSizePixel.Width + aSize3.Width;

        Reference< XWindow > xLabel ( xFixedText->getPeer(), UNO_QUERY_THROW );
        aPosSizePixel = xLabel->getPosSize();
        xLabel->setPosSize( nX, nY,
                            aPosSizePixel.Width,
                            aPosSizePixel.Height,
                            awt::PosSize::POSSIZE );

        nDlgWidth = nX + aPosSizePixel.Width + aSize3.Width;
        nY += aPosSizePixel.Height + aSize3.Height;

        if ( mxCheckBox.is() )
        {
            Reference< XControl > xCheckBoxCtrl( mxCheckBox, UNO_QUERY_THROW );
            Reference< XWindow > xCheckBox( xCheckBoxCtrl->getPeer(), UNO_QUERY_THROW );
            Rectangle aCBSize = xCheckBox->getPosSize();
            xCheckBox->setPosSize( nX, nY,
                                   aPosSizePixel.Width,
                                   aCBSize.Height ,
                                   awt::PosSize::POSSIZE);
            nY += aCBSize.Height + aSize3.Height;
        }

        nY += aSize3.Height;
        Reference< XWindow > xButton ( xButtonCtrl->getPeer(), UNO_QUERY_THROW );
        aPosSizePixel = xButton->getPosSize();
        xButton->setPosSize( nDlgWidth / 2 - aPosSizePixel.Width / 2,
                             nY ,
                             aPosSizePixel.Width,
                             aPosSizePixel.Height,
                             awt::PosSize::POSSIZE );
        nY += aPosSizePixel.Height + aSize3.Height;
        mxDialogWindow->setPosSize( 0, 0, nDlgWidth, nY, awt::PosSize::SIZE );
    }

    centerDialog();
}

// -----------------------------------------------------------------------------

InformationDialog::InformationDialog(
    const Reference< XComponentContext > &rxContext,
    const Reference< XWindowPeer>& rxParent,
    const rtl::OUString& rSaveAsURL,
    sal_Bool& rbOpenNewDocument,
    const sal_Int64& rSourceSize,
    const sal_Int64& rDestSize,
    const sal_Int64& rApproxSize )
:   UnoDialog( rxContext, rxParent ),
    ConfigurationAccess( rxContext, NULL ),
    mxContext( rxContext ),
    mnSourceSize( rSourceSize ),
    mnDestSize( rDestSize ),
    mnApproxSize( rApproxSize ),
    mrbOpenNewDocument( rbOpenNewDocument ),
    maSaveAsURL( rSaveAsURL )
{
    OSL_TRACE( "InformationDialog::InformationDialog" );
    createWindowPeer();

    InitDialog();
}

// -----------------------------------------------------------------------------

InformationDialog::~InformationDialog()
{
    OSL_TRACE( "InformationDialog::~InformationDialog" );
}

// -----------------------------------------------------------------------------

sal_Bool InformationDialog::execute()
{
    OSL_TRACE( "InformationDialog::execute" );
    UnoDialog::execute();

    if ( maSaveAsURL.getLength() && mxCheckBox.is() )
    {
        sal_Bool bOpenNewDocument = static_cast< sal_Bool >( mxCheckBox->getState() );
        mrbOpenNewDocument = bOpenNewDocument;
    }

    endExecute( sal_True );
    return mbStatus;
}

