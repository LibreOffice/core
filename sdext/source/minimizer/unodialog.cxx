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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "unodialog.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <com/sun/star/awt/MessageBoxButtons.hpp>

// -------------
// - UnoDialog -
// -------------

using namespace ::rtl;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;

UnoDialog::UnoDialog( const Reference< XComponentContext > &rxMSF, Reference< XFrame >& rxFrame ) :
    mxMSF( rxMSF ),
    mxController( rxFrame->getController() ),
    mxDialogModel( mxMSF->getServiceManager()->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.UnoControlDialogModel" ) ), mxMSF ), UNO_QUERY_THROW ),
    mxDialogModelMultiPropertySet( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelPropertySet( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelMSF( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameContainer( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameAccess( mxDialogModel, UNO_QUERY_THROW ),
    mxControlModel( mxDialogModel, UNO_QUERY_THROW ),
    mxDialog( mxMSF->getServiceManager()->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.UnoControlDialog" ) ), mxMSF ), UNO_QUERY_THROW ),
    mxControl( mxDialog, UNO_QUERY_THROW ),
    mbStatus( sal_False )
{
    mxControl->setModel( mxControlModel );
    mxDialogControlContainer = Reference< XControlContainer >( mxDialog, UNO_QUERY_THROW );
    mxDialogComponent = Reference< XComponent >( mxDialog, UNO_QUERY_THROW );
    mxDialogWindow = Reference< XWindow >( mxDialog, UNO_QUERY_THROW );

    Reference< XFrame > xFrame( mxController->getFrame() );
    Reference< XWindow > xContainerWindow( xFrame->getContainerWindow() );
    mxWindowPeer = Reference< XWindowPeer >( xContainerWindow, UNO_QUERY_THROW );
    createWindowPeer( mxWindowPeer );
}

// -----------------------------------------------------------------------------

UnoDialog::~UnoDialog()
{

}

// -----------------------------------------------------------------------------

void UnoDialog::execute()
{
    mxDialogWindow->setEnable( sal_True );
    mxDialogWindow->setVisible( sal_True );
    mxDialog->execute();
}

void UnoDialog::endExecute( sal_Bool bStatus )
{
    mbStatus = bStatus;
    mxDialog->endExecute();
}

// -----------------------------------------------------------------------------

Reference< XWindowPeer > UnoDialog::createWindowPeer( Reference< XWindowPeer > xParentPeer )
    throw ( Exception )
{
    mxDialogWindow->setVisible( sal_False );
    Reference< XToolkit > xToolkit( mxMSF->getServiceManager()->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ), mxMSF ), UNO_QUERY_THROW  );
    if ( !xParentPeer.is() )
        xParentPeer = xToolkit->getDesktopWindow();
    mxReschedule = Reference< XReschedule >( xToolkit, UNO_QUERY );
    mxControl->createPeer( xToolkit, xParentPeer );
//  xWindowPeer = xControl.getPeer();
    return mxControl->getPeer();
}

// -----------------------------------------------------------------------------

Reference< XInterface > UnoDialog::insertControlModel( const OUString& rServiceName, const OUString& rName,
                                                        const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XInterface > xControlModel;
    try
    {
        xControlModel = mxDialogModelMSF->createInstance( rServiceName );
        Reference< XMultiPropertySet > xMultiPropSet( xControlModel, UNO_QUERY_THROW );
        xMultiPropSet->setPropertyValues( rPropertyNames, rPropertyValues );
        mxDialogModelNameContainer->insertByName( rName, Any( xControlModel ) );
    }
    catch( Exception& )
    {
    }
    return xControlModel;
}

// -----------------------------------------------------------------------------

void UnoDialog::setVisible( const OUString& rName, sal_Bool bVisible )
{
    try
    {
        Reference< XInterface > xControl( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
        Reference< XWindow > xWindow( xControl, UNO_QUERY_THROW );
        xWindow->setVisible( bVisible );
    }
    catch ( Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

sal_Bool UnoDialog::isHighContrast()
{
    sal_Bool bHighContrast = sal_False;
    try
    {
        sal_Int32 nBackgroundColor = 0;
        if ( mxDialogModelPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "BackgroundColor" ) ) ) >>= nBackgroundColor )
        {
            sal_uInt8 nLum( static_cast< sal_uInt8 >( ( static_cast< sal_uInt8 >( nBackgroundColor >> 16 ) * 28 +
                                                        static_cast< sal_uInt8 >( nBackgroundColor >> 8 ) * 151 +
                                                        static_cast< sal_uInt8 >( nBackgroundColor ) * 77 ) >> 8 ) );
            bHighContrast = nLum <= 38;
        }
    }
    catch( Exception& )
    {
    }
    return bHighContrast;
}

// -----------------------------------------------------------------------------

Reference< XButton > UnoDialog::insertButton( const OUString& rName, Reference< XActionListener > xActionListener,
            const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XButton > xButton;
    try
    {
        Reference< XInterface > xButtonModel( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ),
            rName, rPropertyNames, rPropertyValues ) );
        Reference< XPropertySet > xPropertySet( xButtonModel, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xButton = Reference< XButton >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );

        if ( xActionListener.is() )
        {
            xButton->addActionListener( xActionListener );
            xButton->setActionCommand( rName );
        }
        return xButton;
    }
    catch( Exception& )
    {
    }
    return xButton;
}

// -----------------------------------------------------------------------------

Reference< XFixedText > UnoDialog::insertFixedText( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XFixedText > xFixedText;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedTextModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xFixedText = Reference< XFixedText >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xFixedText;
}

// -----------------------------------------------------------------------------

Reference< XCheckBox > UnoDialog::insertCheckBox( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XCheckBox > xCheckBox;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCheckBoxModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xCheckBox = Reference< XCheckBox >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xCheckBox;
}

// -----------------------------------------------------------------------------

Reference< XControl > UnoDialog::insertFormattedField( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XControl > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFormattedFieldModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xControl = Reference< XControl >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}

// -----------------------------------------------------------------------------

Reference< XComboBox > UnoDialog::insertComboBox( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XComboBox > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlComboBoxModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xControl = Reference< XComboBox >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}

// -----------------------------------------------------------------------------

Reference< XRadioButton > UnoDialog::insertRadioButton( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XRadioButton > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRadioButtonModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xControl = Reference< XRadioButton >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}

// -----------------------------------------------------------------------------

Reference< XListBox > UnoDialog::insertListBox( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XListBox > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlListBoxModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xControl = Reference< XListBox >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}

// -----------------------------------------------------------------------------

Reference< XControl > UnoDialog::insertImage( const OUString& rName, const Sequence< OUString > rPropertyNames, const Sequence< Any > rPropertyValues )
{
    Reference< XControl > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlImageControlModel" ) ),
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( rName ) );
        xControl = Reference< XControl >( mxDialogControlContainer->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}

// -----------------------------------------------------------------------------

void UnoDialog::setControlProperty( const OUString& rControlName, const OUString& rPropertyName, const Any& rPropertyValue )
{
    try
    {
        if ( mxDialogModelNameAccess->hasByName( rControlName ) )
        {
            Reference< XPropertySet > xPropertySet( mxDialogModelNameAccess->getByName( rControlName ), UNO_QUERY_THROW );
            xPropertySet->setPropertyValue( rPropertyName, rPropertyValue );
        }
    }
    catch ( Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

sal_Int32 UnoDialog::getMapsFromPixels( sal_Int32 nPixels ) const
{
    double dMaps = 0;
    try
    {
        sal_Int32 nMapWidth = 0;
        const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
        if ( mxDialogModelPropertySet->getPropertyValue( sWidth  ) >>= nMapWidth )
        {
            Reference< XWindow > xWindow( mxDialog, UNO_QUERY_THROW );
            double pxWidth = xWindow->getPosSize().Width;
            double mapRatio = ( pxWidth / nMapWidth );
            dMaps = nPixels / mapRatio;
        }
    }
    catch ( Exception& )
    {
    }
    return static_cast< sal_Int32 >( dMaps );
}

// -----------------------------------------------------------------------------

Any UnoDialog::getControlProperty( const OUString& rControlName, const OUString& rPropertyName )
{
    Any aRet;
    try
    {
        if ( mxDialogModelNameAccess->hasByName( rControlName ) )
        {
            Reference< XPropertySet > xPropertySet( mxDialogModelNameAccess->getByName( rControlName ), UNO_QUERY_THROW );
            aRet = xPropertySet->getPropertyValue( rPropertyName );
        }
    }
    catch ( Exception& )
    {
    }
    return aRet;
}

// -----------------------------------------------------------------------------

void UnoDialog::enableControl( const OUString& rControlName )
{
    const OUString sEnabled( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) );
    setControlProperty( rControlName, sEnabled, Any( sal_True ) );
}

// -----------------------------------------------------------------------------

void UnoDialog::disableControl( const OUString& rControlName )
{
    const OUString sEnabled( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) );
    setControlProperty( rControlName, sEnabled, Any( sal_False ) );
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
