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



// - INFORMATIONDIALOG -


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





OUString InsertFixedText( InformationDialog& rInformationDialog, const OUString& rControlName, const OUString& rLabel,
                                sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, bool bMultiLine, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        OUString("Height"),
        OUString("Label"),
        OUString("MultiLine"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

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

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertFixedText( rControlName, aNames, aValues );
    return rControlName;
}

OUString InsertImage(
    InformationDialog& rInformationDialog,
    const OUString& rControlName,
    const OUString& rURL,
    sal_Int32 nPosX,
    sal_Int32 nPosY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    bool bScale )
{
    OUString pNames[] = {
        OUString("Border"),
        OUString("Height"),
        OUString("ImageURL"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("ScaleImage"),
        OUString("Width") };

    Any pValues[] = {
        Any( sal_Int16( 0 ) ),
        Any( nHeight ),
        Any( rURL ),
        Any( nPosX ),
        Any( nPosY ),
        Any( bScale ),
        Any( nWidth ) };
    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertImage( rControlName, aNames, aValues );
    return rControlName;
}

OUString InsertCheckBox( InformationDialog& rInformationDialog, const OUString& rControlName,
    const Reference< XItemListener >& rItemListener, const OUString& rLabel,
        sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex )
{
    OUString pNames[] = {
        OUString("Enabled"),
        OUString("Height"),
        OUString("Label"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

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

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    Reference< XCheckBox > xCheckBox( rInformationDialog.insertCheckBox( rControlName, aNames, aValues ) );
    if ( rItemListener.is() )
        xCheckBox->addItemListener( rItemListener );
    return rControlName;
}

OUString InsertButton( InformationDialog& rInformationDialog, const OUString& rControlName, Reference< XActionListener >& xActionListener,
    sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nTabIndex, PPPOptimizerTokenEnum nResID )
{
    OUString pNames[] = {
        OUString("Enabled"),
        OUString("Height"),
        OUString("Label"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("PushButtonType"),
        OUString("Step"),
        OUString("TabIndex"),
        OUString("Width") };

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

    Sequence< OUString >   aNames( pNames, nCount );
    Sequence< Any >             aValues( pValues, nCount );

    rInformationDialog.insertButton( rControlName, xActionListener, aNames, aValues );
    return rControlName;
}


static OUString ImpValueOfInMB( const sal_Int64& rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.' ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

void InformationDialog::InitDialog()
{
    sal_Int32 nDialogHeight = DIALOG_HEIGHT;
    if ( maSaveAsURL.isEmpty() )
        nDialogHeight -= 22;

   // setting the dialog properties
    OUString pNames[] = {
        OUString("Closeable"),
        OUString("Height"),
        OUString("Moveable"),
        OUString("PositionX"),
        OUString("PositionY"),
        OUString("Title"),
        OUString("Width") };

    Any pValues[] = {
        Any( sal_True ),
        Any( nDialogHeight ),
        Any( sal_True ),
        Any( sal_Int32( 245 ) ),
        Any( sal_Int32( 115 ) ),
        Any( getString( STR_SUN_OPTIMIZATION_WIZARD2 ) ),
        Any( sal_Int32( DIALOG_WIDTH ) ) };

    sal_Int32 nCount = SAL_N_ELEMENTS( pNames );

    Sequence< OUString >   aNames( pNames, nCount );
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

    OUString aTitle;
    if ( !maSaveAsURL.isEmpty() )
    {
        Reference< XURLTransformer > xURLTransformer( URLTransformer::create(UnoDialog::mxContext) );
        util::URL aURL, aPresentationURL;
        aURL.Complete = maSaveAsURL;
        xURLTransformer->parseSmart( aURL, OUString() );

        const OUString sFileProtocol( "file:///" );
        aPresentationURL.Complete = sFileProtocol.concat( aURL.Name );
        aTitle = xURLTransformer->getPresentation( aPresentationURL, sal_False );

        if ( aTitle.match( sFileProtocol ) )
            aTitle = aTitle.replaceAt( 0, sFileProtocol.getLength(), OUString() );
    }

    OUString aInfoString( getString( eInfoString ) );
    const OUString aOldSizePlaceholder( "%OLDFILESIZE"  );
    const OUString aNewSizePlaceholder( "%NEWFILESIZE"  );
    const OUString aTitlePlaceholder( !aTitle.isEmpty() ? OUString("%TITLE"  )
                                                         : OUString("'%TITLE'") );

    sal_Int32 i = aInfoString.indexOf( aOldSizePlaceholder );
    if ( i >= 0 )
        aInfoString = aInfoString.replaceAt( i, aOldSizePlaceholder.getLength(), ImpValueOfInMB( nSource ) );

    sal_Int32 j = aInfoString.indexOf( aNewSizePlaceholder );
    if ( j >= 0 )
        aInfoString = aInfoString.replaceAt( j, aNewSizePlaceholder.getLength(), ImpValueOfInMB( nDest ) );

    sal_Int32 k = aInfoString.indexOf( aTitlePlaceholder );
    if ( k >= 0 )
        aInfoString = aInfoString.replaceAt( k, aTitlePlaceholder.getLength(), aTitle );

    css::uno::Reference< css::awt::XItemListener > xItemListener;
    InsertImage( *this,
                 "aboutimage",
                 "private:standardimage/query",
                 5, 5, 25, 25, false );
    InsertFixedText( *this, "fixedtext", aInfoString, PAGE_POS_X, 6, PAGE_WIDTH, 24, true, 0 );
    if ( !maSaveAsURL.isEmpty() )
        InsertCheckBox(  *this, "OpenNewDocument", xItemListener, getString( STR_AUTOMATICALLY_OPEN ), PAGE_POS_X, 42, PAGE_WIDTH, 8, 1 );
    InsertButton( *this, "button", mxActionListener, DIALOG_WIDTH / 2 - 25, nDialogHeight - 20, 50, 14, 2, STR_OK );

    bool bOpenNewDocument = mrbOpenNewDocument;
    setControlProperty( "OpenNewDocument", "State", Any( (sal_Int16)bOpenNewDocument ) );
}



InformationDialog::InformationDialog( const Reference< XComponentContext > &rxContext, Reference< XFrame >& rxFrame, const OUString& rSaveAsURL, bool& rbOpenNewDocument, const sal_Int64& rSourceSize, const sal_Int64& rDestSize, const sal_Int64& rApproxSize ) :
    UnoDialog( rxContext, rxFrame ),
    ConfigurationAccess( rxContext, nullptr ),
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



InformationDialog::~InformationDialog()
{
}



bool InformationDialog::execute()
{
    UnoDialog::execute();

    if ( !maSaveAsURL.isEmpty() )
    {
        sal_Int16 nInt16 = 0;
        Any aAny( getControlProperty( "OpenNewDocument", "State" ) );
        if ( aAny >>= nInt16 )
        {
            bool bOpenNewDocument = static_cast< bool >( nInt16 );
            mrbOpenNewDocument = bOpenNewDocument;
        }
    }
    return mbStatus;
}



void OKActionListener::actionPerformed( const ActionEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.ActionCommand == "button" )
    {
        mrInformationDialog.endExecute( true );
    }
}
void OKActionListener::disposing( const css::lang::EventObject& /* Source */ )
    throw ( css::uno::RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
