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
#include <sortresult.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
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
using namespace cppu;




//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex* pMutex = nullptr;
    if( !pMutex )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            static osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }

    return *pMutex;
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
    bool    mbModified;
    sal_IntPtr  mnCurPos;
    sal_IntPtr  mnOldPos;

    SortListData( sal_IntPtr nPos, bool bModified = false );
};



// class SRSPropertySetInfo.



class SRSPropertySetInfo : public cppu::WeakImplHelper <
    XPropertySetInfo >
{
    Property    maProps[2];

private:

public:
                SRSPropertySetInfo();
    virtual     ~SRSPropertySetInfo();

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties()
        throw( RuntimeException, std::exception ) override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName )
        throw( UnknownPropertyException, RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw( RuntimeException, std::exception ) override;
};

typedef OMultiTypeInterfaceContainerHelperVar<OUString>
    PropertyChangeListenerContainer_Impl;

class PropertyChangeListeners_Impl : public PropertyChangeListenerContainer_Impl
{
public:
    PropertyChangeListeners_Impl()
    : PropertyChangeListenerContainer_Impl( getContainerMutex() ) {}
};


SortedResultSet::SortedResultSet( Reference< XResultSet > aResult )
{
    mpDisposeEventListeners = nullptr;
    mpPropChangeListeners   = nullptr;
    mpVetoChangeListeners   = nullptr;
    mpPropSetInfo           = nullptr;

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

    if ( mpPropSetInfo )
        mpPropSetInfo->release();

    delete mpPropChangeListeners;
    delete mpVetoChangeListeners;
}


// XServiceInfo methods.

OUString SAL_CALL SortedResultSet::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString SortedResultSet::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.ucb.SortedResultSet" );
}

sal_Bool SAL_CALL SortedResultSet::supportsService( const OUString& ServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedResultSet::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence< OUString >SortedResultSet::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSNS { RESULTSET_SERVICE_NAME };
    return aSNS;
}


// XComponent methods.

void SAL_CALL SortedResultSet::dispose()
    throw( RuntimeException, std::exception )
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
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpDisposeEventListeners )
        mpDisposeEventListeners =
                    new OInterfaceContainerHelper( getContainerMutex() );

    mpDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL SortedResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners )
        mpDisposeEventListeners->removeInterface( Listener );
}


// XContentAccess methods.


OUString SAL_CALL
SortedResultSet::queryContentIdentifierString()
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentifierString();
}


Reference< XContentIdentifier > SAL_CALL
SortedResultSet::queryContentIdentifier()
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentifier();
}


Reference< XContent > SAL_CALL
SortedResultSet::queryContent()
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContent();
}



// XResultSet methods.

sal_Bool SAL_CALL SortedResultSet::next()
    throw ( SQLException, RuntimeException, std::exception )
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
    return sal_False;
}


sal_Bool SAL_CALL SortedResultSet::isBeforeFirst()
    throw ( SQLException, RuntimeException, std::exception )
{
    if ( mnCurEntry )
        return sal_False;
    else
        return sal_True;
}


sal_Bool SAL_CALL SortedResultSet::isAfterLast()
    throw ( SQLException, RuntimeException, std::exception )
{
    if ( mnCurEntry > mnCount )
        return sal_True;
    else
        return sal_False;
}


sal_Bool SAL_CALL SortedResultSet::isFirst()
    throw ( SQLException, RuntimeException, std::exception )
{
    if ( mnCurEntry == 1 )
        return sal_True;
    else
        return sal_False;
}


sal_Bool SAL_CALL SortedResultSet::isLast()
    throw ( SQLException, RuntimeException, std::exception )
{
    if ( mnCurEntry == mnCount )
        return sal_True;
    else
        return sal_False;
}


void SAL_CALL SortedResultSet::beforeFirst()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = 0;
    mxOriginal->beforeFirst();
}


void SAL_CALL SortedResultSet::afterLast()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = mnCount+1;
    mxOriginal->afterLast();
}


sal_Bool SAL_CALL SortedResultSet::first()
    throw ( SQLException, RuntimeException, std::exception )
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
        return sal_False;
    }
}


