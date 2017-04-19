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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_TCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_TCONNECTION_HXX

#include <rtl/textenc.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/compbase.hxx>
#include "propertyids.hxx"
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include "resource/sharedresources.hxx"

namespace connectivity
{
    typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XConnection,
                                               css::sdbc::XWarningsSupplier,
                                               css::lang::XServiceInfo,
                                               css::lang::XUnoTunnel
                                           > OMetaConnection_BASE;

    class OOO_DLLPUBLIC_DBTOOLS OMetaConnection : public OMetaConnection_BASE
    {
    protected:
        ::osl::Mutex                    m_aMutex;
        css::uno::Sequence< css::beans::PropertyValue >
                                        m_aConnectionInfo;
        connectivity::OWeakRefArray     m_aStatements;  //  vector containing a list
                                                        //  of all the Statement objects
                                                        //  for this Connection
        OUString                        m_sURL;
        rtl_TextEncoding                m_nTextEncoding; // the encoding which is used for all text conversions
        css::uno::WeakReference< css::sdbc::XDatabaseMetaData >
                                        m_xMetaData;
        SharedResources                 m_aResources;
    public:

        static ::dbtools::OPropertyMap& getPropMap();

        OMetaConnection();

        rtl_TextEncoding getTextEncoding() const { return m_nTextEncoding; }
        const OUString&         getURL() const  { return m_sURL; }
        void             setURL(const OUString& _rsUrl) { m_sURL = _rsUrl; }
        void                    throwGenericSQLException(const char* pErrorResourceId, const css::uno::Reference< css::uno::XInterface>& _xContext);
        const SharedResources& getResources() const { return m_aResources;}

        void setConnectionInfo(const css::uno::Sequence< css::beans::PropertyValue >& _aInfo) { m_aConnectionInfo = _aInfo; }
        const css::uno::Sequence< css::beans::PropertyValue >&
            getConnectionInfo() const { return m_aConnectionInfo; }

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        //XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
        static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TCONNECTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
