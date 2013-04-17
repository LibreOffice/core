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


#include "chinese_translation_unodialog.hxx"
#include "chinese_translationdialog.hxx"
#include <osl/mutex.hxx>
// header for class Application
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
// header for define RET_CANCEL
#include <vcl/msgbox.hxx>

// header for class OImplementationId
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................
using namespace ::com::sun::star;

#define SERVICE_IMPLEMENTATION_NAME OUString("com.sun.star.comp.linguistic2.ChineseTranslationDialog")
#define SERVICE_NAME OUString("com.sun.star.linguistic2.ChineseTranslationDialog")

ChineseTranslation_UnoDialog::ChineseTranslation_UnoDialog( const uno::Reference< uno::XComponentContext >& xContext )
                    : m_xCC( xContext )
                    , m_xParentWindow( 0 )
                    , m_pDialog( 0 )
                    , m_bDisposed(false)
                    , m_bInDispose(false)
                    , m_aContainerMutex()
                    , m_aDisposeEventListeners(m_aContainerMutex)
{
}

ChineseTranslation_UnoDialog::~ChineseTranslation_UnoDialog()
{
    SolarMutexGuard aSolarGuard;
    impl_DeleteDialog();
}

void ChineseTranslation_UnoDialog::impl_DeleteDialog()
{
    if( m_pDialog )
    {
        if(m_pDialog->IsInExecute())
            m_pDialog->EndDialog(RET_CANCEL);
        delete m_pDialog;
        m_pDialog = 0;
    }
}
//-------------------------------------------------------------------------
// lang::XServiceInfo

OUString SAL_CALL ChineseTranslation_UnoDialog::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

OUString ChineseTranslation_UnoDialog::getImplementationName_Static()
{
    return SERVICE_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL ChineseTranslation_UnoDialog::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString* pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if( pArray[ i ] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< OUString > SAL_CALL ChineseTranslation_UnoDialog::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ChineseTranslation_UnoDialog::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = SERVICE_NAME;
    return aSNS;
}

//-------------------------------------------------------------------------
// ui::dialogs::XExecutableDialog

void SAL_CALL ChineseTranslation_UnoDialog::setTitle( const OUString& ) throw(uno::RuntimeException)
{
    //not implemented - fell free to do so, if you do need this
}

//-------------------------------------------------------------------------
void SAL_CALL ChineseTranslation_UnoDialog::initialize( const uno::Sequence< uno::Any >& aArguments ) throw(uno::Exception, uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( m_bDisposed || m_bInDispose )
        return;

    const uno::Any* pArguments = aArguments.getConstArray();
    for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
    {
        beans::PropertyValue aProperty;
        if(*pArguments >>= aProperty)
        {
            if( aProperty.Name == "ParentWindow" )
            {
                aProperty.Value >>= m_xParentWindow;
            }
        }
    }
}

//-------------------------------------------------------------------------
sal_Int16 SAL_CALL ChineseTranslation_UnoDialog::execute() throw(uno::RuntimeException)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;
    {
        SolarMutexGuard aSolarGuard;
        if( m_bDisposed || m_bInDispose )
            return nRet;

        if( !m_pDialog )
        {
            Window* pParent = NULL;
            if( m_xParentWindow.is() )
            {
                VCLXWindow* pImplementation = VCLXWindow::GetImplementation(m_xParentWindow);
                if (pImplementation)
                    pParent = pImplementation->GetWindow();
            }
            uno::Reference< XComponent > xComp( this );
            m_pDialog = new ChineseTranslationDialog( pParent );
        }
        if( !m_pDialog )
            return nRet;
        nRet = m_pDialog->Execute();
        if(nRet==RET_OK)
           nRet=ui::dialogs::ExecutableDialogResults::OK;
    }
    return nRet;
}

//-------------------------------------------------------------------------
// lang::XComponent

void SAL_CALL ChineseTranslation_UnoDialog::dispose() throw (uno::RuntimeException)
{
    lang::EventObject aEvt;
    {
        SolarMutexGuard aSolarGuard;
        if( m_bDisposed || m_bInDispose )
            return;
        m_bInDispose = true;

        impl_DeleteDialog();
        m_xParentWindow = 0;
        m_bDisposed = true;

        aEvt.Source = static_cast< XComponent * >( this );
    }
    if( m_aDisposeEventListeners.getLength() )
        m_aDisposeEventListeners.disposeAndClear( aEvt );
}

void SAL_CALL ChineseTranslation_UnoDialog::addEventListener( const uno::Reference< lang::XEventListener > & xListener ) throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( m_bDisposed || m_bInDispose )
        return;
    m_aDisposeEventListeners.addInterface( xListener );
}

void SAL_CALL ChineseTranslation_UnoDialog::removeEventListener( const uno::Reference< lang::XEventListener > & xListener ) throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( m_bDisposed || m_bInDispose )
        return;
    m_aDisposeEventListeners.removeInterface( xListener );
}

//-------------------------------------------------------------------------
// XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL ChineseTranslation_UnoDialog::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    return 0;
}
void SAL_CALL ChineseTranslation_UnoDialog::setPropertyValue( const OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only read only properties
    throw beans::PropertyVetoException();
}
uno::Any SAL_CALL ChineseTranslation_UnoDialog::getPropertyValue( const OUString& rPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;

    sal_Bool bDirectionToSimplified = sal_True;
    sal_Bool bUseCharacterVariants = sal_False;
    sal_Bool bTranslateCommonTerms = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        if( m_bDisposed || m_bInDispose || !m_pDialog )
            return aRet;
        m_pDialog->getSettings( bDirectionToSimplified, bTranslateCommonTerms );
    }

    if( rPropertyName.equals( "IsDirectionToSimplified" ) )
    {
        aRet <<= bDirectionToSimplified;
    }
    else if( rPropertyName.equals( "IsUseCharacterVariants" ) )
    {
        aRet <<= bUseCharacterVariants;
    }
    else if( rPropertyName.equals( "IsTranslateCommonTerms" ) )
    {
        aRet <<= bTranslateCommonTerms;
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
    return aRet;

}
void SAL_CALL ChineseTranslation_UnoDialog::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}
void SAL_CALL ChineseTranslation_UnoDialog::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    //only not bound properties -> ignore listener
}

//.............................................................................
} //end namespace
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
