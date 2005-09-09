/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheddynamicresultset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:11:52 $
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

#ifndef _CACHED_DYNAMIC_RESULTSET_HXX
#define _CACHED_DYNAMIC_RESULTSET_HXX

#ifndef _DYNAMIC_RESULTSET_WRAPPER_HXX
#include <dynamicresultsetwrapper.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERMAPPING_HPP_
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCACHEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XCachedDynamicResultSetFactory.hpp>
#endif

#define CACHED_DRS_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSet"
#define CACHED_DRS_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetFactory"

//=========================================================================

class CachedDynamicResultSet
                : public DynamicResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;

protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:
    CachedDynamicResultSet( com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifierMapping > & xContentMapping
        , const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory > & xSMgr );

    virtual ~CachedDynamicResultSet();


    //-----------------------------------------------------------------
    // XInterface inherited
    //-----------------------------------------------------------------
    XINTERFACE_DECL()
    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()
    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()

    //-----------------------------------------------------------------
    // own methods ( inherited )
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );
};

//=========================================================================

class CachedDynamicResultSetFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedDynamicResultSetFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedDynamicResultSetFactory();

    //-----------------------------------------------------------------
    // XInterface
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    XSERVICEINFO_DECL()

    //-----------------------------------------------------------------
    // XCachedDynamicResultSetFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSet(
                  const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > &
                        SourceStub
                , const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifierMapping > &
                        ContentIdentifierMapping
                )
            throw( com::sun::star::uno::RuntimeException );
};

#endif

