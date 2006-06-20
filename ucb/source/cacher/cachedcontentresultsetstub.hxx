/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedcontentresultsetstub.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:15:11 $
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

#ifndef _CACHED_CONTENT_RESULTSET_STUB_HXX
#define _CACHED_CONTENT_RESULTSET_STUB_HXX

#ifndef _CONTENT_RESULTSET_WRAPPER_HXX
#include <contentresultsetwrapper.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XFETCHPROVIDER_HPP_
#include <com/sun/star/ucb/XFetchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XFETCHPROVIDERFORCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XFetchProviderForContentAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCACHEDCONTENTRESULTSETSTUBFACTORY_HPP_
#include <com/sun/star/ucb/XCachedContentResultSetStubFactory.hpp>
#endif

#define CACHED_CRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedContentResultSetStub"
#define CACHED_CRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedContentResultSetStubFactory"

//=========================================================================

class CachedContentResultSetStub
                : public ContentResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XFetchProvider
                , public com::sun::star::ucb::XFetchProviderForContentAccess
{
private:
    sal_Int32       m_nColumnCount;
    sal_Bool        m_bColumnCountCached;

    //members to propagate fetchsize and direction:
    sal_Bool        m_bNeedToPropagateFetchSize;
    sal_Bool        m_bFirstFetchSizePropagationDone;
    sal_Int32       m_nLastFetchSize;
    sal_Bool        m_bLastFetchDirection;
    const rtl::OUString     m_aPropertyNameForFetchSize;
    const rtl::OUString     m_aPropertyNameForFetchDirection;


    void SAL_CALL
    impl_getCurrentRowContent(
        com::sun::star::uno::Any& rRowContent,
        com::sun::star::uno::Reference<
            com::sun::star::sdbc::XRow > xRow )
        throw ( com::sun::star::sdbc::SQLException
              , com::sun::star::uno::RuntimeException );

    sal_Int32 SAL_CALL
    impl_getColumnCount();

    void SAL_CALL
    impl_getCurrentContentIdentifierString(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_getCurrentContentIdentifier(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_getCurrentContent(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, sal_Bool bFetchDirection )
        throw ( com::sun::star::uno::RuntimeException );

public:
    CachedContentResultSetStub( com::sun::star::uno::Reference<
                        com::sun::star::sdbc::XResultSet > xOrigin );

    virtual ~CachedContentResultSetStub();


    //-----------------------------------------------------------------
    // XInterface inherited
    //-----------------------------------------------------------------
    XINTERFACE_DECL()
    //-----------------------------------------------------------------
    // own inherited
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException );
    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()
    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()

    //-----------------------------------------------------------------
    // XFetchProvider
    //-----------------------------------------------------------------

    virtual com::sun::star::ucb::FetchResult SAL_CALL
    fetch( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XFetchProviderForContentAccess
    //-----------------------------------------------------------------
    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );
};

//=========================================================================

class CachedContentResultSetStubFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedContentResultSetStubFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedContentResultSetStubFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedContentResultSetStubFactory();

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
    // XCachedContentResultSetStubFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::sdbc::XResultSet > & xSource )
            throw( com::sun::star::uno::RuntimeException );
};

#endif

