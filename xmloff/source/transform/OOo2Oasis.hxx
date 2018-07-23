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
        public css::document::XImporter,
        public css::document::XFilter
{
    OUString const m_aImplName;
    OUString const m_aSubServiceName;

    std::unique_ptr<XMLTransformerActions> m_aActions[MAX_OOO_ACTIONS];
    XMLTransformerOOoEventMap_Impl *m_pEventMap;
protected:

    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const OUString& rQName,
                                         bool bPersistent=false ) override;

    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n ) override;

public:
    OOo2OasisTransformer( OUString const & rImplName,
                          OUString const & rSubServiceName ) throw();
    virtual ~OOo2OasisTransformer() throw() override;

    // XInterface

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through XMLTransformerBase and
    // the new interfaces).

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { XMLTransformerBase::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { XMLTransformerBase::release(); };

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;
    virtual void SAL_CALL cancel(  ) override;

    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    void Initialize( const css::uno::Sequence< css::uno::Any >& aArguments );

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument() override;

    virtual OUString GetEventName( const OUString& rName,
                                            bool bForm = false ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_OOO2OASIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
