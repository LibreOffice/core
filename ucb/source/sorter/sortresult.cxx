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


#include <vector>
#include "sortresult.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <memory>

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace comphelper;
using namespace cppu;


//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex ourMutex;

    return ourMutex;
}


struct SortInfo
{
    bool    mbUseOwnCompare;
    bool    mbAscending;
    bool    mbCaseSensitive;
    sal_Int32   mnColumn;
    sal_Int32   mnType;
    SortInfo*   mpNext;
    Reference < XAnyCompare >   mxCompareFunction;
};


struct SortListData
{
    bool        mbModified;
    sal_IntPtr  mnCurPos;
    sal_IntPtr const  mnOldPos;

    explicit SortListData( sal_IntPtr nPos );
};


// class SRSPropertySetInfo.


class SRSPropertySetInfo : public cppu::WeakImplHelper <
    XPropertySetInfo >
{
    Property    maProps[2];

private:

public:
                SRSPropertySetInfo();

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;
};

typedef OMultiTypeInterfaceContainerHelperVar<OUString>
    PropertyChangeListenerContainer_Impl;

class PropertyChangeListeners_Impl : public PropertyChangeListenerContainer_Impl
{
public:
    PropertyChangeListeners_Impl()
    : PropertyChangeListenerContainer_Impl( getContainerMutex() ) {}
};


SortedResultSet::SortedResultSet( Reference< XResultSet > const & aResult )
{
    mpDisposeEventListeners = nullptr;
    mpPropChangeListeners   = nullptr;
    mpVetoChangeListeners   = nullptr;

    mxOriginal  = aResult;
    mpSortInfo  = nullptr;
    mnLastSort  = 0;
    mnCurEntry  = 0;
    mnCount     = 0;
    mbIsCopy    = false;
}


SortedResultSet::~SortedResultSet()
{
    mxOriginal.clear();
    mxOther.clear();

    if ( !mbIsCopy )
    {
        SortInfo *pInfo = mpSortInfo;
        while ( pInfo )
        {
            mpSortInfo = pInfo->mpNext;
            delete pInfo;
            pInfo = mpSortInfo;
        }
    }

    mpSortInfo = nullptr;

    mpPropSetInfo.clear();
}


// XServiceInfo methods.

OUString SAL_CALL SortedResultSet::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.SortedResultSet" );
}

sal_Bool SAL_CALL SortedResultSet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedResultSet::getSupportedServiceNames()
{
    return { RESULTSET_SERVICE_NAME };
}


// XComponent methods.

void SAL_CALL SortedResultSet::dispose()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners && mpDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        mpDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( mpPropChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );
        mpPropChangeListeners->disposeAndClear( aEvt );
    }

    if ( mpVetoChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );
        mpVetoChangeListeners->disposeAndClear( aEvt );
    }

    mxOriginal.clear();
    mxOther.clear();
}


void SAL_CALL SortedResultSet::addEventListener(
                            const Reference< XEventListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpDisposeEventListeners )
        mpDisposeEventListeners =
                    new OInterfaceContainerHelper2( getContainerMutex() );

    mpDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL SortedResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners )
        mpDisposeEventListeners->removeInterface( Listener );
}


// XContentAccess methods.


OUString SAL_CALL
SortedResultSet::queryContentIdentifierString()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentifierString();
}


Reference< XContentIdentifier > SAL_CALL
SortedResultSet::queryContentIdentifier()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentifier();
}


Reference< XContent > SAL_CALL
SortedResultSet::queryContent()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContent();
}


// XResultSet methods.

sal_Bool SAL_CALL SortedResultSet::next()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    mnCurEntry++;

    if ( mnCurEntry > 0 )
    {
        if ( mnCurEntry <= mnCount )
        {
            sal_Int32 nIndex = maS2O[ mnCurEntry ];
            return mxOriginal->absolute( nIndex );
        }
        else
        {
            mnCurEntry = mnCount + 1;
        }
    }
    return false;
}


sal_Bool SAL_CALL SortedResultSet::isBeforeFirst()
{
    if ( mnCurEntry )
        return false;
    else
        return true;
}


sal_Bool SAL_CALL SortedResultSet::isAfterLast()
{
    if ( mnCurEntry > mnCount )
        return true;
    else
        return false;
}


sal_Bool SAL_CALL SortedResultSet::isFirst()
{
    if ( mnCurEntry == 1 )
        return true;
    else
        return false;
}


sal_Bool SAL_CALL SortedResultSet::isLast()
{
    if ( mnCurEntry == mnCount )
        return true;
    else
        return false;
}


void SAL_CALL SortedResultSet::beforeFirst()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = 0;
    mxOriginal->beforeFirst();
}


void SAL_CALL SortedResultSet::afterLast()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = mnCount+1;
    mxOriginal->afterLast();
}


sal_Bool SAL_CALL SortedResultSet::first()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mnCount )
    {
        mnCurEntry = 1;
        sal_Int32 nIndex = maS2O[ mnCurEntry ];
        return mxOriginal->absolute( nIndex );
    }
    else
    {
        mnCurEntry = 0;
        return false;
    }
}