sal_Bool SAL_CALL SortedResultSet::last()
    throw ( SQLException, RuntimeException, std::exception )
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
        return sal_False;
    }
}


sal_Int32 SAL_CALL SortedResultSet::getRow()
    throw ( SQLException, RuntimeException, std::exception )
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
    throw ( SQLException, RuntimeException, std::exception )
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
            return sal_False;
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
            return sal_False;
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
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    if ( rows == 0 )
        return sal_True;

    sal_Int32 nTmp = mnCurEntry + rows;

    if ( nTmp <= 0 )
    {
        mnCurEntry = 0;
        return sal_False;
    }
    else if ( nTmp > mnCount )
    {
        mnCurEntry = mnCount + 1;
        return sal_False;
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
    throw ( SQLException, RuntimeException, std::exception )
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

    return sal_False;
}


void SAL_CALL SortedResultSet::refreshRow()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    mxOriginal->refreshRow();
}


sal_Bool SAL_CALL SortedResultSet::rowUpdated()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowUpdated();
}


sal_Bool SAL_CALL SortedResultSet::rowInserted()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowInserted();
}


sal_Bool SAL_CALL SortedResultSet::rowDeleted()
    throw ( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowDeleted();
}


Reference< XInterface > SAL_CALL SortedResultSet::getStatement()
    throw ( SQLException, RuntimeException, std::exception )
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
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->wasNull();
}


OUString SAL_CALL SortedResultSet::getString( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getString( columnIndex );
}


sal_Bool SAL_CALL SortedResultSet::getBoolean( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBoolean( columnIndex );
}


sal_Int8 SAL_CALL SortedResultSet::getByte( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getByte( columnIndex );
}


sal_Int16 SAL_CALL SortedResultSet::getShort( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getShort( columnIndex );
}


sal_Int32 SAL_CALL SortedResultSet::getInt( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getInt( columnIndex );
}

sal_Int64 SAL_CALL SortedResultSet::getLong( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getLong( columnIndex );
}


float SAL_CALL SortedResultSet::getFloat( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getFloat( columnIndex );
}


double SAL_CALL SortedResultSet::getDouble( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDouble( columnIndex );
}


Sequence< sal_Int8 > SAL_CALL SortedResultSet::getBytes( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBytes( columnIndex );
}


Date SAL_CALL SortedResultSet::getDate( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDate( columnIndex );
}


Time SAL_CALL SortedResultSet::getTime( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTime( columnIndex );
}


DateTime SAL_CALL SortedResultSet::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTimestamp( columnIndex );
}


Reference< XInputStream > SAL_CALL
SortedResultSet::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBinaryStream( columnIndex );
}


Reference< XInputStream > SAL_CALL
SortedResultSet::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getCharacterStream( columnIndex );
}


Any SAL_CALL SortedResultSet::getObject( sal_Int32 columnIndex,
                       const Reference< XNameAccess >& typeMap )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getObject( columnIndex,
                                                            typeMap);
}


Reference< XRef > SAL_CALL SortedResultSet::getRef( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getRef( columnIndex );
}


Reference< XBlob > SAL_CALL SortedResultSet::getBlob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBlob( columnIndex );
}


Reference< XClob > SAL_CALL SortedResultSet::getClob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getClob( columnIndex );
}


Reference< XArray > SAL_CALL SortedResultSet::getArray( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getArray( columnIndex );
}



// XCloseable methods.


void SAL_CALL SortedResultSet::close()
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    Reference< XCloseable >::query(mxOriginal)->close();
}


// XResultSetMetaDataSupplier methods.


Reference< XResultSetMetaData > SAL_CALL SortedResultSet::getMetaData()
    throw( SQLException, RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XResultSetMetaDataSupplier >::query(mxOriginal)->getMetaData();
}



// XPropertySet methods.


Reference< XPropertySetInfo > SAL_CALL
SortedResultSet::getPropertySetInfo() throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropSetInfo )
    {
        mpPropSetInfo = new SRSPropertySetInfo();
        mpPropSetInfo->acquire();
    }

    return Reference< XPropertySetInfo >( mpPropSetInfo );
}


