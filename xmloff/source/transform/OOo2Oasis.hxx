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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_OOO2OASIS_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_OOO2OASIS_HXX

#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include "ActionMapTypesOOo.hxx"
#include "TransformerBase.hxx"

class XMLTransformerOOoEventMap_Impl;

class OOo2OasisTransformer :
        public XMLTransformerBase,
        public ::com::sun::star::document::XImporter,
        public ::com::sun::star::document::XFilter
{
    OUString m_aImplName;
    OUString m_aSubServiceName;

    XMLTransformerActions       *m_aActions[MAX_OOO_ACTIONS];
    XMLTransformerOOoEventMap_Impl *m_pEventMap;
protected:

    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const OUString& rQName,
                                         bool bPersistent=false ) SAL_OVERRIDE;

    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n ) SAL_OVERRIDE;

public:
    OOo2OasisTransformer( const sal_Char *pImplName=0,
                          const sal_Char *pSubServiceName=0 ) throw();
    virtual ~OOo2OasisTransformer() throw();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // XInterface

    // (XInterface methods need to be implemented to disambigouate
    // between those inherited through XMLTransformerBase and
    // the new interfaces).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL acquire(  ) throw () SAL_OVERRIDE
        { XMLTransformerBase::acquire(); };

    virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE
        { XMLTransformerBase::release(); };

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void SAL_CALL Initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual OUString GetEventName( const OUString& rName,
                                            bool bForm = false ) SAL_OVERRIDE;
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_OOO2OASIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