sal_Bool SAL_CALL SortedResultSet::last()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mnCount )
    {
        mnCurEntry = mnCount;
        sal_Int32 nIndex = maS2O[ mnCurEntry ];
        return mxOriginal->absolute( nIndex );
    }
    else
    {
        mnCurEntry = 0;
        return false;
    }
}


sal_Int32 SAL_CALL SortedResultSet::getRow()
{
    return mnCurEntry;
}


/**
 moves the cursor to the given row number in the result set.
 <p>If the row number is positive, the cursor moves to the given row
 number with respect to the beginning of the result set. The first
 row is row 1, the second is row 2, and so on.
 <p>If the given row number is negative, the cursor moves to an
 absolute row position with respect to the end of the result set.
 For example, calling <code>moveToPosition(-1)</code> positions the
 cursor on the last row, <code>moveToPosition(-2)</code> indicates the
 next-to-last row, and so on.
 <p>An attempt to position the cursor beyond the first/last row in the
 result set leaves the cursor before/after the first/last row,
 respectively.
 <p>Note: Calling <code>moveToPosition(1)</code> is the same
 as calling <code>moveToFirst()</code>. Calling
 <code>moveToPosition(-1)</code> is the same as calling
 <code>moveToLast()</code>.
 @param row
    is the number of rows to move. Could be negative.
 @returns
    <TRUE/> if the cursor is on a row; <FALSE/> otherwise
 @throws SQLException
    if a database access error occurs or if row is 0, or the result set
    type is FORWARD_ONLY.
 */
sal_Bool SAL_CALL SortedResultSet::absolute( sal_Int32 row )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    sal_Int32 nIndex;

    if ( row > 0 )
    {
        if ( row <= mnCount )
        {
            mnCurEntry = row;
            nIndex = maS2O[ mnCurEntry ];
            return mxOriginal->absolute( nIndex );
        }
        else
        {
            mnCurEntry = mnCount + 1;
            return false;
        }
    }
    else if ( row == 0 )
    {
        throw SQLException();
    }
    else
    {
        if ( mnCount + row + 1 > 0 )
        {
            mnCurEntry = mnCount + row + 1;
            nIndex = maS2O[ mnCurEntry ];
            return mxOriginal->absolute( nIndex );
        }
        else
        {
            mnCurEntry = 0;
            return false;
        }
    }
}


/**
 moves the cursor a relative number of rows, either positive or negative.
 <p>
 Attempting to move beyond the first/last row in the result set positions
 the cursor before/after the first/last row. Calling
 <code>moveRelative(0)</code> is valid, but does not change the cursor
 position.
 <p>Note: Calling <code>moveRelative(1)</code> is different from calling
 <code>moveNext()</code> because is makes sense to call
 <code>moveNext()</code> when there is no current row, for example,
 when the cursor is positioned before the first row or after the last
 row of the result set.
 @param rows
    is the number of rows to move. Could be negative.
 @returns
    <TRUE/> if the cursor is on a valid row; <FALSE/> if it is off
    the result set.
 @throws SQLException
    if a database access error occurs or if there is no
    current row, or the result set type is FORWARD_ONLY.
 */
sal_Bool SAL_CALL SortedResultSet::relative( sal_Int32 rows )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    if ( rows == 0 )
        return true;

    sal_Int32 nTmp = mnCurEntry + rows;

    if ( nTmp <= 0 )
    {
        mnCurEntry = 0;
        return false;
    }
    else if ( nTmp > mnCount )
    {
        mnCurEntry = mnCount + 1;
        return false;
    }
    else
    {
        mnCurEntry = nTmp;
        nTmp = maS2O[ mnCurEntry ];
        return mxOriginal->absolute( nTmp );
    }
}


/**
 moves the cursor to the previous row in the result set.
 <p>Note: <code>previous()</code> is not the same as
 <code>relative(-1)</code> because it makes sense to call
 <code>previous()</code> when there is no current row.
 @returns <TRUE/> if the cursor is on a valid row; <FALSE/> if it is off
    the result set.
 @throws SQLException
    if a database access error occurs or the result set type
    is FORWARD_ONLY.
 */
sal_Bool SAL_CALL SortedResultSet::previous()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    mnCurEntry -= 1;

    if ( mnCurEntry > 0 )
    {
        if ( mnCurEntry <= mnCount )
        {
            sal_Int32 nIndex = maS2O[ mnCurEntry ];
            return mxOriginal->absolute( nIndex );
        }
    }
    else
        mnCurEntry = 0;

    return false;
}


void SAL_CALL SortedResultSet::refreshRow()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    mxOriginal->refreshRow();
}


sal_Bool SAL_CALL SortedResultSet::rowUpdated()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowUpdated();
}


sal_Bool SAL_CALL SortedResultSet::rowInserted()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowInserted();
}


sal_Bool SAL_CALL SortedResultSet::rowDeleted()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowDeleted();
}


Reference< XInterface > SAL_CALL SortedResultSet::getStatement()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->getStatement();
}


