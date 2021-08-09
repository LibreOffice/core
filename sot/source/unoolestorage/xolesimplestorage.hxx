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

#ifndef INCLUDED_SOT_SOURCE_UNOOLESTORAGE_XOLESIMPLESTORAGE_HXX
#define INCLUDED_SOT_SOURCE_UNOOLESTORAGE_XOLESIMPLESTORAGE_HXX

#include <sal/config.h>

#include <memory>

#include <com/sun/star/embed/XOLESimpleStorage.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <osl/mutex.hxx>

#include <memory>

namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XStream; }
namespace com::sun::star::lang { class XEventListener; }
namespace com::sun::star::uno { class XComponentContext; }
namespace comphelper { class OInterfaceContainerHelper2; }

class BaseStorage;
class SvStream;

class OLESimpleStorage : public cppu::WeakImplHelper<css::embed::XOLESimpleStorage, css::lang::XServiceInfo>
{
    ::osl::Mutex m_aMutex;

    bool m_bDisposed;

    css::uno::Reference< css::io::XStream > m_xStream;
    css::uno::Reference< css::io::XStream > m_xTempStream;
    std::unique_ptr<SvStream> m_pStream;
    std::unique_ptr<BaseStorage> m_pStorage;

    ::comphelper::OInterfaceContainerHelper2* m_pListenersContainer; // list of listeners
    css::uno::Reference<css::uno::XComponentContext> m_xContext;

    bool m_bNoTemporaryCopy;

    void UpdateOriginal_Impl();

    /// @throws css::uno::Exception
    static void InsertInputStreamToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const css::uno::Reference< css::io::XInputStream >& xInputStream );

    /// @throws css::uno::Exception
    static void InsertNameAccessToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const css::uno::Reference< css::container::XNameAccess >& xNameAccess );

public:

    OLESimpleStorage(css::uno::Reference<css::uno::XComponentContext> const & xContext,
            css::uno::Sequence<css::uno::Any> const &arguments);

    virtual ~OLESimpleStorage() override;

    //  XNameContainer

    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;

    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    virtual css::uno::Type SAL_CALL getElementType() override;

    virtual sal_Bool SAL_CALL hasElements() override;

    //  XComponent

    virtual void SAL_CALL dispose() final override;

    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    //  XTransactedObject

    virtual void SAL_CALL commit() override;

    virtual void SAL_CALL revert() override;

    //  XClassifiedObject

    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getClassID() override;

    virtual OUString SAL_CALL getClassName() override;

    virtual void SAL_CALL setClassInfo( const css::uno::Sequence< ::sal_Int8 >& aClassID,
                                        const OUString& sClassName ) override;

    //  XServiceInfo

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
