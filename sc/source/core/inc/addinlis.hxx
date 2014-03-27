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

#ifndef SC_ADDINLIS_HXX
#define SC_ADDINLIS_HXX

#include "adiasync.hxx"
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>

class ScDocument;

class ScAddInListener : public cppu::WeakImplHelper2<
                            com::sun::star::sheet::XResultListener,
                            com::sun::star::lang::XServiceInfo >,
                        public SvtBroadcaster
{
private:
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVolRes;
    com::sun::star::uno::Any aResult;
    ScAddInDocs* pDocs; // documents where this is used

    static ::std::list<ScAddInListener*> aAllListeners;

    // always allocated via CreateListener
    ScAddInListener( com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVR,
                    ScDocument* pD );

public:
    virtual ~ScAddInListener();

    // create Listener and put it into global list
    static ScAddInListener* CreateListener(
                                com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult> xVR,
                                ScDocument* pDoc );

    static ScAddInListener*Get( com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XVolatileResult> xVR );

    static void RemoveDocument( ScDocument* pDocument );

    bool HasDocument( ScDocument* pDoc ) const
         { return pDocs->find( pDoc ) != pDocs->end(); }

    void AddDocument( ScDocument* pDoc )
         { pDocs->insert( pDoc ); }

    const com::sun::star::uno::Any& GetResult() const
          { return aResult; }

    // XResultListener
    virtual void SAL_CALL modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
                              throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                              throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                         throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // SC_ADDINLIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