void SAL_CALL SortedResultSet::setPropertyValue(
                        const OUString& PropertyName,
                        const Any& )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( PropertyName == "RowCount" || PropertyName == "IsRowCountFinal" )
        throw IllegalArgumentException();
    else
        throw UnknownPropertyException();
}


Any SAL_CALL SortedResultSet::getPropertyValue( const OUString& PropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
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
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        mpPropChangeListeners =
                    new PropertyChangeListeners_Impl();

    mpPropChangeListeners->addInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::removePropertyChangeListener(
                        const OUString& PropertyName,
                        const Reference< XPropertyChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpPropChangeListeners )
        mpPropChangeListeners->removeInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::addVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpVetoChangeListeners )
        mpVetoChangeListeners =
                    new PropertyChangeListeners_Impl();

    mpVetoChangeListeners->addInterface( PropertyName, Listener );
}


void SAL_CALL SortedResultSet::removeVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpVetoChangeListeners )
        mpVetoChangeListeners->removeInterface( PropertyName, Listener );
}


// private methods

sal_IntPtr SortedResultSet::CompareImpl( Reference < XResultSet > xResultOne,
                                   Reference < XResultSet > xResultTwo,
                                   sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo,
                                   SortInfo* pSortInfo )

    throw( SQLException, RuntimeException )
{
    Reference < XRow > xRowOne( xResultOne, UNO_QUERY );
    Reference < XRow > xRowTwo( xResultTwo, UNO_QUERY );

    sal_IntPtr nCompare = 0;
    sal_IntPtr nColumn = pSortInfo->mnColumn;

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

                nTmp = (sal_Int32) aTwo.Year - (sal_Int32) aOne.Year;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Month - (sal_Int32) aOne.Month;
                if ( !nTmp )
                    nTmp = (sal_Int32) aTwo.Day - (sal_Int32) aOne.Day;
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

                nTmp = (sal_Int32) aTwo.Hours - (sal_Int32) aOne.Hours;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Minutes - (sal_Int32) aOne.Minutes;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Seconds - (sal_Int32) aOne.Seconds;
                if ( !nTmp )
                    nTmp = (sal_Int32) aTwo.NanoSeconds
                                    - (sal_Int32) aOne.NanoSeconds;
                }}

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

                nTmp = (sal_Int32) aTwo.Year - (sal_Int32) aOne.Year;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Month - (sal_Int32) aOne.Month;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Day - (sal_Int32) aOne.Day;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Hours - (sal_Int32) aOne.Hours;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Minutes - (sal_Int32) aOne.Minutes;
                if ( !nTmp ) {
                    nTmp = (sal_Int32) aTwo.Seconds - (sal_Int32) aOne.Seconds;
                if ( !nTmp )
                    nTmp = (sal_Int32) aTwo.NanoSeconds
                                    - (sal_Int32) aOne.NanoSeconds;
                }}}}}

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


sal_IntPtr SortedResultSet::CompareImpl( Reference < XResultSet > xResultOne,
                                   Reference < XResultSet > xResultTwo,
                                   sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo )
    throw( SQLException, RuntimeException )
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


sal_IntPtr SortedResultSet::Compare( SortListData *pOne,
                               SortListData *pTwo )
    throw( SQLException, RuntimeException )
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


sal_IntPtr SortedResultSet::FindPos( SortListData *pEntry,
                               sal_IntPtr _nStart, sal_IntPtr _nEnd )
    throw( SQLException, RuntimeException )
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
    const SortedEntryList& rSrcS2O = pSource->GetS2OList();
    const SimpleList&      rSrcO2S = pSource->GetO2SList();

    sal_IntPtr i, nCount;

    maS2O.Clear();
    maO2S.Clear();
    maModList.Clear();

    maS2O.Insert( nullptr, 0 );
    maO2S.Insert( nullptr, (sal_uInt32) 0 );  // value, pos

    nCount = rSrcS2O.Count();

    for ( i=1; i<nCount; i++ )
    {
        maS2O.Insert( new SortListData( rSrcS2O[ i ] ), i );
        maO2S.Insert( rSrcO2S.GetObject( i ), (sal_uInt32) i );
    }

    mnLastSort = maS2O.Count();
    mxOther = pSource->GetResultSet();

    if ( !mpSortInfo )
    {
        mpSortInfo = pSource->GetSortInfo();
        mbIsCopy = true;
    }
}


