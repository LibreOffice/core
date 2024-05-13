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

#include <cppuhelper/supportsservice.hxx>

/// anonymous implementation namespace
namespace dlgprov {

using namespace ::com::sun::star;
using namespace lang;
using namespace uno;
using namespace beans;


DialogModelProvider::DialogModelProvider(Reference< XComponentContext > const & context) :
    m_xContext(context)
{}

// lang::XInitialization:
void SAL_CALL DialogModelProvider::initialize(const css::uno::Sequence< uno::Any > & aArguments)
{
    if ( aArguments.getLength() != 1 )
        return;

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
            m_xDialogModel.set( dlgprov::lcl_createDialogModel( m_xContext, xInput , xModel, xStringResourceManager, aDialogSourceURLAny  ), UNO_SET_THROW);
            m_xDialogModelProp.set(m_xDialogModel, UNO_QUERY_THROW);
        }
    }
    catch( Exception& )
    {}
    //m_sURL = sURL;
}

// container::XElementAccess:
uno::Type SAL_CALL DialogModelProvider::getElementType()
{
    return m_xDialogModel->getElementType();
}

sal_Bool SAL_CALL DialogModelProvider::hasElements()
{
    return m_xDialogModel->hasElements();
}

// container::XNameAccess:
uno::Any SAL_CALL DialogModelProvider::getByName(const OUString & aName)
{
    return m_xDialogModel->getByName(aName);
}

css::uno::Sequence< OUString > SAL_CALL DialogModelProvider::getElementNames()
{
    return m_xDialogModel->getElementNames();
}

sal_Bool SAL_CALL DialogModelProvider::hasByName(const OUString & aName)
{
    return m_xDialogModel->hasByName(aName);
}

// container::XNameReplace:
void SAL_CALL DialogModelProvider::replaceByName(const OUString & aName, const uno::Any & aElement)
{
    m_xDialogModel->replaceByName(aName,aElement);
}

// container::XNameContainer:
void SAL_CALL DialogModelProvider::insertByName(const OUString & aName, const uno::Any & aElement)
{
    m_xDialogModel->insertByName(aName,aElement);
}

void SAL_CALL DialogModelProvider::removeByName(const OUString & aName)
{
    m_xDialogModel->removeByName(aName);
}
uno::Reference< beans::XPropertySetInfo > SAL_CALL DialogModelProvider::getPropertySetInfo(  )
{
    return m_xDialogModelProp->getPropertySetInfo();
}
void SAL_CALL DialogModelProvider::setPropertyValue( const OUString&, const uno::Any& )
{
}
uno::Any SAL_CALL DialogModelProvider::getPropertyValue( const OUString& PropertyName )
{
    return m_xDialogModelProp->getPropertyValue(PropertyName);
}
void SAL_CALL DialogModelProvider::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& )
{
}
void SAL_CALL DialogModelProvider::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& )
{
}
void SAL_CALL DialogModelProvider::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& )
{
}
void SAL_CALL DialogModelProvider::removeVetoableChangeListener( const OUString& ,const uno::Reference< beans::XVetoableChangeListener >& )
{
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL DialogModelProvider::getImplementationName()
{
    return u"com.sun.star.comp.scripting.DialogModelProvider"_ustr;
}

sal_Bool SAL_CALL DialogModelProvider::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL DialogModelProvider::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.UnoControlDialogModelProvider"_ustr };
}

} // closing anonymous implementation namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_DialogModelProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new dlgprov::DialogModelProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
