/*************************************************************************
 *
 *  $RCSfile: sortresult.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-17 11:50:06 $
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

#ifndef _SORTRESULT_HXX
#define _SORTRESULT_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_LISTACTION_HPP_
#include <com/sun/star/ucb/ListAction.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


#include <tools/list.hxx>

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace cppu {
    class OInterfaceContainerHelper;
}

//-----------------------------------------------------------------------------
struct  SortInfo;
struct  SortListData;
class   SRSPropertySetInfo;
class   PropertyChangeListeners_Impl;

//-----------------------------------------------------------------------------
class SortedEntryList : protected List
{
public:
                        SortedEntryList();
                        ~SortedEntryList();

                        List::Count;

    void                Clear();
    void                Insert( SortListData *pEntry, long nPos );
    SortListData*       Remove( long nPos );
    SortListData*       GetData( long nPos );

    long                operator [] ( long nPos ) const;
};

//-----------------------------------------------------------------------------
#define LISTACTION  com::sun::star::ucb::ListAction

class EventList : protected List
{
public:
                    EventList();
                    ~EventList();

                    List::Count;

    void            AddEvent( long nType, long nPos, long nCount );
    void            Insert( LISTACTION *pAction ) { List::Insert( pAction, LIST_APPEND ); }
    void            Clear();
    LISTACTION*     GetAction( long nIndex ) { return (LISTACTION*) GetObject( nIndex ); }
};

//-----------------------------------------------------------------------------

#define PROPERTYCHANGEEVENT     com::sun::star::beans::PropertyChangeEvent
#define RUNTIME_EXCEPTION       com::sun::star::uno::RuntimeException
#define REFERENCE               com::sun::star::uno::Reference
#define SEQUENCE                com::sun::star::uno::Sequence
#define XEVENTLISTENER          com::sun::star::lang::XEventListener
#define XRESULTSET              com::sun::star::sdbc::XResultSet
#define SQLEXCEPTION            com::sun::star::sdbc::SQLException
#define XRESULTSETMETADATA      com::sun::star::sdbc::XResultSetMetaData
#define NUMBERED_SORTINGINFO    com::sun::star::ucb::NumberedSortingInfo
#define XANYCOMPAREFACTORY      com::sun::star::ucb::XAnyCompareFactory

#define RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedResultSet"

//-----------------------------------------------------------------------------

class SortedResultSet:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XContentAccess,
                public XRESULTSET,
                public com::sun::star::sdbc::XRow,
                public com::sun::star::sdbc::XCloseable,
                public com::sun::star::sdbc::XResultSetMetaDataSupplier,
                public com::sun::star::beans::XPropertySet
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;
    PropertyChangeListeners_Impl    *mpPropChangeListeners;
    PropertyChangeListeners_Impl    *mpVetoChangeListeners;

    REFERENCE < XRESULTSET >            mxOriginal;
    REFERENCE < XRESULTSET >            mxOther;

    SRSPropertySetInfo* mpPropSetInfo;
    SortInfo*           mpSortInfo;
    osl::Mutex          maMutex;
    SortedEntryList     maS2O;          // maps the sorted entries to the original ones
    List                maO2S;          // maps the original Entries to the sorted ones
    List                maModList;      // keeps track of modified entries
    long                mnLastSort;     // index of the last sorted entry;
    long                mnCurEntry;     // index of the current entry
    long                mnCount;        // total count of the elements
    BOOL                mbIsCopy;


private:

    long                FindPos( SortListData *pEntry, long nStart, long nEnd );
    long                Compare( SortListData *pOne,
                                 SortListData *pTwo );
    void                BuildSortInfo( REFERENCE< XRESULTSET > aResult,
                                       const SEQUENCE < NUMBERED_SORTINGINFO > &xSortInfo,
                                       const REFERENCE< XANYCOMPAREFACTORY > &xCompFac );
    long                CompareImpl( REFERENCE < XRESULTSET > xResultOne,
                                     REFERENCE < XRESULTSET > xResultTwo,
                                     long nIndexOne, long nIndexTwo,
                                     SortInfo* pSortInfo );
    long                CompareImpl( REFERENCE < XRESULTSET > xResultOne,
                                     REFERENCE < XRESULTSET > xResultTwo,
                                     long nIndexOne, long nIndexTwo );
    void                PropertyChanged( const PROPERTYCHANGEEVENT& rEvt );

public:
                        SortedResultSet( REFERENCE< XRESULTSET > aResult );
                        ~SortedResultSet();

    const SortedEntryList*      GetS2OList() const { return &maS2O; }
    const List*                 GetO2SList() const { return &maO2S; }
    REFERENCE < XRESULTSET >    GetResultSet() const { return mxOriginal; }
    SortInfo*                   GetSortInfo() const { return mpSortInfo; }
    long                        GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const SEQUENCE < NUMBERED_SORTINGINFO > &xSortInfo,
                                    const REFERENCE< XANYCOMPAREFACTORY > &xCompFac );
    void                CheckProperties( long nOldCount, BOOL bWasFinal );

    void                InsertNew( long nPos, long nCount );
    void                SetChanged( long nPos, long nCount );
    void                Remove( long nPos, long nCount, EventList *pList );
    void                Move( long nPos, long nCount, long nOffset );

    void                ResortModified( EventList* pList );
    void                ResortNew( EventList* pList );

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose() throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    // XContentAccess
    virtual rtl::OUString SAL_CALL
    queryContentIdentfierString()
        throw( RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent()
        throw( RUNTIME_EXCEPTION );

    // XResultSet
    virtual sal_Bool SAL_CALL
    next()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    beforeFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    afterLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    first()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    last()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    previous()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    refreshRow()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::uno::XInterface > SAL_CALL
    getStatement()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull() throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
    throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual REFERENCE<
                com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual REFERENCE<
                com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const REFERENCE<
                   com::sun::star::container::XNameAccess >& typeMap )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XCloseable
    virtual void SAL_CALL
    close()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XResultSetMetaDataSupplier
    virtual REFERENCE< XRESULTSETMETADATA > SAL_CALL
    getMetaData()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );


    // XPropertySet
    virtual REFERENCE<
                com::sun::star::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    setPropertyValue( const rtl::OUString& PropertyName,
                      const com::sun::star::uno::Any& Value )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Any SAL_CALL
    getPropertyValue( const rtl::OUString& PropertyName )
        throw( com::sun::star::beans::UnknownPropertyException,
        com::sun::star::lang::WrappedTargetException,
        RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addPropertyChangeListener( const rtl::OUString& PropertyName,
                               const REFERENCE<
                                       com::sun::star::beans::XPropertyChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removePropertyChangeListener( const rtl::OUString& PropertyName,
                                  const REFERENCE<
                                      com::sun::star::beans::XPropertyChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addVetoableChangeListener( const rtl::OUString& PropertyName,
                               const REFERENCE<
                                       com::sun::star::beans::XVetoableChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                  const REFERENCE<
                                      com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );
};

#endif