void SortedResultSet::Initialize(
                const Sequence < NumberedSortingInfo > &xSortInfo,
                const Reference< XAnyCompareFactory > &xCompFactory )
{
    BuildSortInfo( mxOriginal, xSortInfo, xCompFactory );
    // Insert dummy at pos 0
    SortListData *pData = new SortListData( 0 );
    maS2O.Insert( pData, 0 );

    sal_IntPtr nIndex = 1;

    // now fetch all the elements from the original result set,
    // get there new position in the sorted result set and insert
    // an entry in the sorted to original mapping list
    try {
        while ( mxOriginal->absolute( nIndex ) )
        {
            pData       = new SortListData( nIndex );
            sal_IntPtr nPos   = FindPos( pData, 1, nIndex-1 );

            maS2O.Insert( pData, nPos );

            nIndex++;
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::Initialize() : Got unexpected SQLException" );
    }

    // when we have fetched all the elements, we can create the
    // original to sorted mapping list from the s2o list
    maO2S.Clear();
    maO2S.Insert( nullptr, (sal_uInt32) 0 );

    // insert some dummy entries first and replace then
    // the entries with the right ones
    size_t i;

    for ( i=1; i<maS2O.Count(); i++ )
        maO2S.Insert( nullptr, i );   // Insert( data, pos )
    for ( i=1; i<maS2O.Count(); i++ )
        maO2S.Replace( reinterpret_cast<void*>(i), maS2O[ i ] ); // Insert( data, pos )

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
            aEvt.Further = sal_False;
            aEvt.PropertyHandle = -1;
            aEvt.OldValue <<= nOldCount;
            aEvt.NewValue <<= GetCount();

            PropertyChanged( aEvt );

            OUString aName = "IsRowCountFinal";
            Any aRet = getPropertyValue( aName );
            if ( (aRet >>= bIsFinal) && bIsFinal != bWasFinal )
            {
                aEvt.PropertyName = aName;
                aEvt.Further = sal_False;
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
    SortListData    *pData;
    sal_IntPtr      i, nEnd;

    nEnd = maS2O.Count();
    for ( i=1; i<=nEnd; i++ )
    {
        pData = maS2O.GetData( i );
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
        pData = new SortListData( nEnd );

        maS2O.Insert( pData, nEnd );    // Insert( Value, Position )
        maO2S.Insert( reinterpret_cast<void*>(nEnd), (sal_uInt32)(nPos+i) );  // Insert( Value, Position )
    }

    mnCount += nCount;
}


void SortedResultSet::Remove( sal_IntPtr nPos, sal_IntPtr nCount, EventList *pEvents )
{
    sal_uInt32  i, j;
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
    for ( i=0; i < (sal_uInt32) nCount; i++ )
    {
        sal_IntPtr nSortPos = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( nPos ) );
        maO2S.Remove( (sal_uInt32) nPos );

        for ( j=1; j<=maO2S.Count(); j++ )
        {
            sal_IntPtr nVal = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( j ) );
            if ( nVal > nSortPos )
            {
                --nVal;
                maO2S.Replace( reinterpret_cast<void*>(nVal), j );
            }
        }

        SortListData *pData = maS2O.Remove( nSortPos );
        if ( pData->mbModified )
            maModList.Remove( static_cast<void*>(pData) );
        delete pData;

        // generate remove Event, but not for new entries
        if ( nSortPos <= nOldLastSort )
            pEvents->AddEvent( ListActionType::REMOVED, nSortPos, 1 );
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
        nSortPos = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( nPos+i ));
        pData = maS2O.GetData( nSortPos );
        pData->mnCurPos += nOffset;
    }

    if ( nOffset < 0 )
    {
        for ( i=nPos+nOffset; i<nPos; i++ )
        {
            nSortPos = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( i ));
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
            nSortPos = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( i ));
            pData = maS2O.GetData( nSortPos );
            pData->mnCurPos -= nCount;
        }
    }

    // remember the to be moved entries
    std::unique_ptr<sal_IntPtr[]> pTmpArr(new sal_IntPtr[ nCount ]);
    for ( i=0; i<nCount; i++ )
        pTmpArr[i] = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( (sal_uInt32)( nPos+i ) ));

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
            sal_IntPtr nVal = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( (sal_uInt32)( nFrom+i ) ) );
            maO2S.Replace( reinterpret_cast<void*>(nVal), (sal_uInt32)( nTo+i ) );
        }

    }
    else
    {
        sal_IntPtr nStart = nPos + nCount;
        for ( i=0; i<nOffset; i++ )
        {
            sal_IntPtr nVal = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( (sal_uInt32)( nStart+i ) ) );
            maO2S.Replace( reinterpret_cast<void*>(nVal), (sal_uInt32)( nPos+i ) );
        }
    }

    // finally put the remembered entries at there new location
    nTo = nPos + nOffset;
    for ( i=0; i<nCount; i++ )
    {
        maO2S.Replace( reinterpret_cast<void*>(pTmpArr[ i ]), (sal_uInt32)( nTo+i ) );
    }
}


