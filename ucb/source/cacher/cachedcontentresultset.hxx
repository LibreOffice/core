/*************************************************************************
 *
 *  $RCSfile: cachedcontentresultset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-31 10:37:35 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _CACHED_CONTENT_RESULTSET_HXX
#define _CACHED_CONTENT_RESULTSET_HXX

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

#ifndef _COM_SUN_STAR_UCB_FETCHRESULT_HPP_
#include <com/sun/star/ucb/FetchResult.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERMAPPING_HPP_
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCACHEDCONTENTRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XCachedContentResultSetFactory.hpp>
#endif

#define CACHED_CONTENT_RESULTSET_SERVICE_NAME "com.sun.star.ucb.CachedContentResultSet"
#define CACHED_CONTENT_RESULTSET_FACTORY_NAME "com.sun.star.ucb.CachedContentResultSetFactory"

//=========================================================================

class CCRS_PropertySetInfo;
class CachedContentResultSet
                : public ContentResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
    //--------------------------------------------------------------------------
    // class CCRS_Cache

    class CCRS_Cache
    {
    private:
        com::sun::star::ucb::FetchResult*           m_pResult;
        com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentIdentifierMapping >
                                                    m_xContentIdentifierMapping;
        com::sun::star::uno::Sequence< sal_Bool >*  m_pMappedReminder;

    private:
        com::sun::star::uno::Any& SAL_CALL
        getRowAny( sal_Int32 nRow )
            throw( com::sun::star::sdbc::SQLException,
            com::sun::star::uno::RuntimeException );

        void SAL_CALL clear();


        void SAL_CALL remindMapped( sal_Int32 nRow );
        sal_Bool SAL_CALL isRowMapped( sal_Int32 nRow );
        void SAL_CALL clearMappedReminder();
        com::sun::star::uno::Sequence< sal_Bool >* SAL_CALL getMappedReminder();

    public:
        CCRS_Cache( const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifierMapping > & xMapping );
        ~CCRS_Cache();

        void SAL_CALL loadData(
            const com::sun::star::ucb::FetchResult& rResult );

        sal_Bool SAL_CALL
        hasRow( sal_Int32 nRow );

        sal_Bool SAL_CALL
        hasCausedException( sal_Int32 nRow );

        sal_Int32 SAL_CALL
        getMaxRow();

        sal_Bool SAL_CALL
        hasKnownLast();

        //---
        const com::sun::star::uno::Any& SAL_CALL
        getAny( sal_Int32 nRow, sal_Int32 nColumnIndex )
            throw( com::sun::star::sdbc::SQLException,
            com::sun::star::uno::RuntimeException );

        const rtl::OUString& SAL_CALL
        getContentIdentifierString( sal_Int32 nRow )
            throw( com::sun::star::uno::RuntimeException );

        const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& SAL_CALL
        getContentIdentifier( sal_Int32 nRow )
            throw( com::sun::star::uno::RuntimeException );

        const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent >& SAL_CALL
        getContent( sal_Int32 nRow )
            throw( com::sun::star::uno::RuntimeException );
    };
    //-----------------------------------------------------------------
    //members

    //different Interfaces from Origin:
    com::sun::star::uno::Reference< com::sun::star::ucb::XFetchProvider >
                            m_xFetchProvider; //XFetchProvider-interface from m_xOrigin

    com::sun::star::uno::Reference< com::sun::star::ucb::XFetchProviderForContentAccess >
                            m_xFetchProviderForContentAccess; //XFetchProviderForContentAccess-interface from m_xOrigin

    //my PropertySetInfo
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >
                            m_xMyPropertySetInfo;//holds m_pMyPropSetInfo alive
    CCRS_PropertySetInfo*   m_pMyPropSetInfo;


    //
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;// can be used for remote optimized ContentAccess

    //some Properties and helping variables
    sal_Int32               m_nRow;
    sal_Bool                m_bAfterLast; // TRUE, if m_nRow is after final count; can be TRUE without knowing the exact final count

    sal_Int32               m_nLastAppliedPos;
    sal_Bool                m_bAfterLastApplied;

    sal_Int32               m_nKnownCount; // count we know from the Origin
    sal_Bool                m_bFinalCount; // TRUE if the Origin has reached final count and we got that count in m_nKnownCount

    sal_Int32               m_nFetchSize;
    sal_Int32               m_nFetchDirection;

    sal_Bool                m_bLastReadWasFromCache;
    sal_Bool                m_bLastCachedReadWasNull;

    //cache:
    CCRS_Cache              m_aCache;
    CCRS_Cache              m_aCacheContentIdentifierString;
    CCRS_Cache              m_aCacheContentIdentifier;
    CCRS_Cache              m_aCacheContent;


private:
    //-----------------------------------------------------------------
    //helping XPropertySet methods.
    virtual void SAL_CALL impl_initPropertySetInfo();


    //-----------------------------------------------------------------
    sal_Bool SAL_CALL
    applyPositionToOrigin( sal_Int32 nRow )
        throw( com::sun::star::sdbc::SQLException,
        com::sun::star::uno::RuntimeException );

    sal_Bool SAL_CALL
    applyPositionToOrigin()
        throw( com::sun::star::sdbc::SQLException,
        com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_fetchData( sal_Int32 nRow, sal_Int32 nCount
                    , sal_Int32 nFetchDirection )
        throw( com::sun::star::uno::RuntimeException );

    sal_Bool SAL_CALL
    impl_isKnownValidPosition( sal_Int32 nRow );

    sal_Bool SAL_CALL
    impl_isKnownInvalidPosition( sal_Int32 nRow );

    void SAL_CALL
    impl_changeRowCount( sal_Int32 nOld, sal_Int32 nNew );

    void SAL_CALL
    impl_changeIsRowCountFinal( sal_Bool bOld, sal_Bool bNew );

public:
    CachedContentResultSet( com::sun::star::uno::Reference<
                        com::sun::star::sdbc::XResultSet > xOrigin,
                        com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifierMapping >
                        xContentIdentifierMapping );

    virtual ~CachedContentResultSet();

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
    // XPropertySet inherited
    //-----------------------------------------------------------------

    virtual void SAL_CALL
    setPropertyValue( const rtl::OUString& aPropertyName,
                      const com::sun::star::uno::Any& aValue )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
    getPropertyValue( const rtl::OUString& PropertyName )
        throw( com::sun::star::beans::UnknownPropertyException,
        com::sun::star::lang::WrappedTargetException,
        com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // own inherited
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XContentAccess inherited
    //-----------------------------------------------------------------
    virtual rtl::OUString SAL_CALL
    queryContentIdentifierString()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent()
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XResultSet inherited
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
    next()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    beforeFirst()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    afterLast()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    first()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    last()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    previous()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    refreshRow()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > SAL_CALL
    getStatement()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XRow inherited
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
    wasNull()
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const com::sun::star::uno::Reference<
                   com::sun::star::container::XNameAccess >& typeMap )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( com::sun::star::sdbc::SQLException,
               com::sun::star::uno::RuntimeException );
};

//=========================================================================

class CachedContentResultSetFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedContentResultSetFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedContentResultSetFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedContentResultSetFactory();

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
    // XCachedContentResultSetFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSet(
            const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > & xSource,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifierMapping > & xMapping )
        throw( com::sun::star::uno::RuntimeException );
};

#endif

