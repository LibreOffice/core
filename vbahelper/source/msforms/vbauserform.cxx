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
#include <vbahelper/helperdecl.hxx>
#include "vbauserform.hxx"
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include "vbacontrols.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// some little notes
// XDialog implementation has the following interesting bits
// a Controls property ( which is an array of the container controls )
//   each item in the controls array is a XControl, where the model is
//   basically a property bag
// additionally the XDialog instance has itself a model
//     this model has a ControlModels ( array of models ) property
//     the models in ControlModels can be accessed by name
// also the XDialog is a XControl ( to access the model above

ScVbaUserForm::ScVbaUserForm( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& xContext ) throw ( lang::IllegalArgumentException, uno::RuntimeException ) :  ScVbaUserForm_BASE( getXSomethingFromArgs< XHelperInterface >( aArgs, 0 ), xContext, getXSomethingFromArgs< uno::XInterface >( aArgs, 1 ), getXSomethingFromArgs< frame::XModel >( aArgs, 2 ), static_cast< ooo::vba::AbstractGeometryAttributes* >(nullptr) ),  mbDispose( true )
{
    m_xDialog.set( m_xControl, uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControl > xControl( m_xDialog, uno::UNO_QUERY_THROW );
    m_xProps.set( xControl->getModel(), uno::UNO_QUERY_THROW );
    setGeometryHelper( new UserFormGeometryHelper( xContext, xControl, 0.0, 0.0 ) );
    if ( aArgs.getLength() >= 4 )
        aArgs[ 3 ] >>= m_sLibName;
}

ScVbaUserForm::~ScVbaUserForm()
{
}

void SAL_CALL
ScVbaUserForm::Show(  ) throw (uno::RuntimeException, std::exception)
{
    SAL_INFO("vbahelper", "ScVbaUserForm::Show(  )");
    short aRet = 0;
    mbDispose = true;

    if ( m_xDialog.is() )
    {
        // try to center dialog on model window
        if( m_xModel.is() ) try
        {
            uno::Reference< frame::XController > xController( m_xModel->getCurrentController(), uno::UNO_SET_THROW );
            uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW );
            uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW );
            awt::Rectangle aPosSize = xWindow->getPosSize();    // already in pixel

            uno::Reference< awt::XControl > xControl( m_xDialog, uno::UNO_QUERY_THROW );
            uno::Reference< awt::XWindow > xControlWindow( xControl->getPeer(), uno::UNO_QUERY_THROW );
            xControlWindow->setPosSize(static_cast<sal_Int32>((aPosSize.Width - getWidth()) / 2.0), static_cast<sal_Int32>((aPosSize.Height - getHeight()) / 2.0), 0, 0, awt::PosSize::POS );
        }
        catch( uno::Exception& )
        {
        }

        aRet = m_xDialog->execute();
    }
    SAL_INFO("vbahelper", "ScVbaUserForm::Show() execute returned " << aRet);
    if ( mbDispose )
    {
        try
        {
            uno::Reference< lang::XComponent > xComp( m_xDialog, uno::UNO_QUERY_THROW );
            m_xDialog = nullptr;
            xComp->dispose();
            mbDispose = false;
        }
        catch( uno::Exception& )
        {
        }
    }
}

OUString SAL_CALL
ScVbaUserForm::getCaption() throw (uno::RuntimeException, std::exception)
{
    OUString sCaption;
    m_xProps->getPropertyValue( "Title" ) >>= sCaption;
    return sCaption;
}
void
ScVbaUserForm::setCaption( const OUString& _caption ) throw (uno::RuntimeException, std::exception)
{
    m_xProps->setPropertyValue( "Title", uno::makeAny( _caption ) );
}

sal_Bool SAL_CALL
ScVbaUserForm::getVisible() throw (uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XControl > xControl( m_xDialog, uno::UNO_QUERY_THROW );
    uno::Reference< awt::XWindow2 > xControlWindow( xControl->getPeer(), uno::UNO_QUERY_THROW );
    return xControlWindow->isVisible();
}

void SAL_CALL
ScVbaUserForm::setVisible( sal_Bool bVis ) throw (uno::RuntimeException, std::exception)
{
    if ( bVis )
        Show();
    else
        Hide();
}

double SAL_CALL ScVbaUserForm::getInnerWidth() throw (uno::RuntimeException, std::exception)
{
    return mpGeometryHelper->getInnerWidth();
}

void SAL_CALL ScVbaUserForm::setInnerWidth( double fInnerWidth ) throw (uno::RuntimeException, std::exception)
{
    mpGeometryHelper->setInnerWidth( fInnerWidth );
}

double SAL_CALL ScVbaUserForm::getInnerHeight() throw (uno::RuntimeException, std::exception)
{
    return mpGeometryHelper->getInnerHeight();
}

void SAL_CALL ScVbaUserForm::setInnerHeight( double fInnerHeight ) throw (uno::RuntimeException, std::exception)
{
    mpGeometryHelper->setInnerHeight( fInnerHeight );
}

void SAL_CALL
ScVbaUserForm::Hide(  ) throw (uno::RuntimeException, std::exception)
{
    mbDispose = false;  // hide not dispose
    if ( m_xDialog.is() )
        m_xDialog->endExecute();
}

void SAL_CALL
ScVbaUserForm::RePaint(  ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
    // do nothing
}

void SAL_CALL
ScVbaUserForm::UnloadObject(  ) throw (uno::RuntimeException, std::exception)
{
    mbDispose = true;
    if ( m_xDialog.is() )
        m_xDialog->endExecute();
}

OUString
ScVbaUserForm::getServiceImplName()
{
    return OUString("ScVbaUserForm");
}

uno::Sequence< OUString >
ScVbaUserForm::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.UserForm";
    }
    return aServiceNames;
}