// XRow methods.


sal_Bool SAL_CALL SortedResultSet::wasNull()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->wasNull();
}


OUString SAL_CALL SortedResultSet::getString( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getString( columnIndex );
}


sal_Bool SAL_CALL SortedResultSet::getBoolean( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBoolean( columnIndex );
}


sal_Int8 SAL_CALL SortedResultSet::getByte( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getByte( columnIndex );
}


sal_Int16 SAL_CALL SortedResultSet::getShort( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getShort( columnIndex );
}


sal_Int32 SAL_CALL SortedResultSet::getInt( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getInt( columnIndex );
}

sal_Int64 SAL_CALL SortedResultSet::getLong( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getLong( columnIndex );
}


float SAL_CALL SortedResultSet::getFloat( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getFloat( columnIndex );
}


double SAL_CALL SortedResultSet::getDouble( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDouble( columnIndex );
}


Sequence< sal_Int8 > SAL_CALL SortedResultSet::getBytes( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBytes( columnIndex );
}


Date SAL_CALL SortedResultSet::getDate( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDate( columnIndex );
}


Time SAL_CALL SortedResultSet::getTime( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTime( columnIndex );
}


DateTime SAL_CALL SortedResultSet::getTimestamp( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTimestamp( columnIndex );
}


Reference< XInputStream > SAL_CALL
SortedResultSet::getBinaryStream( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBinaryStream( columnIndex );
}


Reference< XInputStream > SAL_CALL
SortedResultSet::getCharacterStream( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getCharacterStream( columnIndex );
}


Any SAL_CALL SortedResultSet::getObject( sal_Int32 columnIndex,
                       const Reference< XNameAccess >& typeMap )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getObject( columnIndex,
                                                            typeMap);
}


Reference< XRef > SAL_CALL SortedResultSet::getRef( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getRef( columnIndex );
}


Reference< XBlob > SAL_CALL SortedResultSet::getBlob( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBlob( columnIndex );
}


Reference< XClob > SAL_CALL SortedResultSet::getClob( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getClob( columnIndex );
}


Reference< XArray > SAL_CALL SortedResultSet::getArray( sal_Int32 columnIndex )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getArray( columnIndex );
}


// XCloseable methods.


void SAL_CALL SortedResultSet::close()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    Reference< XCloseable >::query(mxOriginal)->close();
}


// XResultSetMetaDataSupplier methods.


Reference< XResultSetMetaData > SAL_CALL SortedResultSet::getMetaData()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XResultSetMetaDataSupplier >::query(mxOriginal)->getMetaData();
}


// XPropertySet methods.


Reference< XPropertySetInfo > SAL_CALL
SortedResultSet::getPropertySetInfo()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropSetInfo.is() )
    {
        mpPropSetInfo = new SRSPropertySetInfo();
    }

    return Reference< XPropertySetInfo >( mpPropSetInfo.get() );
}


void SAL_CALL SortedResultSet::setPropertyValue(
                        const OUString& PropertyName,
                        const Any& )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( PropertyName == "RowCount" || PropertyName == "IsRowCountFinal" )
        throw IllegalArgumentException();
    else
        throw UnknownPropertyException();
}


Any SAL_CALL SortedResultSet::getPropertyValue( const OUString& PropertyName )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    Any aRet;

    if ( PropertyName == "RowCount" )
    {
        aRet <<= maS2O.Count();
    }
    else if ( PropertyName == "IsRowCountFinal" )
    {
        bool    bOrgFinal = false;
        Any         aOrgRet;

        aRet <<= false;

        aOrgRet = Reference< XPropertySet >::query(mxOriginal)->
                        getPropertyValue( PropertyName );
        aOrgRet >>= bOrgFinal;

        if ( bOrgFinal )
        {
            aOrgRet = Reference< XPropertySet >::query(mxOriginal)->
                getPropertyValue("RowCount");
            sal_uInt32  nOrgCount = 0;
            aOrgRet >>= nOrgCount;
            if ( nOrgCount == maS2O.Count() )
                aRet <<= true;
        }
    }
    else
        throw UnknownPropertyException();

    return aRet;
}


void SAL_CALL SortedResultSet::addPropertyChangeListener(
                        const OUString& PropertyName,
                        const Reference< XPropertyChangeListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        mpPropChangeListeners.reset(
                    new PropertyChangeListeners_Impl() );

    mpPropChangeListeners->addInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::removePropertyChangeListener(
                        const OUString& PropertyName,
                        const Reference< XPropertyChangeListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpPropChangeListeners )
        mpPropChangeListeners->removeInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::addVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpVetoChangeListeners )
        mpVetoChangeListeners.reset(
                    new PropertyChangeListeners_Impl() );

    mpVetoChangeListeners->addInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::removeVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpVetoChangeListeners )
        mpVetoChangeListeners->removeInterface( PropertyName, Listener );
}


// private methods

