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


#include "unodialog.hxx"

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>


using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

UnoDialog::UnoDialog( const Reference< XComponentContext > &rxContext, Reference< XFrame > const & rxFrame ) :
    mxContext( rxContext ),
    mxController( rxFrame->getController() ),
    mxDialogModel( mxContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.awt.UnoControlDialogModel", mxContext ), UNO_SET_THROW ),
    mxDialogModelMultiPropertySet( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelMSF( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameContainer( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameAccess( mxDialogModel, UNO_QUERY_THROW ),
    mxControlModel( mxDialogModel, UNO_QUERY_THROW ),
    mxDialog( UnoControlDialog::create(rxContext) ),
    mxControl( mxDialog, UNO_QUERY_THROW ),
    mbStatus( false )
{
    mxControl->setModel( mxControlModel );

    Reference< XFrame > xFrame( mxController->getFrame() );
    Reference< XWindow > xContainerWindow( xFrame->getContainerWindow() );
    Reference< XWindowPeer > xWindowPeer( xContainerWindow, UNO_QUERY_THROW );

    // set the main loop handle to update GUI while busy
    Reference< XToolkit > xToolkit( Toolkit::create( mxContext ), UNO_QUERY_THROW  );
    mxReschedule.set( xToolkit, UNO_QUERY );

    // allocate the real window resources
    mxDialog->createPeer(xToolkit,
        xWindowPeer.is() ? xWindowPeer : xToolkit->getDesktopWindow());
}

UnoDialog::~UnoDialog()
{
    // free the resources
    Reference<XComponent> xDialogComponent(mxDialog, UNO_QUERY_THROW);
    xDialogComponent->dispose();
}

void UnoDialog::execute()
{
    mxDialog->setEnable( true );
    mxDialog->setVisible( true );
    mxDialog->execute();
}

void UnoDialog::endExecute( bool bStatus )
{
    mbStatus = bStatus;
    mxDialog->endExecute();
}

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


void UnoDialog::setVisible( const OUString& rName, bool bVisible )
{
    try
    {
        Reference< XInterface > xControl( mxDialog->getControl( rName ), UNO_QUERY_THROW );
        Reference< XWindow > xWindow( xControl, UNO_QUERY_THROW );
        xWindow->setVisible( bVisible );
    }
    catch ( Exception& )
    {
    }
}


Reference< XButton > UnoDialog::insertButton( const OUString& rName, const Reference< XActionListener >& xActionListener,
            const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XButton > xButton;
    try
    {
        Reference< XInterface > xButtonModel( insertControlModel( "com.sun.star.awt.UnoControlButtonModel",
            rName, rPropertyNames, rPropertyValues ) );
        Reference< XPropertySet > xPropertySet( xButtonModel, UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xButton.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );

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


Reference< XFixedText > UnoDialog::insertFixedText( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XFixedText > xFixedText;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlFixedTextModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xFixedText.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xFixedText;
}


Reference< XCheckBox > UnoDialog::insertCheckBox( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XCheckBox > xCheckBox;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlCheckBoxModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xCheckBox.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xCheckBox;
}


Reference< XControl > UnoDialog::insertFormattedField( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XControl > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlFormattedFieldModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xControl.set( mxDialog->getControl( rName ), UNO_SET_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}


Reference< XComboBox > UnoDialog::insertComboBox( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XComboBox > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlComboBoxModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xControl.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}


Reference< XRadioButton > UnoDialog::insertRadioButton( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XRadioButton > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlRadioButtonModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xControl.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}


Reference< XListBox > UnoDialog::insertListBox( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XListBox > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlListBoxModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xControl.set( mxDialog->getControl( rName ), UNO_QUERY_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}


Reference< XControl > UnoDialog::insertImage( const OUString& rName, const Sequence< OUString >& rPropertyNames, const Sequence< Any >& rPropertyValues )
{
    Reference< XControl > xControl;
    try
    {
        Reference< XPropertySet > xPropertySet( insertControlModel( "com.sun.star.awt.UnoControlImageControlModel",
            rName, rPropertyNames, rPropertyValues ), UNO_QUERY_THROW );
        xPropertySet->setPropertyValue("Name", Any( rName ) );
        xControl.set( mxDialog->getControl( rName ), UNO_SET_THROW );
    }
    catch ( Exception& )
    {
    }
    return xControl;
}


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


void UnoDialog::enableControl( const OUString& rControlName )
{
    setControlProperty( rControlName, "Enabled", Any( true ) );
}


void UnoDialog::disableControl( const OUString& rControlName )
{
    setControlProperty( rControlName, "Enabled", Any( false ) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
