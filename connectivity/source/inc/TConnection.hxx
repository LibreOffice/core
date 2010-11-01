/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CONNECTIVITY_CONNECTION_HXX
#define CONNECTIVITY_CONNECTION_HXX

#include <rtl/textenc.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/compbase4.hxx>
#include "propertyids.hxx"
#include "connectivity/CommonTools.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include "resource/sharedresources.hxx"

namespace connectivity
{
    typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::sdbc::XConnection,
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
        ::rtl::OUString                 m_sURL;
        rtl_TextEncoding                m_nTextEncoding; // the encoding which is used for all text conversions
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData >
                                        m_xMetaData;
        SharedResources                 m_aResources;
    public:

        static ::dbtools::OPropertyMap& getPropMap();

        OMetaConnection();

        inline rtl_TextEncoding getTextEncoding() const { return m_nTextEncoding; }
        inline ::rtl::OUString  getURL() const  { return m_sURL; }
        inline void             setURL(const ::rtl::OUString& _rsUrl) { m_sURL = _rsUrl; }
        void                    throwGenericSQLException( sal_uInt16 _nErrorResourceId,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xContext  );
        const SharedResources& getResources() const { return m_aResources;}

        inline void setConnectionInfo(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aInfo) { m_aConnectionInfo = _aInfo; }
        inline const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&
            getConnectionInfo() const { return m_aConnectionInfo; }

        // OComponentHelper
        virtual void SAL_CALL disposing(void);

        //XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };
}
#endif // CONNECTIVITY_CONNECTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
