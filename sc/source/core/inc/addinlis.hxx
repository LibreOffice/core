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

#pragma once

#include <memory>
#include "adiasync.hxx"
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::sheet { class XVolatileResult; }
namespace rtl { template <class reference_type> class Reference; }

class ScDocument;

class ScAddInListener final : public cppu::WeakImplHelper<
                            css::sheet::XResultListener,
                            css::lang::XServiceInfo >,
                        public SvtBroadcaster
{
private:
    css::uno::Reference<css::sheet::XVolatileResult> xVolRes;
    css::uno::Any aResult;
    std::unique_ptr<ScAddInDocs> pDocs; // documents where this is used

    static ::std::vector<rtl::Reference<ScAddInListener>> aAllListeners;

    // always allocated via CreateListener
    ScAddInListener( css::uno::Reference<css::sheet::XVolatileResult> const & xVR,
                    ScDocument* pD );

public:
    virtual ~ScAddInListener() override;

    // create Listener and put it into global list
    static ScAddInListener* CreateListener(
                                const css::uno::Reference<css::sheet::XVolatileResult>& xVR,
                                ScDocument* pDoc );

    static ScAddInListener* Get( const css::uno::Reference<css::sheet::XVolatileResult>& xVR );

    static void RemoveDocument( ScDocument* pDocument );

    bool HasDocument( ScDocument* pDoc ) const
         { return pDocs->find( pDoc ) != pDocs->end(); }

    void AddDocument( ScDocument* pDoc )
         { pDocs->insert( pDoc ); }

    const css::uno::Any& GetResult() const
          { return aResult; }

    // XResultListener
    virtual void SAL_CALL modified( const css::sheet::ResultEvent& aEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