void SortedResultSet::BuildSortInfo(
                Reference< XResultSet > aResult,
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

    for ( sal_IntPtr i=xSortInfo.getLength(); i > 0; )
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
        sal_IntPtr nSortPos = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( nPos ));
        if ( nSortPos < mnLastSort )
        {
            SortListData *pData = maS2O.GetData( nSortPos );
            if ( ! pData->mbModified )
            {
                pData->mbModified = true;
                maModList.Append( pData );
            }
        }
        nPos += 1;
    }
}


void SortedResultSet::ResortModified( EventList* pList )
{
    sal_uInt32 i, j;
    sal_IntPtr nCompare, nCurPos, nNewPos;
    sal_IntPtr nStart, nEnd, nOffset, nVal;
    ListAction *pAction;

    try {
        for ( i=0; i<maModList.Count(); i++ )
        {
            SortListData *pData = static_cast<SortListData*>(maModList.GetObject( i ));
            nCompare = CompareImpl( mxOther, mxOriginal,
                                    pData->mnOldPos, pData->mnCurPos );
            pData->mbModified = false;
            if ( nCompare != 0 )
            {
                nCurPos = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( (sal_uInt32) pData->mnCurPos ) );
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
                    maS2O.Remove( (sal_uInt32) nCurPos );
                    maS2O.Insert( pData, nNewPos );
                        for ( j=1; j<maO2S.Count(); j++ )
                    {
                        nVal = reinterpret_cast<sal_IntPtr>( maO2S.GetObject( (sal_uInt32)j ) );
                        if ( ( nStart <= nVal ) && ( nVal <= nEnd ) )
                        {
                            nVal += nOffset;
                            maO2S.Replace( reinterpret_cast<void*>(nVal), (sal_uInt32)j );
                        }
                    }

                    maO2S.Replace( reinterpret_cast<void*>(nNewPos), (sal_uInt32) pData->mnCurPos );

                    pAction = new ListAction;
                    pAction->Position = nCurPos;
                    pAction->Count = 1;
                    pAction->ListActionType = ListActionType::MOVED;
                    pAction->ActionInfo <<= nNewPos-nCurPos;
                    pList->Insert( pAction );
                }
                pList->AddEvent( ListActionType::PROPERTIES_CHANGED,
                                 nNewPos, 1 );
            }
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::ResortModified() : Got unexpected SQLException" );
    }

    maModList.Clear();
}