sal_IntPtr SortedResultSet::CompareImpl( const Reference < XResultSet >& xResultOne,
                                   const Reference < XResultSet >& xResultTwo,
                                   sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo,
                                   SortInfo const * pSortInfo )
{
    Reference < XRow > xRowOne( xResultOne, UNO_QUERY );
    Reference < XRow > xRowTwo( xResultTwo, UNO_QUERY );

    sal_IntPtr nCompare = 0;
    sal_Int32 nColumn = pSortInfo->mnColumn;

    switch ( pSortInfo->mnType )
    {
        case DataType::BIT :
        case DataType::TINYINT :
        case DataType::SMALLINT :
        case DataType::INTEGER :
            {
                sal_Int32 aOne = 0;
                sal_Int32 aTwo = 0;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getInt( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getInt( nColumn );

                if ( aOne < aTwo )
                    nCompare = -1;
                else if ( aOne == aTwo )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::BIGINT :
            {
                sal_Int64 aOne = 0;
                sal_Int64 aTwo = 0;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getLong( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getLong( nColumn );

                if ( aOne < aTwo )
                    nCompare = -1;
                else if ( aOne == aTwo )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::CHAR :
        case DataType::VARCHAR :
        case DataType::LONGVARCHAR :
            {
                OUString aOne, aTwo;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getString( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getString( nColumn );

                if ( ! pSortInfo->mbCaseSensitive )
                {
                    aOne = aOne.toAsciiLowerCase();
                    aTwo = aTwo.toAsciiLowerCase();
                }

                nCompare = aOne.compareTo( aTwo );
                break;
            }
        case DataType::DATE :
            {
                Date aOne, aTwo;
                sal_Int32   nTmp;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getDate( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getDate( nColumn );

                nTmp = static_cast<sal_Int32>(aTwo.Year) - static_cast<sal_Int32>(aOne.Year);
                if ( !nTmp ) {
                    nTmp = static_cast<sal_Int32>(aTwo.Month) - static_cast<sal_Int32>(aOne.Month);
                    if ( !nTmp )
                        nTmp = static_cast<sal_Int32>(aTwo.Day) - static_cast<sal_Int32>(aOne.Day);
                }

                if ( nTmp < 0 )
                    nCompare = -1;
                else if ( nTmp == 0 )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::TIME :
            {
                Time aOne, aTwo;
                sal_Int32   nTmp;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getTime( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getTime( nColumn );

                nTmp = static_cast<sal_Int32>(aTwo.Hours) - static_cast<sal_Int32>(aOne.Hours);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Minutes) - static_cast<sal_Int32>(aOne.Minutes);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Seconds) - static_cast<sal_Int32>(aOne.Seconds);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.NanoSeconds)
                                    - static_cast<sal_Int32>(aOne.NanoSeconds);

                if ( nTmp < 0 )
                    nCompare = -1;
                else if ( nTmp == 0 )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::TIMESTAMP :
            {
                DateTime aOne, aTwo;
                sal_Int32   nTmp;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getTimestamp( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getTimestamp( nColumn );

                nTmp = static_cast<sal_Int32>(aTwo.Year) - static_cast<sal_Int32>(aOne.Year);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Month) - static_cast<sal_Int32>(aOne.Month);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Day) - static_cast<sal_Int32>(aOne.Day);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Hours) - static_cast<sal_Int32>(aOne.Hours);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Minutes) - static_cast<sal_Int32>(aOne.Minutes);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.Seconds) - static_cast<sal_Int32>(aOne.Seconds);
                if ( !nTmp )
                    nTmp = static_cast<sal_Int32>(aTwo.NanoSeconds)
                                    - static_cast<sal_Int32>(aOne.NanoSeconds);

                if ( nTmp < 0 )
                    nCompare = -1;
                else if ( nTmp == 0 )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::REAL :
            {
                float aOne = 0;
                float aTwo = 0;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getFloat( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getFloat( nColumn );

                if ( aOne < aTwo )
                    nCompare = -1;
                else if ( aOne == aTwo )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        case DataType::FLOAT :
        case DataType::DOUBLE :
            {
                double aOne = 0;
                double aTwo = 0;

                if ( xResultOne->absolute( nIndexOne ) )
                    aOne = xRowOne->getDouble( nColumn );
                if ( xResultTwo->absolute( nIndexTwo ) )
                    aTwo = xRowTwo->getDouble( nColumn );

                if ( aOne < aTwo )
                    nCompare = -1;
                else if ( aOne == aTwo )
                    nCompare = 0;
                else
                    nCompare = 1;

                break;
            }
        default:
            {
                OSL_FAIL( "DataType not supported for compare!" );
            }
    }

    return nCompare;
}


sal_IntPtr SortedResultSet::CompareImpl( const Reference < XResultSet >& xResultOne,
                                   const Reference < XResultSet >& xResultTwo,
                                   sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo )
{
    sal_IntPtr  nCompare = 0;
    SortInfo*   pInfo = mpSortInfo;

    while ( !nCompare && pInfo )
    {
        if ( pInfo->mbUseOwnCompare )
        {
            nCompare = CompareImpl( xResultOne, xResultTwo,
                                    nIndexOne, nIndexTwo, pInfo );
        }
        else
        {
            Any aOne, aTwo;

            Reference < XRow > xRowOne =
                            Reference< XRow >::query( xResultOne );
            Reference < XRow > xRowTwo =
                            Reference< XRow >::query( xResultTwo );

            if ( xResultOne->absolute( nIndexOne ) )
                aOne = xRowOne->getObject( pInfo->mnColumn, nullptr );
            if ( xResultTwo->absolute( nIndexTwo ) )
                aTwo = xRowTwo->getObject( pInfo->mnColumn, nullptr );

            nCompare = pInfo->mxCompareFunction->compare( aOne, aTwo );
        }

        if ( ! pInfo->mbAscending )
            nCompare = - nCompare;

        pInfo = pInfo->mpNext;
    }

    return nCompare;
}


sal_IntPtr SortedResultSet::Compare( SortListData const *pOne,
                               SortListData const *pTwo )
{
    sal_IntPtr nIndexOne;
    sal_IntPtr nIndexTwo;

    Reference < XResultSet > xResultOne;
    Reference < XResultSet > xResultTwo;

    if ( pOne->mbModified )
    {
        xResultOne = mxOther;
        nIndexOne = pOne->mnOldPos;
    }
    else
    {
        xResultOne = mxOriginal;
        nIndexOne = pOne->mnCurPos;
    }

    if ( pTwo->mbModified )
    {
        xResultTwo = mxOther;
        nIndexTwo = pTwo->mnOldPos;
    }
    else
    {
        xResultTwo = mxOriginal;
        nIndexTwo = pTwo->mnCurPos;
    }

    sal_IntPtr nCompare;
    nCompare = CompareImpl( xResultOne, xResultTwo,
                            nIndexOne, nIndexTwo );
    return nCompare;
}


sal_IntPtr SortedResultSet::FindPos( SortListData const *pEntry,
                               sal_IntPtr _nStart, sal_IntPtr _nEnd )
{
    if ( _nStart > _nEnd )
        return _nStart + 1;

    sal_IntPtr nStart = _nStart;
    sal_IntPtr nEnd   = _nEnd;
    sal_IntPtr nMid = 0, nCompare = 0;


    while ( nStart <= nEnd )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        SortListData *pMid = maS2O.GetData( nMid );
        nCompare = Compare( pEntry, pMid );

        if ( !nCompare )
            nCompare = (pEntry != pMid) ? ((pEntry < pMid) ? -1 : 1) : 0;

        if ( nCompare < 0 ) // pEntry < pMid
            nEnd = nMid - 1;
        else
            nStart = nMid + 1;
    }

    if ( nCompare < 0 )     // pEntry < pMid
        return nMid;
    else
        return nMid+1;
}


void SortedResultSet::PropertyChanged( const PropertyChangeEvent& rEvt )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        return;

    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper* pPropsContainer =
            mpPropChangeListeners->getContainer( rEvt.PropertyName );
    if ( pPropsContainer )
    {
        OInterfaceIteratorHelper aIter( *pPropsContainer );
        while ( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }

    // Notify listeners interested in all properties.
    pPropsContainer = mpPropChangeListeners->getContainer( OUString() );
    if ( pPropsContainer )
    {
        OInterfaceIteratorHelper aIter( *pPropsContainer );
        while ( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }
}


// public methods


void SortedResultSet::CopyData( SortedResultSet *pSource )
{
    const SortedEntryList& rSrcS2O = pSource->maS2O;

    sal_IntPtr i, nCount;

    maS2O.Clear();
    m_O2S.clear();
    m_ModList.clear();

    maS2O.Insert( nullptr, 0 );
    m_O2S.push_back(0);

    nCount = rSrcS2O.Count();

    for ( i=1; i<nCount; i++ )
    {
        maS2O.Insert( std::unique_ptr<SortListData>(new SortListData( rSrcS2O[ i ] )), i );
        m_O2S.push_back(pSource->m_O2S[i]);
    }

    mnLastSort = maS2O.Count();
    mxOther = pSource->mxOriginal;

    if ( !mpSortInfo )
    {
        mpSortInfo = pSource->mpSortInfo;
        mbIsCopy = true;
    }
}


void SortedResultSet::Initialize(
                const Sequence < NumberedSortingInfo > &xSortInfo,
                const Reference< XAnyCompareFactory > &xCompFactory )
{
    BuildSortInfo( mxOriginal, xSortInfo, xCompFactory );
    // Insert dummy at pos 0
    maS2O.Insert( std::unique_ptr<SortListData>(new SortListData( 0 )), 0 );

    sal_IntPtr nIndex = 1;

    // now fetch all the elements from the original result set,
    // get there new position in the sorted result set and insert
    // an entry in the sorted to original mapping list
    try {
        while ( mxOriginal->absolute( nIndex ) )
        {
            std::unique_ptr<SortListData> pData(new SortListData( nIndex ));
            sal_IntPtr nPos   = FindPos( pData.get(), 1, nIndex-1 );

            maS2O.Insert( std::move(pData), nPos );

            nIndex++;
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::Initialize() : Got unexpected SQLException" );
    }

    // when we have fetched all the elements, we can create the
    // original to sorted mapping list from the s2o list
    m_O2S.clear();
    m_O2S.push_back(0);

    // insert some dummy entries first and replace then
    // the entries with the right ones
    size_t i;

    for ( i=1; i<maS2O.Count(); i++ )
        m_O2S.push_back(0);
    for ( i=1; i<maS2O.Count(); i++ )
        m_O2S[maS2O[i]] = i;

    mnCount = maS2O.Count() - 1;
}


void SortedResultSet::CheckProperties( sal_IntPtr nOldCount, bool bWasFinal )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        return;

    try {
        // check for propertyChangeEvents
        if ( nOldCount != GetCount() )
        {
            bool bIsFinal = false;
            PropertyChangeEvent aEvt;

            aEvt.PropertyName = "RowCount";
            aEvt.Further = false;
            aEvt.PropertyHandle = -1;
            aEvt.OldValue <<= nOldCount;
            aEvt.NewValue <<= GetCount();

            PropertyChanged( aEvt );

            OUString aName = "IsRowCountFinal";
            Any aRet = getPropertyValue( aName );
            if ( (aRet >>= bIsFinal) && bIsFinal != bWasFinal )
            {
                aEvt.PropertyName = aName;
                aEvt.Further = false;
                aEvt.PropertyHandle = -1;
                aEvt.OldValue <<= bWasFinal;
                aEvt.NewValue <<= bIsFinal;
                PropertyChanged( aEvt );
            }
        }
    }
    catch (const UnknownPropertyException&) {}
    catch (const WrappedTargetException&) {}
}


void SortedResultSet::InsertNew( sal_IntPtr nPos, sal_IntPtr nCount )
{
    // for all entries in the msS20-list, which are >= nPos, increase by nCount
    sal_IntPtr      i, nEnd;

    nEnd = maS2O.Count();
    for ( i=1; i<=nEnd; i++ )
    {
        SortListData *pData = maS2O.GetData( i );
        if ( pData->mnCurPos >= nPos )
        {
            pData->mnCurPos += nCount;
        }
    }

    // and append the new entries at the end of the maS20-list or insert at the
    // position nPos in the maS2O-list
    for ( i=0; i<nCount; i++ )
    {
        nEnd += 1;
        std::unique_ptr<SortListData> pData(new SortListData( nEnd ));

        maS2O.Insert( std::move(pData), nEnd );    // Insert( Value, Position )
        m_O2S.insert(m_O2S.begin() + nPos + i, nEnd);
    }

    mnCount += nCount;
}


void SortedResultSet::Remove( sal_IntPtr nPos, sal_IntPtr nCount, EventList *pEvents )
{
    sal_uInt32  i;
    sal_IntPtr        nOldLastSort;

    // correct mnLastSort first
    nOldLastSort = mnLastSort;
    if ( nPos <= mnLastSort )
    {
        if ( nPos + nCount - 1 <= mnLastSort )
            mnLastSort -= nCount;
        else
            mnLastSort = nPos - 1;
    }

    // remove the entries from the lists and correct the positions
    // in the original2sorted list
    for ( i=0; i < static_cast<sal_uInt32>(nCount); i++ )
    {
        sal_IntPtr nSortPos = m_O2S[nPos];
        m_O2S.erase(m_O2S.begin() + nPos);

        for (size_t j=1; j < m_O2S.size(); ++j)
        {
            sal_IntPtr nVal = m_O2S[j];
            if ( nVal > nSortPos )
            {
                --nVal;
                m_O2S[j] = nVal;
            }
        }

        std::unique_ptr<SortListData> pData = maS2O.Remove( nSortPos );
        if ( pData->mbModified )
            m_ModList.erase(std::find(m_ModList.begin(), m_ModList.end(), pData.get()));

        // generate remove Event, but not for new entries
        if ( nSortPos <= nOldLastSort )
            pEvents->AddEvent( ListActionType::REMOVED, nSortPos );
    }

    // correct the positions in the sorted list
    for ( i=1; i<= maS2O.Count(); i++ )
    {
        SortListData *pData = maS2O.GetData( i );
        if ( pData->mnCurPos > nPos )
            pData->mnCurPos -= nCount;
    }

    mnCount -= nCount;
}


void SortedResultSet::Move( sal_IntPtr nPos, sal_IntPtr nCount, sal_IntPtr nOffset )
{
    if ( !nOffset )
        return;

    sal_IntPtr i, nSortPos, nTo;
    SortListData *pData;

    for ( i=0; i<nCount; i++ )
    {
        nSortPos = m_O2S[nPos + i];
        pData = maS2O.GetData( nSortPos );
        pData->mnCurPos += nOffset;
    }

    if ( nOffset < 0 )
    {
        for ( i=nPos+nOffset; i<nPos; i++ )
        {
            nSortPos = m_O2S[i];
            pData = maS2O.GetData( nSortPos );
            pData->mnCurPos += nCount;
        }
    }
    else
    {
        sal_IntPtr nStart = nPos + nCount;
        sal_IntPtr nEnd = nStart + nOffset;
        for ( i=nStart; i<nEnd; i++ )
        {
            nSortPos = m_O2S[i];
            pData = maS2O.GetData( nSortPos );
            pData->mnCurPos -= nCount;
        }
    }

    // remember the to be moved entries
    std::unique_ptr<sal_IntPtr[]> pTmpArr(new sal_IntPtr[ nCount ]);
    for ( i=0; i<nCount; i++ )
        pTmpArr[i] = m_O2S[nPos + i];

    // now move the entries, which are in the way
    if ( nOffset < 0 )
    {
        // be carefully here, because nOffset is negative here, so an
        // addition is a subtraction
        sal_IntPtr nFrom = nPos - 1;
        nTo = nPos + nCount - 1;

        // same for i here
        for ( i=0; i>nOffset; i-- )
        {
            sal_IntPtr const nVal = m_O2S[nFrom + i];
            m_O2S[nTo + i] = nVal;
        }

    }
    else
    {
        sal_IntPtr nStart = nPos + nCount;
        for ( i=0; i<nOffset; i++ )
        {
            sal_IntPtr const nVal = m_O2S[nStart + i];
            m_O2S[nPos + i] = nVal;
        }
    }

    // finally put the remembered entries at their new location
    nTo = nPos + nOffset;
    for ( i=0; i<nCount; i++ )
    {
        m_O2S[nTo + i] = pTmpArr[i];
    }
}


void SortedResultSet::BuildSortInfo(
                const Reference< XResultSet >& aResult,
                const Sequence < NumberedSortingInfo > &xSortInfo,
                const Reference< XAnyCompareFactory > &xCompFactory )
{
    Reference < XResultSetMetaDataSupplier > xMeta ( aResult, UNO_QUERY );

    if ( ! xMeta.is() )
    {
        OSL_FAIL( "No MetaData, No Sorting!" );
        return;
    }

    Reference < XResultSetMetaData > xData = xMeta->getMetaData();
    const NumberedSortingInfo *pSortInfo = xSortInfo.getConstArray();

    sal_Int32   nColumn;
    OUString    aPropName;
    SortInfo    *pInfo;

    for ( sal_Int32 i=xSortInfo.getLength(); i > 0; )
    {
        --i;
        nColumn = pSortInfo[ i ].ColumnIndex;
        aPropName = xData->getColumnName( nColumn );
        pInfo = new SortInfo;

        if ( xCompFactory.is() )
            pInfo->mxCompareFunction = xCompFactory->createAnyCompareByName(
                                            aPropName );

        if ( pInfo->mxCompareFunction.is() )
        {
            pInfo->mbUseOwnCompare = false;
            pInfo->mnType = 0;
        }
        else
        {
            pInfo->mbUseOwnCompare = true;
            pInfo->mnType = xData->getColumnType( nColumn );
        }

        pInfo->mnColumn = nColumn;
        pInfo->mbAscending = pSortInfo[ i ].Ascending;
        pInfo->mbCaseSensitive = xData->isCaseSensitive( nColumn );
        pInfo->mpNext = mpSortInfo;
        mpSortInfo = pInfo;
    }
}


void SortedResultSet::SetChanged( sal_IntPtr nPos, sal_IntPtr nCount )
{
    for ( sal_IntPtr i=0; i<nCount; i++ )
    {
        sal_IntPtr const nSortPos = m_O2S[nPos];
        if ( nSortPos < mnLastSort )
        {
            SortListData *pData = maS2O.GetData( nSortPos );
            if ( ! pData->mbModified )
            {
                pData->mbModified = true;
                m_ModList.push_back(pData);
            }
        }
        nPos += 1;
    }
}


void SortedResultSet::ResortModified( EventList* pList )
{
    sal_IntPtr nCompare, nCurPos, nNewPos;
    sal_IntPtr nStart, nEnd, nOffset, nVal;

    try {
        for (size_t i = 0; i < m_ModList.size(); ++i)
        {
            SortListData *const pData = m_ModList[i];
            nCompare = CompareImpl( mxOther, mxOriginal,
                                    pData->mnOldPos, pData->mnCurPos );
            pData->mbModified = false;
            if ( nCompare != 0 )
            {
                nCurPos = m_O2S[pData->mnCurPos];
                if ( nCompare < 0 )
                {
                    nNewPos = FindPos( pData, 1, nCurPos-1 );
                    nStart = nNewPos;
                    nEnd = nCurPos;
                    nOffset = 1;
                }
                else
                {
                    nNewPos = FindPos( pData, nCurPos+1, mnLastSort );
                    nStart = nCurPos;
                    nEnd = mnLastSort;
                    nOffset = -1;
                }

                if ( nNewPos != nCurPos )
                {
                    // correct the lists!
                    maS2O.Move( static_cast<sal_uInt32>(nCurPos), nNewPos );
                    for (size_t j = 1; j < m_O2S.size(); ++j)
                    {
                        nVal = m_O2S[j];
                        if ( ( nStart <= nVal ) && ( nVal <= nEnd ) )
                        {
                            nVal += nOffset;
                            m_O2S[j] = nVal;
                        }
                    }

                    m_O2S[pData->mnCurPos] = nNewPos;

                    std::unique_ptr<ListAction> pAction(new ListAction);
                    pAction->Position = nCurPos;
                    pAction->Count = 1;
                    pAction->ListActionType = ListActionType::MOVED;
                    pAction->ActionInfo <<= nNewPos-nCurPos;
                    pList->Insert( std::move(pAction) );
                }
                pList->AddEvent( ListActionType::PROPERTIES_CHANGED, nNewPos );
            }
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::ResortModified() : Got unexpected SQLException" );
    }

    m_ModList.clear();
}


void SortedResultSet::ResortNew( EventList* pList )
{
    sal_IntPtr            i, nNewPos, nVal;

    try {
        for ( i = mnLastSort; i<static_cast<sal_IntPtr>(maS2O.Count()); i++ )
        {
            SortListData *const pData = m_ModList[i];
            nNewPos = FindPos( pData, 1, mnLastSort );
            if ( nNewPos != i )
            {
                maS2O.Move( static_cast<sal_uInt32>(i), nNewPos );
                for (size_t j=1; j< m_O2S.size(); ++j)
                {
                    nVal = m_O2S[j];
                    if ( nVal >= nNewPos )
                        m_O2S[j] = nVal + 1;
                }
                m_O2S[pData->mnCurPos] = nNewPos;
            }
            mnLastSort++;
            pList->AddEvent( ListActionType::INSERTED, nNewPos );
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::ResortNew() : Got unexpected SQLException" );
    }
}


// SortListData


SortListData::SortListData( sal_IntPtr nPos )
    : mbModified(false)
    , mnCurPos(nPos)
    , mnOldPos(nPos)
{
};

SortedEntryList::SortedEntryList()
{
}

SortedEntryList::~SortedEntryList()
{
}

void SortedEntryList::Clear()
{
    maData.clear();
}


void SortedEntryList::Insert( std::unique_ptr<SortListData> pEntry, sal_IntPtr nPos )
{
    if ( nPos < static_cast<sal_IntPtr>(maData.size()) )
        maData.insert( maData.begin() + nPos, std::move(pEntry) );
    else
        maData.push_back( std::move(pEntry) );
}

void SortedEntryList::Move( sal_IntPtr nOldPos, sal_IntPtr nNewPos )
{
    auto p = std::move(maData[nOldPos]);
    maData.erase( maData.begin() + nOldPos );
    maData.insert(maData.begin() + nNewPos, std::move(p));
}

std::unique_ptr<SortListData> SortedEntryList::Remove( sal_IntPtr nPos )
{
    std::unique_ptr<SortListData> pData;

    if ( nPos < static_cast<sal_IntPtr>(maData.size()) )
    {
        pData = std::move(maData[ nPos ]);
        maData.erase( maData.begin() + nPos );
    }

    return pData;
}


SortListData* SortedEntryList::GetData( sal_IntPtr nPos )
{
    SortListData *pData;

    if ( nPos < static_cast<sal_IntPtr>(maData.size()) )
        pData = maData[ nPos ].get();
    else
        pData = nullptr;

    return pData;
}


sal_IntPtr SortedEntryList::operator [] ( sal_IntPtr nPos ) const
{
    SortListData *pData;

    if ( nPos < static_cast<sal_IntPtr>(maData.size()) )
        pData = maData[ nPos ].get();
    else
        pData = nullptr;

    if ( pData )
        if ( ! pData->mbModified )
            return pData->mnCurPos;
        else
        {
            OSL_FAIL( "SortedEntryList: Can't get value for modified entry!");
            return 0;
        }
    else
    {
        OSL_FAIL( "SortedEntryList: invalid pos!");
        return 0;
    }
}

// class SRSPropertySetInfo.

SRSPropertySetInfo::SRSPropertySetInfo()
{
    maProps[0].Name = "RowCount";
    maProps[0].Handle = -1;
    maProps[0].Type = cppu::UnoType<OUString>::get();
    maProps[0].Attributes = -1;

    maProps[1].Name = "IsRowCountFinal";
    maProps[1].Handle = -1;
    maProps[1].Type = cppu::UnoType<bool>::get();
    maProps[1].Attributes = -1;
}

// XPropertySetInfo methods.

Sequence< Property > SAL_CALL
SRSPropertySetInfo::getProperties()
{
    return Sequence < Property > ( maProps, 2 );
}


Property SAL_CALL
SRSPropertySetInfo::getPropertyByName( const OUString& Name )
{
    if ( Name == "RowCount" )
        return maProps[0];
    else if ( Name == "IsRowCountFinal" )
        return maProps[1];
    else
        throw UnknownPropertyException();
}


sal_Bool SAL_CALL
SRSPropertySetInfo::hasPropertyByName( const OUString& Name )
{
    if ( Name == "RowCount" )
        return true;
    else if ( Name == "IsRowCountFinal" )
        return true;
    else
        return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
