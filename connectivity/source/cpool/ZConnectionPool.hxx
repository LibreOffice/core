/*************************************************************************
 *
 *  $RCSfile: ZConnectionPool.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:37:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_ZCONNECTIONPOOL_HXX_
#define _CONNECTIVITY_ZCONNECTIONPOOL_HXX_

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPOOLEDCONNECTION_HPP_
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_TIMER_HXX_
#include <vos/timer.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif

namespace connectivity
{
    class OConnectionPool;
    //==========================================================================
    /// OPoolTimer - Invalidates the connection pool
    //==========================================================================
    class OPoolTimer : public ::vos::OTimer
    {
        OConnectionPool* m_pPool;
    public:
        OPoolTimer(OConnectionPool* _pPool,const ::vos::TTimeValue& _Time)
            : ::vos::OTimer(_Time)
            ,m_pPool(_pPool)
        {}
    protected:
        virtual void SAL_CALL onShot();
    };

    //==========================================================================
    //= OConnectionPool - the one-instance service for PooledConnections
    //= manages the active connections and the connections in the pool
    //==========================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>  OConnectionPool_Base;

    // typedef for the interanl structure
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPooledConnection> > TPooledConnections;

     // contains the currently pooled connections
    typedef struct
    {
        TPooledConnections  aConnections;
        sal_Int32           nALiveCount; // will be decremented everytime a time says to, when will reach zero the pool will be deleted
    } TConnectionPool;

    struct TDigestHolder
    {
        sal_uInt8 m_pBuffer[RTL_DIGEST_LENGTH_SHA1];
        TDigestHolder()
        {
            m_pBuffer[0] = 0;
        }

    };

    //  typedef TDigestHolder

    struct TDigestLess : public ::std::binary_function< TDigestHolder, TDigestHolder, bool>
    {
        bool operator() (const TDigestHolder& x, const TDigestHolder& y) const
        {
            sal_uInt32 i;
            for(i=0;i < RTL_DIGEST_LENGTH_SHA1 && (x.m_pBuffer[i] >= y.m_pBuffer[i]); ++i)
                ;
            return i < RTL_DIGEST_LENGTH_SHA1;
        }
    };

    typedef ::std::map< TDigestHolder,TConnectionPool,TDigestLess> TConnectionMap;

    // contains additional information about a activeconnection which is needed to put it back to the pool
    typedef struct
    {
        TConnectionMap::iterator aPos;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPooledConnection> xPooledConnection;
    } TActiveConnectionInfo;

    typedef ::std::map< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>,
                        TActiveConnectionInfo> TActiveConnectionMap;

    class OConnectionPool : public OConnectionPool_Base
    {
        TConnectionMap          m_aPool;                // the pooled connections
        TActiveConnectionMap    m_aActiveConnections;   // the currently active connections

        ::osl::Mutex            m_aMutex;
        ::vos::ORef<OPoolTimer> m_xInvalidator;         // invalidates the conntection pool when shot

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >             m_xDriver;      // the one and only driver for this connectionpool
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >           m_xDriverNode;  // config node entry
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory > m_xProxyFactory;
        sal_Int32               m_nTimeOut;
        sal_Int32               m_nALiveCount;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> createNewConnection(const ::rtl::OUString& _rURL,
                                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getPooledConnection(TConnectionMap::iterator& _rIter);
        // calculate the timeout and the corresponding ALiveCount
        void calculateTimeOuts(sal_Int32 _nTimeOut);

    protected:
        // the dtor will be called from the last instance  (last release call)
        virtual ~OConnectionPool();
    public:
        OConnectionPool(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _xDriver,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xDriverNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory);

        // delete all refs
        void clear(sal_Bool _bDispose);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        void invalidatePooledConnections();
    };
}
#endif // _CONNECTIVITY_ZCONNECTIONPOOL_HXX_


