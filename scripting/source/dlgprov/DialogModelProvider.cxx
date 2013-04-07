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

#include "DialogModelProvider.hxx"
#include "dlgprov.hxx"
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>


// component helper namespace
namespace comp_DialogModelProvider {

using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;


// component and service helper functions:
OUString SAL_CALL _getImplementationName();
css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace



/// anonymous implementation namespace
namespace dlgprov {

using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;


DialogModelProvider::DialogModelProvider(Reference< XComponentContext > const & context) :
    m_xContext(context)
{}

// lang::XInitialization:
void SAL_CALL DialogModelProvider::initialize(const css::uno::Sequence< uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception)
{
    if ( aArguments.getLength() == 1 )
    {
        OUString sURL;
        if ( !( aArguments[ 0 ] >>= sURL ))
            throw css::lang::IllegalArgumentException();
         // Try any other URL with SimpleFileAccess
        Reference< ucb::XSimpleFileAccess3 > xSFI = ucb::SimpleFileAccess::create(m_xContext);

        try
        {
            Reference< io::XInputStream > xInput = xSFI->openFileRead( sURL );
            Reference< resource::XStringResourceManager > xStringResourceManager;
            if ( xInput.is() )
            {
                xStringResourceManager = dlgprov::lcl_getStringResourceManager(m_xContext,sURL);
                Any aDialogSourceURLAny;
                aDialogSourceURLAny <<= sURL;

                Reference< frame::XModel > xModel;
                m_xDialogModel.set( dlgprov::lcl_createDialogModel( m_xContext, xInput , xModel, xStringResourceManager, aDialogSourceURLAny  ), UNO_QUERY_THROW);
                m_xDialogModelProp.set(m_xDialogModel, UNO_QUERY_THROW);
            }
        }
        catch( Exception& )
        {}
        //m_sURL = sURL;
    }
}

// container::XElementAccess:
uno::Type SAL_CALL DialogModelProvider::getElementType() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->getElementType();
}

::sal_Bool SAL_CALL DialogModelProvider::hasElements() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->hasElements();
}

// container::XNameAccess:
uno::Any SAL_CALL DialogModelProvider::getByName(const OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    return m_xDialogModel->getByName(aName);
}

css::uno::Sequence< OUString > SAL_CALL DialogModelProvider::getElementNames() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->getElementNames();
}

::sal_Bool SAL_CALL DialogModelProvider::hasByName(const OUString & aName) throw (css::uno::RuntimeException)
{
    return m_xDialogModel->hasByName(aName);
}

// container::XNameReplace:
void SAL_CALL DialogModelProvider::replaceByName(const OUString & aName, const uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    m_xDialogModel->replaceByName(aName,aElement);
}

// container::XNameContainer:
void SAL_CALL DialogModelProvider::insertByName(const OUString & aName, const uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException)
{
    m_xDialogModel->insertByName(aName,aElement);
}

void SAL_CALL DialogModelProvider::removeByName(const OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    m_xDialogModel->removeByName(aName);
}
uno::Reference< beans::XPropertySetInfo > SAL_CALL DialogModelProvider::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    return m_xDialogModelProp->getPropertySetInfo();
}
void SAL_CALL DialogModelProvider::setPropertyValue( const OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
}
uno::Any SAL_CALL DialogModelProvider::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_xDialogModelProp->getPropertyValue(PropertyName);
}
void SAL_CALL DialogModelProvider::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::removeVetoableChangeListener( const OUString& ,const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL DialogModelProvider::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_DialogModelProvider::_getImplementationName();
}

::sal_Bool SAL_CALL DialogModelProvider::supportsService(OUString const & serviceName) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > serviceNames = comp_DialogModelProvider::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< OUString > SAL_CALL DialogModelProvider::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_DialogModelProvider::_getSupportedServiceNames();
}

} // closing anonymous implementation namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
