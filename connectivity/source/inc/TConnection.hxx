/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TConnection.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:59:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_CONNECTION_HXX
#define CONNECTIVITY_CONNECTION_HXX

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif


namespace connectivity
{
    typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::sdbc::XConnection,
                                                ::com::sun::star::sdbc::XWarningsSupplier,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::lang::XUnoTunnel
                                            > OMetaConnection_BASE;

    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >  TConditions;

    class OMetaConnection : public OMetaConnection_BASE
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
    public:

        static ::dbtools::OPropertyMap& getPropMap();

        OMetaConnection() : OMetaConnection_BASE(m_aMutex) ,m_nTextEncoding(RTL_TEXTENCODING_MS_1252){}

        inline rtl_TextEncoding getTextEncoding() const { return m_nTextEncoding; }
        inline ::rtl::OUString  getURL() const  { return m_sURL; }
        inline void             setURL(const ::rtl::OUString& _rsUrl) { m_sURL = _rsUrl; }

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


