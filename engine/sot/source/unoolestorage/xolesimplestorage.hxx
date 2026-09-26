/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/embed/XOLESimpleStorage.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <mutex>

namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XStream; }
namespace com::sun::star::lang { class XEventListener; }
namespace cpo::uno { class XComponentContext; }

class BaseStorage;
class SvStream;

class OLESimpleStorage : public cppu::WeakImplHelper<css::embed::XOLESimpleStorage, css::lang::XServiceInfo>
{
    std::mutex m_aMutex;

    bool m_bDisposed;

    cpo::uno::Reference< css::io::XStream > m_xStream;
    cpo::uno::Reference< css::io::XStream > m_xTempStream;
    std::unique_ptr<SvStream> m_pStream;
    std::unique_ptr<BaseStorage> m_pStorage;

    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aListenersContainer; // list of listeners
    cpo::uno::Reference<cpo::uno::XComponentContext> m_xContext;

    bool m_bNoTemporaryCopy;

    void UpdateOriginal_Impl();

    /// @throws cpo::uno::Exception
    static void InsertInputStreamToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const cpo::uno::Reference< css::io::XInputStream >& xInputStream );

    /// @throws cpo::uno::Exception
    static void InsertNameAccessToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const cpo::uno::Reference< css::container::XNameAccess >& xNameAccess );

public:

    OLESimpleStorage(cpo::uno::Reference<cpo::uno::XComponentContext> xContext,
            cpo::uno::Sequence<cpo::uno::Any> const &arguments);

    virtual ~OLESimpleStorage() override;

    //  XNameContainer

    virtual void insertByName( const OUString& aName, const cpo::uno::Any& aElement ) override;

    virtual void removeByName( const OUString& Name ) override;

    virtual void replaceByName( const OUString& aName, const cpo::uno::Any& aElement ) override;

    virtual cpo::uno::Any getByName( const OUString& aName ) override;

    virtual cpo::uno::Sequence< OUString > getElementNames() override;

    virtual bool hasByName( const OUString& aName ) override;

    virtual cpo::uno::Type getElementType() override;

    virtual bool hasElements() override;

    //  XComponent

    virtual void dispose() final override;

    virtual void addEventListener(
            const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;

    virtual void removeEventListener(
            const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;

    //  XTransactedObject

    virtual void commit() override;

    virtual void revert() override;

    //  XClassifiedObject

    virtual cpo::uno::Sequence< ::sal_Int8 > getClassID() override;

    virtual OUString getClassName() override;

    virtual void setClassInfo( const cpo::uno::Sequence< ::sal_Int8 >& aClassID,
                                        const OUString& sClassName ) override;

    //  XServiceInfo

    virtual OUString getImplementationName() override;

    virtual bool supportsService( const OUString& ServiceName ) override;

    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
