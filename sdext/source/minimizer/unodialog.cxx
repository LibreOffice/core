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

#include "unodialog.hxx"

#include <com/sun/star/awt/MessageBoxButtons.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

// -------------
// - UnoDialog -
// -------------

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;

using ::rtl::OUString;

UnoDialog::UnoDialog(
    const Reference< XComponentContext > &rxContext,
    const Reference< XWindowPeer >& rxParent ) :
    mxContext( rxContext ),
    mxParent( rxParent ),
    mxDialogModel( mxContext->getServiceManager()->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.UnoControlDialogModel" ) ), mxContext ), UNO_QUERY_THROW ),
    mxDialogModelMultiPropertySet( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelPropertySet( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelMSF( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameContainer( mxDialogModel, UNO_QUERY_THROW ),
    mxDialogModelNameAccess( mxDialogModel, UNO_QUERY_THROW ),
    mxControlModel( mxDialogModel, UNO_QUERY_THROW ),
    mxDialog( mxContext->getServiceManager()->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.UnoControlDialog" ) ), mxContext ), UNO_QUERY_THROW ),
    mxControl( mxDialog, UNO_QUERY_THROW ),
    mbStatus( sal_False )
{
    OSL_TRACE("UnoDialog::UnoDialog");
    mxControl->setModel( mxControlModel );
    mxDialogControlContainer = Reference< XControlContainer >( mxDialog, UNO_QUERY_THROW );
    mxDialogWindow = Reference< XWindow >( mxDialog, UNO_QUERY );
    mxDialogWindowPeer = createWindowPeer();
}

// -----------------------------------------------------------------------------

UnoDialog::~UnoDialog()
{
    OSL_TRACE("UnoDialog::~UnoDialog");
    Reference< XComponent > xComponent( mxDialog, UNO_QUERY );
    if ( xComponent.is() )
    {
        xComponent->dispose();
    }
}

// -----------------------------------------------------------------------------

void UnoDialog::setTitle( const rtl::OUString &rTitle )
{
    if ( rTitle.getLength() )
    {
        mxDialog->setTitle( rTitle );
    }
}

void UnoDialog::execute()
{
    OSL_TRACE("UnoDialog::execute");
    mxDialogWindow->setEnable( sal_True );
    mxDialogWindow->setVisible( sal_True );
    mxDialog->execute();
}

void UnoDialog::endExecute( sal_Bool bStatus )
{
    OSL_TRACE("UnoDialog::endExecute");
    mbStatus = bStatus;
    mxDialog->endExecute();
}

void UnoDialog::centerDialog()
{
    Reference< XWindow > xParent( mxParent, UNO_QUERY );
    if ( !xParent.is() )
        return;

    Rectangle aParentPosSize = xParent->getPosSize();
    Rectangle aWinPosSize = mxDialogWindow->getPosSize();
    Point aWinPos((aParentPosSize.Width - aWinPosSize.Width) / 2,
                  (aParentPosSize.Height - aWinPosSize.Height) / 2);

    if ( ( aWinPos.X + aWinPosSize.Width ) > ( aParentPosSize.X + aParentPosSize.Width ) )
        aWinPos.X = aParentPosSize.X + aParentPosSize.Width - aWinPosSize.Width;

    if ( ( aWinPos.Y + aWinPosSize.Height ) > ( aParentPosSize.Y + aParentPosSize.Height ) )
        aWinPos.Y = aParentPosSize.Y + aParentPosSize.Height - aWinPosSize.Height;

    mxDialogWindow->setPosSize( aWinPos.X, aWinPos.Y,
                                aWinPosSize.Width,
                                aWinPosSize.Height,
                                PosSize::POS );
}

// -----------------------------------------------------------------------------

Reference< XWindowPeer > UnoDialog::createWindowPeer()
    throw ( Exception )
{
    mxDialogWindow->setVisible( sal_False );

    // reuse the parent's toolkit
    Reference< XToolkit > xToolkit;
    if ( mxParent.is() )
        xToolkit.set( mxParent->getToolkit() );

    if ( !xToolkit.is() )
         xToolkit.set( mxContext->getServiceManager()->createInstanceWithContext(
             OUString( RTL_CONSTASCII_USTRINGPARAM(
                 "com.sun.star.awt.Toolkit" ) ), mxContext ),
                    UNO_QUERY_THROW  );

    mxReschedule = Reference< XReschedule >( xToolkit, UNO_QUERY );
    mxControl->createPeer( xToolkit, mxParent );

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
        Reference< XStyleSettingsSupplier > xStyleSettingsSuppl( mxDialogWindow, UNO_QUERY_THROW );
        Reference< XStyleSettings > xStyleSettings( xStyleSettingsSuppl->getStyleSettings() );
        bHighContrast = xStyleSettings->getHighContrastMode();
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
