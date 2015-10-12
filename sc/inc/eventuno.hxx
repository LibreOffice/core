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

#ifndef INCLUDED_SC_INC_EVENTUNO_HXX
#define INCLUDED_SC_INC_EVENTUNO_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>

#include "global.hxx"

class ScDocShell;

class ScSheetEventsObj : public cppu::WeakImplHelper<
                                        com::sun::star::container::XNameReplace,
                                        com::sun::star::lang::XServiceInfo>,
                                    public SfxListener
{
private:
    ScDocShell*             mpDocShell;
    SCTAB                   mnTab;

public:
                            ScSheetEventsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScSheetEventsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