uno::Reference< beans::XIntrospectionAccess > SAL_CALL
ScVbaUserForm::getIntrospection(  ) throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< beans::XIntrospectionAccess >();
}

uno::Any SAL_CALL
ScVbaUserForm::invoke( const OUString& /*aFunctionName*/, const uno::Sequence< uno::Any >& /*aParams*/, uno::Sequence< ::sal_Int16 >& /*aOutParamIndex*/, uno::Sequence< uno::Any >& /*aOutParam*/ ) throw (lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException, std::exception)
{
    throw uno::RuntimeException(); // unsupported operation
}

void SAL_CALL
ScVbaUserForm::setValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException, std::exception)
{
    uno::Any aObject = getValue( aPropertyName );

    // in case the dialog is already closed the VBA implementation should not throw exceptions
    if ( aObject.hasValue() )
    {
        // The Object *must* support XDefaultProperty here because getValue will
        // only return properties that are Objects ( e.g. controls )
        // e.g. Userform1.aControl = something
        // 'aControl' has to support XDefaultProperty to make sense here
        uno::Reference< script::XDefaultProperty > xDfltProp( aObject, uno::UNO_QUERY_THROW );
        OUString aDfltPropName = xDfltProp->getDefaultPropertyName();
        uno::Reference< beans::XIntrospectionAccess > xUnoAccess( getIntrospectionAccess( aObject ) );
        uno::Reference< beans::XPropertySet > xPropSet( xUnoAccess->queryAdapter( cppu::UnoType<beans::XPropertySet>::get()), uno::UNO_QUERY_THROW );
        xPropSet->setPropertyValue( aDfltPropName, aValue );
    }
}

uno::Reference< awt::XControl >
ScVbaUserForm::nestedSearch( const OUString& aPropertyName, uno::Reference< awt::XControlContainer >& xContainer )
{
    uno::Reference< awt::XControl > xControl = xContainer->getControl( aPropertyName );
    if ( !xControl.is() )
    {
        uno::Sequence< uno::Reference< awt::XControl > > aControls = xContainer->getControls();
        const uno::Reference< awt::XControl >* pCtrl = aControls.getConstArray();
        const uno::Reference< awt::XControl >* pCtrlsEnd = pCtrl + aControls.getLength();

        for ( ; pCtrl < pCtrlsEnd; ++pCtrl )
        {
            uno::Reference< awt::XControlContainer > xC( *pCtrl, uno::UNO_QUERY );
            if ( xC.is() )
            {
                xControl.set( nestedSearch( aPropertyName, xC ) );
                if ( xControl.is() )
                    break;
            }
        }
    }
    return xControl;
}

uno::Any SAL_CALL
ScVbaUserForm::getValue( const OUString& aPropertyName ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    uno::Any aResult;

    // in case the dialog is already closed the VBA implementation should not throw exceptions
    if ( m_xDialog.is() )
    {
        uno::Reference< awt::XControl > xDialogControl( m_xDialog, uno::UNO_QUERY_THROW );
        uno::Reference< awt::XControlContainer > xContainer( m_xDialog, uno::UNO_QUERY_THROW );
        uno::Reference< awt::XControl > xControl = nestedSearch( aPropertyName, xContainer );
        xContainer->getControl( aPropertyName );
        if ( xControl.is() )
        {
            uno::Reference< msforms::XControl > xVBAControl = ScVbaControlFactory::createUserformControl( mxContext, xControl, xDialogControl, m_xModel, mpGeometryHelper->getOffsetX(), mpGeometryHelper->getOffsetY() );
            ScVbaControl* pControl  = dynamic_cast< ScVbaControl* >( xVBAControl.get() );
            if (pControl && !m_sLibName.isEmpty())
                pControl->setLibraryAndCodeName( m_sLibName.concat( "." ).concat( getName() ) );
            aResult = uno::makeAny( xVBAControl );
        }
    }

    return aResult;
}

sal_Bool SAL_CALL
ScVbaUserForm::hasMethod( const OUString& /*aName*/ ) throw (uno::RuntimeException, std::exception)
{
    return false;
}
uno::Any SAL_CALL
ScVbaUserForm::Controls( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    // if the dialog already closed we should do nothing, but the VBA will call methods of the Controls objects
    // thus we have to provide a dummy object in this case
    uno::Reference< awt::XControl > xDialogControl( m_xDialog, uno::UNO_QUERY );
    uno::Reference< XCollection > xControls( new ScVbaControls( this, mxContext, xDialogControl, m_xModel, mpGeometryHelper->getOffsetX(), mpGeometryHelper->getOffsetY() ) );
    if ( index.hasValue() )
        return xControls->Item( index, uno::Any() );
    return uno::makeAny( xControls );
}

sal_Bool SAL_CALL
ScVbaUserForm::hasProperty( const OUString& aName ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XControl > xControl( m_xDialog, uno::UNO_QUERY );

    SAL_INFO("vbahelper", "ScVbaUserForm::hasProperty(" << aName << ") " << xControl.is() );
    if ( xControl.is() )
    {
        uno::Reference< beans::XPropertySet > xDlgProps( xControl->getModel(), uno::UNO_QUERY );
        if ( xDlgProps.is() )
        {
            uno::Reference< container::XNameContainer > xAllChildren( xDlgProps->getPropertyValue( "AllDialogChildren" ), uno::UNO_QUERY_THROW );
            bool bRes =  xAllChildren->hasByName( aName );
            SAL_INFO("vbahelper", "ScVbaUserForm::hasProperty(" << aName << ") " << xAllChildren.is() << " ---> " << bRes );
            return bRes;
        }
    }
    return false;
}

namespace userform
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaUserForm, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaUserForm",
    "ooo.vba.msforms.UserForm" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
