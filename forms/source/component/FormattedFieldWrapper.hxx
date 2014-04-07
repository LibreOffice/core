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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX

#include "FormComponent.hxx"
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>


namespace frm
{

class OEditModel;

//= OFormattedFieldWrapper

typedef ::cppu::WeakAggImplHelper3  <   ::com::sun::star::io::XPersistObject
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::util::XCloneable
                                    >   OFormattedFieldWrapper_Base;

class OFormattedFieldWrapper : public OFormattedFieldWrapper_Base
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>      m_xAggregate;

    rtl::Reference< OEditModel > m_pEditPart;
        // if we act as formatted this is used to write the EditModel part
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>     m_xFormattedPart;
    // if we act as formatted, this is the PersistObject interface of our aggregate, used
    // to read and write the FormattedModel part
    // if bActAsFormatted is false, the state is undetermined until somebody calls
    // ::read or does anything which requires a living aggregate
    static InterfaceRef createFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory, bool bActAsFormatted);

private:
    OFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);

protected:
    virtual ~OFormattedFieldWrapper();

    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

public:
    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OFormattedFieldWrapper, OWeakAggObject)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    /// ensure we're in a defined state, which means a FormattedModel _OR_ an EditModel
    void ensureAggregate();
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
