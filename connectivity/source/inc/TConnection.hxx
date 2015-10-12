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
    typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::sdbc::XConnection,
                                               ::com::sun::star::sdbc::XWarningsSupplier,
                                               ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::lang::XUnoTunnel
                                           > OMetaConnection_BASE;

    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >  TConditions;

    class OOO_DLLPUBLIC_DBTOOLS OMetaConnection : public OMetaConnection_BASE
    {
    protected:
        ::osl::Mutex                    m_aMutex;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                        m_aConnectionInfo;
        connectivity::OWeakRefArray     m_aStatements;  //  vector containing a list
                                                        //  of all the Statement objects
                                                        //  for this Connection
        OUString                 m_sURL;
        rtl_TextEncoding                m_nTextEncoding; // the encoding which is used for all text conversions
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData >
                                        m_xMetaData;
        SharedResources                 m_aResources;
    public:

        static ::dbtools::OPropertyMap& getPropMap();

        OMetaConnection();

        inline rtl_TextEncoding getTextEncoding() const { return m_nTextEncoding; }
        inline OUString  getURL() const  { return m_sURL; }
        inline void             setURL(const OUString& _rsUrl) { m_sURL = _rsUrl; }
        void                    throwGenericSQLException( sal_uInt16 _nErrorResourceId,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xContext  );
        const SharedResources& getResources() const { return m_aResources;}

        inline void setConnectionInfo(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aInfo) { m_aConnectionInfo = _aInfo; }
        inline const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&
            getConnectionInfo() const { return m_aConnectionInfo; }

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        //XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TCONNECTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