void SortedResultSet::ResortNew( EventList* pList )
{
    sal_IntPtr            i, j, nNewPos, nVal;

    try {
        for ( i = mnLastSort; i<(sal_IntPtr)maS2O.Count(); i++ )
        {
            SortListData *pData = static_cast<SortListData*>(maModList.GetObject( i ));
            nNewPos = FindPos( pData, 1, mnLastSort );
            if ( nNewPos != i )
            {
                maS2O.Remove( (sal_uInt32) i );
                maS2O.Insert( pData, nNewPos );
                // maO2S liste korigieren
                for ( j=1; j<(sal_IntPtr)maO2S.Count(); j++ )
                {
                    nVal = reinterpret_cast<sal_IntPtr>(maO2S.GetObject( (sal_uInt32)j ));
                    if ( nVal >= nNewPos )
                        maO2S.Replace( reinterpret_cast<void*>(nVal+1), (sal_uInt32)( j ) );
                }
                maO2S.Replace( reinterpret_cast<void*>(nNewPos), (sal_uInt32) pData->mnCurPos );
            }
            mnLastSort++;
            pList->AddEvent( ListActionType::INSERTED, nNewPos, 1 );
        }
    }
    catch (const SQLException&)
    {
        OSL_FAIL( "SortedResultSet::ResortNew() : Got unexpected SQLException" );
    }
}



// SortListData


SortListData::SortListData( sal_IntPtr nPos, bool bModified )
{
    mbModified = bModified;
    mnCurPos = nPos;
    mnOldPos = nPos;
};



void SortedEntryList::Clear()
{
    for ( std::deque< ListAction* >::size_type i = 0;
          i < maData.size(); ++i )
    {
        delete maData[i];
    }

    maData.clear();
}


void SortedEntryList::Insert( SortListData *pEntry, sal_IntPtr nPos )
{
    if ( nPos < (sal_IntPtr) maData.size() )
        maData.insert( maData.begin() + nPos, pEntry );
    else
        maData.push_back( pEntry );
}


SortListData* SortedEntryList::Remove( sal_IntPtr nPos )
{
    SortListData *pData;

    if ( nPos < (sal_IntPtr) maData.size() )
    {
        pData = maData[ nPos ];
        maData.erase( maData.begin() + nPos );
    }
    else
        pData = nullptr;

    return pData;
}


SortListData* SortedEntryList::GetData( sal_IntPtr nPos )
{
    SortListData *pData;

    if ( nPos < (sal_IntPtr) maData.size() )
        pData = maData[ nPos ];
    else
        pData = nullptr;

    return pData;
}


sal_IntPtr SortedEntryList::operator [] ( sal_IntPtr nPos ) const
{
    SortListData *pData;

    if ( nPos < (sal_IntPtr) maData.size() )
        pData = maData[ nPos ];
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




void SimpleList::Remove( sal_uInt32 nPos )
{
    if ( nPos < (sal_uInt32) maData.size() )
    {
        maData.erase( maData.begin() + nPos );
    }
}


void SimpleList::Remove( void* pData )
{
    bool    bFound = false;
    sal_uInt32  i;

    for ( i = 0; i < (sal_uInt32) maData.size(); i++ )
    {
        if ( maData[ i ] == pData )
        {
            bFound = true;
            break;
        }
    }

    if ( bFound )
        maData.erase( maData.begin() + i );
}


void SimpleList::Insert( void* pData, sal_uInt32 nPos )
{
    if ( nPos < (sal_uInt32) maData.size() )
        maData.insert( maData.begin() + nPos, pData );
    else
        maData.push_back( pData );
}


void* SimpleList::GetObject( sal_uInt32 nPos ) const
{
    if ( nPos < (sal_uInt32) maData.size() )
        return maData[ nPos ];
    else
        return nullptr;
}


void SimpleList::Replace( void* pData, sal_uInt32 nPos )
{
    if ( nPos < (sal_uInt32) maData.size() )
        maData[ nPos ] = pData;
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


SRSPropertySetInfo::~SRSPropertySetInfo()
{}

// XPropertySetInfo methods.

Sequence< Property > SAL_CALL
SRSPropertySetInfo::getProperties() throw( RuntimeException, std::exception )
{
    return Sequence < Property > ( maProps, 2 );
}


Property SAL_CALL
SRSPropertySetInfo::getPropertyByName( const OUString& Name )
    throw( UnknownPropertyException, RuntimeException, std::exception )
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
    throw( RuntimeException, std::exception )
{
    if ( Name == "RowCount" )
        return sal_True;
    else if ( Name == "IsRowCountFinal" )
        return sal_True;
    else
        return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
