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
    OUString m_aImplName;
    OUString m_aSubServiceName;

    XMLTransformerActions       *m_aActions[MAX_OOO_ACTIONS];
    XMLTransformerOOoEventMap_Impl *m_pEventMap;
protected:

    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const OUString& rQName,
                                         bool bPersistent=false ) override;

    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n ) override;

public:
    OOo2OasisTransformer( const sal_Char *pImplName=nullptr,
                          const sal_Char *pSubServiceName=nullptr ) throw();
    virtual ~OOo2OasisTransformer() throw();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // XInterface

    // (XInterface methods need to be implemented to disambigouate
    // between those inherited through XMLTransformerBase and
    // the new interfaces).

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { XMLTransformerBase::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { XMLTransformerBase::release(); };

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel(  ) throw (css::uno::RuntimeException, std::exception) override;

    void SAL_CALL Initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException);

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual OUString GetEventName( const OUString& rName,
                                            bool bForm = false ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_OOO2OASIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
