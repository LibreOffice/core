/*************************************************************************
 *
 *  $RCSfile: sortresult.cxx,v $
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

#include <sortresult.hxx>

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef  _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef  _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef  _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_LISTACTIONTYPE_HPP_
#include <com/sun/star/ucb/ListActionType.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XANYCOMPARE_HPP_
#include <com/sun/star/ucb/XAnyCompare.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//-----------------------------------------------------------------------------
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace rtl;

//=========================================================================

//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex* pMutex = NULL;
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

//==========================================================================

struct SortInfo
{
    BOOL        mbUseOwnCompare;
    BOOL        mbAscending;
    BOOL        mbCaseSensitive;
    sal_Int32   mnColumn;
    sal_Int32   mnType;
    SortInfo*   mpNext;
    Reference < XAnyCompare >   mxCompareFunction;
};

//-----------------------------------------------------------------------------

struct SortListData
{
    sal_Bool    mbModified;
    long        mnCurPos;
    long        mnOldPos;

                SortListData( long nPos, sal_Bool bModified = sal_False );
};

//============================================================================
//
// class SRSPropertySetInfo.
//
//============================================================================

class SRSPropertySetInfo :
                public OWeakObject,
                public XTypeProvider,
                public XPropertySetInfo
{
    Property    maProps[2];

private:

public:
                SRSPropertySetInfo();
    virtual     ~SRSPropertySetInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties()
        throw( RuntimeException );
    virtual Property SAL_CALL getPropertyByName( const OUString& aName )
        throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw( RuntimeException );
};

//=========================================================================
//
// PropertyChangeListenerContainer_Impl.
//
//=========================================================================

struct equalStr_Impl
{
    bool operator()( const OUString& s1, const OUString& s2 ) const
      {
        return !!( s1 == s2 );
    }
};

struct hashStr_Impl
{
    size_t operator()( const OUString& rName ) const
    {
        return rName.hashCode();
    }
};

typedef OMultiTypeInterfaceContainerHelperVar
<
    OUString,
    hashStr_Impl,
    equalStr_Impl
> PropertyChangeListenerContainer_Impl;

//=========================================================================
//
// class PropertyChangeListeners_Impl
//
//=========================================================================

class PropertyChangeListeners_Impl : public PropertyChangeListenerContainer_Impl
{
public:
    PropertyChangeListeners_Impl()
    : PropertyChangeListenerContainer_Impl( getContainerMutex() ) {}
};

//==========================================================================
SortedResultSet::SortedResultSet( Reference< XResultSet > aResult )
{
    mpDisposeEventListeners = NULL;
    mpPropChangeListeners   = NULL;
    mpVetoChangeListeners   = NULL;
    mpPropSetInfo           = NULL;

    mxOriginal  = aResult;
    mpSortInfo  = NULL;
    mnLastSort  = 0;
    mnCurEntry  = 0;
    mnCount     = 0;
    mbIsCopy    = FALSE;
}

//--------------------------------------------------------------------------
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

    mpSortInfo = NULL;

    if ( mpPropSetInfo )
        mpPropSetInfo->release();

    delete mpPropChangeListeners;
    delete mpVetoChangeListeners;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_9( SortedResultSet,
                   XTypeProvider,
                   XServiceInfo,
                   XComponent,
                   XContentAccess,
                   XResultSet,
                   XRow,
                   XCloseable,
                   XResultSetMetaDataSupplier,
                   XPropertySet );

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_9( SortedResultSet,
                      XTypeProvider,
                         XServiceInfo,
                      XComponent,
                      XContentAccess,
                      XResultSet,
                      XRow,
                      XCloseable,
                      XResultSetMetaDataSupplier,
                      XPropertySet );

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( SortedResultSet,
                    OUString::createFromAscii( "SortedResultSet" ),
                    OUString::createFromAscii( RESULTSET_SERVICE_NAME ) );

//--------------------------------------------------------------------------
// XComponent methods.
//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::dispose()
    throw( RuntimeException )
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

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpDisposeEventListeners )
        mpDisposeEventListeners =
                    new OInterfaceContainerHelper( getContainerMutex() );

    mpDisposeEventListeners->addInterface( Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners )
        mpDisposeEventListeners->removeInterface( Listener );
}

//--------------------------------------------------------------------------
// XContentAccess methods.
//--------------------------------------------------------------------------

OUString SAL_CALL
SortedResultSet::queryContentIdentfierString()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentfierString();
}

//--------------------------------------------------------------------------
Reference< XContentIdentifier > SAL_CALL
SortedResultSet::queryContentIdentifier()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContentIdentifier();
}

//--------------------------------------------------------------------------
Reference< XContent > SAL_CALL
SortedResultSet::queryContent()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XContentAccess >::query(mxOriginal)->queryContent();
}


//--------------------------------------------------------------------------
// XResultSet methods.
//--------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::next()
    throw ( SQLException, RuntimeException )
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
    return FALSE;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::isBeforeFirst()
    throw ( SQLException, RuntimeException )
{
    if ( mnCurEntry )
        return sal_False;
    else
        return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::isAfterLast()
    throw ( SQLException, RuntimeException )
{
    if ( mnCurEntry > mnCount )
        return sal_True;
    else
        return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::isFirst()
    throw ( SQLException, RuntimeException )
{
    if ( mnCurEntry == 1 )
        return sal_True;
    else
        return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::isLast()
    throw ( SQLException, RuntimeException )
{
    if ( mnCurEntry == mnCount )
        return sal_True;
    else
        return sal_False;
}

//-------------------------------------------------------------------------
void SAL_CALL SortedResultSet::beforeFirst()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = 0;
    mxOriginal->beforeFirst();
}

//-------------------------------------------------------------------------
void SAL_CALL SortedResultSet::afterLast()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mnCurEntry = mnCount+1;
    mxOriginal->afterLast();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::first()
    throw ( SQLException, RuntimeException )
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

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::last()
    throw ( SQLException, RuntimeException )
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

//-------------------------------------------------------------------------
sal_Int32 SAL_CALL SortedResultSet::getRow()
    throw ( SQLException, RuntimeException )
{
    return mnCurEntry;
}

//-------------------------------------------------------------------------
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
    throw ( SQLException, RuntimeException )
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

//-------------------------------------------------------------------------
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
    throw ( SQLException, RuntimeException )
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

//-------------------------------------------------------------------------
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
    throw ( SQLException, RuntimeException )
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

//-------------------------------------------------------------------------
void SAL_CALL SortedResultSet::refreshRow()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    mxOriginal->refreshRow();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::rowUpdated()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowUpdated();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::rowInserted()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowInserted();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::rowDeleted()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->rowDeleted();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SortedResultSet::getStatement()
    throw ( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( mnCurEntry <= 0 ) || ( mnCurEntry > mnCount ) )
    {
        throw SQLException();
    }

    return mxOriginal->getStatement();
}

//--------------------------------------------------------------------------
// XRow methods.
//--------------------------------------------------------------------------

sal_Bool SAL_CALL SortedResultSet::wasNull()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->wasNull();
}

//-------------------------------------------------------------------------
OUString SAL_CALL SortedResultSet::getString( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getString( columnIndex );
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SortedResultSet::getBoolean( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBoolean( columnIndex );
}

//-------------------------------------------------------------------------
sal_Int8 SAL_CALL SortedResultSet::getByte( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getByte( columnIndex );
}

//-------------------------------------------------------------------------
sal_Int16 SAL_CALL SortedResultSet::getShort( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getShort( columnIndex );
}

//-------------------------------------------------------------------------
sal_Int32 SAL_CALL SortedResultSet::getInt( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getInt( columnIndex );
}
//-------------------------------------------------------------------------
sal_Int64 SAL_CALL SortedResultSet::getLong( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getLong( columnIndex );
}

//-------------------------------------------------------------------------
float SAL_CALL SortedResultSet::getFloat( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getFloat( columnIndex );
}

//-------------------------------------------------------------------------
double SAL_CALL SortedResultSet::getDouble( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDouble( columnIndex );
}

//-------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL SortedResultSet::getBytes( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBytes( columnIndex );
}

//-------------------------------------------------------------------------
Date SAL_CALL SortedResultSet::getDate( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getDate( columnIndex );
}

//-------------------------------------------------------------------------
Time SAL_CALL SortedResultSet::getTime( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTime( columnIndex );
}

//-------------------------------------------------------------------------
DateTime SAL_CALL SortedResultSet::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getTimestamp( columnIndex );
}

//-------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL
SortedResultSet::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBinaryStream( columnIndex );
}

//-------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL
SortedResultSet::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getCharacterStream( columnIndex );
}

//-------------------------------------------------------------------------
Any SAL_CALL SortedResultSet::getObject( sal_Int32 columnIndex,
                       const Reference< XNameAccess >& typeMap )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getObject( columnIndex,
                                                            typeMap);
}

//-------------------------------------------------------------------------
Reference< XRef > SAL_CALL SortedResultSet::getRef( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getRef( columnIndex );
}

//-------------------------------------------------------------------------
Reference< XBlob > SAL_CALL SortedResultSet::getBlob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getBlob( columnIndex );
}

//-------------------------------------------------------------------------
Reference< XClob > SAL_CALL SortedResultSet::getClob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getClob( columnIndex );
}

//-------------------------------------------------------------------------
Reference< XArray > SAL_CALL SortedResultSet::getArray( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XRow >::query(mxOriginal)->getArray( columnIndex );
}


//--------------------------------------------------------------------------
// XCloseable methods.
//--------------------------------------------------------------------------

void SAL_CALL SortedResultSet::close()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    Reference< XCloseable >::query(mxOriginal)->close();
}

//--------------------------------------------------------------------------
// XResultSetMetaDataSupplier methods.
//--------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL SortedResultSet::getMetaData()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    return Reference< XResultSetMetaDataSupplier >::query(mxOriginal)->getMetaData();
}


//--------------------------------------------------------------------------
// XPropertySet methods.
//--------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL
SortedResultSet::getPropertySetInfo() throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropSetInfo )
    {
        mpPropSetInfo = new SRSPropertySetInfo();
        mpPropSetInfo->acquire();
    }

    return Reference< XPropertySetInfo >( mpPropSetInfo );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::setPropertyValue(
                        const OUString& PropertyName,
                        const Any& Value )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( ( PropertyName.compareToAscii( "RowCount" ) == 0 ) ||
         ( PropertyName.compareToAscii( "IsRowCountFinal" ) == 0 ) )
        throw IllegalArgumentException();
    else
        throw UnknownPropertyException();
}

//--------------------------------------------------------------------------
Any SAL_CALL SortedResultSet::getPropertyValue( const OUString& PropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    Any aRet;

    if ( PropertyName.compareToAscii( "RowCount" ) == 0 )
    {
        aRet <<= maS2O.Count();
    }
    else if ( PropertyName.compareToAscii( "IsRowCountFinal" ) == 0 )
    {
        sal_Int32   nOrgCount;
        sal_Bool    bOrgFinal;
        Any         aOrgRet;

        aRet <<= (sal_Bool) FALSE;

        aOrgRet = Reference< XPropertySet >::query(mxOriginal)->
                        getPropertyValue( PropertyName );
        aOrgRet >>= bOrgFinal;

        if ( bOrgFinal )
        {
            aOrgRet = Reference< XPropertySet >::query(mxOriginal)->
                getPropertyValue( OUString::createFromAscii( "RowCount" ) );
            aOrgRet >>= nOrgCount;
            if ( nOrgCount == maS2O.Count() )
                aRet <<= (sal_Bool) TRUE;
        }
    }
    else
        throw UnknownPropertyException();

    return aRet;
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::addPropertyChangeListener(
                        const OUString& PropertyName,
                        const Reference< XPropertyChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        mpPropChangeListeners =
                    new PropertyChangeListeners_Impl();

    mpPropChangeListeners->addInterface( PropertyName, Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::removePropertyChangeListener(
                        const OUString& PropertyName,
                        const Reference< XPropertyChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpPropChangeListeners )
        mpPropChangeListeners->removeInterface( PropertyName, Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::addVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpVetoChangeListeners )
        mpVetoChangeListeners =
                    new PropertyChangeListeners_Impl();

    mpVetoChangeListeners->addInterface( PropertyName, Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedResultSet::removeVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& Listener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpVetoChangeListeners )
        mpVetoChangeListeners->removeInterface( PropertyName, Listener );
}

//--------------------------------------------------------------------------
// private methods
//--------------------------------------------------------------------------
long SortedResultSet::CompareImpl( Reference < XResultSet > xResultOne,
                                   Reference < XResultSet > xResultTwo,
                                   long nIndexOne, long nIndexTwo,
                                   SortInfo* pSortInfo )

{
    Reference < XRow > xRowOne = Reference< XRow >::query( xResultOne );
    Reference < XRow > xRowTwo = Reference< XRow >::query( xResultTwo );

    long nCompare = 0;
    long nColumn = pSortInfo->mnColumn;

    switch ( pSortInfo->mnType )
    {
        case DataType::BIT :
        case DataType::TINYINT :
        case DataType::SMALLINT :
        case DataType::INTEGER :
            {
                sal_Int32 aOne, aTwo;

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
                sal_Int64 aOne, aTwo;

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
                    aOne = aOne.toLowerCase();
                    aTwo = aTwo.toLowerCase();
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
                    nTmp = (sal_Int32) aTwo.HundredthSeconds
                                    - (sal_Int32) aOne.HundredthSeconds;
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
                    nTmp = (sal_Int32) aTwo.HundredthSeconds
                                    - (sal_Int32) aOne.HundredthSeconds;
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
                float aOne, aTwo;

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
                double aOne, aTwo;

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
                OSL_ENSURE( sal_False, "DataType not supported for compare!" );
            }
    }

    return nCompare;
}

//--------------------------------------------------------------------------
long SortedResultSet::CompareImpl( Reference < XResultSet > xResultOne,
                                   Reference < XResultSet > xResultTwo,
                                   long nIndexOne, long nIndexTwo )
{
    long        nCompare = 0;
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
                aOne = xRowOne->getObject( pInfo->mnColumn, NULL );
            if ( xResultTwo->absolute( nIndexTwo ) )
                aTwo = xRowTwo->getObject( pInfo->mnColumn, NULL );

            nCompare = pInfo->mxCompareFunction->compare( aOne, aTwo );
        }

        if ( ! pInfo->mbAscending )
            nCompare = - nCompare;

        pInfo = pInfo->mpNext;
    }

    return nCompare;
}

//--------------------------------------------------------------------------
long SortedResultSet::Compare( SortListData *pOne,
                               SortListData *pTwo )
{
    long nIndexOne;
    long nIndexTwo;

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

    long nCompare;
    nCompare = CompareImpl( xResultOne, xResultTwo,
                            nIndexOne, nIndexTwo );
    return nCompare;
}

//--------------------------------------------------------------------------
long SortedResultSet::FindPos( SortListData *pEntry,
                               long _nStart, long _nEnd )
{
    if ( _nStart > _nEnd )
        return _nStart + 1;

    long nStart = _nStart;
    long nEnd   = _nEnd;
    long nMid, nCompare;

    SortListData    *pMid;

    while ( nStart <= nEnd )
    {
        nMid = ( nEnd - nStart ) / 2 + nStart;
        pMid = maS2O.GetData( nMid );
        nCompare = Compare( pEntry, pMid );

        if ( !nCompare )
            nCompare = ((long) pEntry ) - ( (long) pMid );

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

//--------------------------------------------------------------------------
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

//-------------------------------------------------------------------------

//--------------------------------------------------------------------------
// public methods
//--------------------------------------------------------------------------

void SortedResultSet::CopyData( SortedResultSet *pSource )
{
    const SortedEntryList *pSrcS2O = pSource->GetS2OList();
    const List            *pSrcO2S = pSource->GetO2SList();

    long i, nCount;

    maS2O.Clear();
    maO2S.Clear();
    maModList.Clear();

    maS2O.Insert( NULL, 0 );
    maO2S.Insert( 0, (ULONG) 0 );   // value, pos

    nCount = pSrcS2O->Count();

    for ( i=1; i<nCount; i++ )
    {
        maS2O.Insert( new SortListData( (*pSrcS2O)[ i ] ), i );
        maO2S.Insert( pSrcO2S->GetObject( i ), (ULONG) i );
    }

    mnLastSort = maS2O.Count();
    mxOther = pSource->GetResultSet();

    if ( !mpSortInfo )
    {
        mpSortInfo = pSource->GetSortInfo();
        mbIsCopy = TRUE;
    }
}

//--------------------------------------------------------------------------
void SortedResultSet::Initialize(
                const Sequence < NumberedSortingInfo > &xSortInfo,
                const Reference< XAnyCompareFactory > &xCompFactory )
{
    BuildSortInfo( mxOriginal, xSortInfo, xCompFactory );
    // Insert dummy at pos 0
    SortListData *pData = new SortListData( 0 );
    maS2O.Insert( pData, 0 );

    long nIndex = 1;

    // now fetch all the elements from the original result set,
    // get there new position in the sorted result set and insert
    // an entry in the sorted to original mapping list
    while ( mxOriginal->absolute( nIndex ) )
    {
        pData       = new SortListData( nIndex );
        long nPos   = FindPos( pData, 1, nIndex-1 );

        maS2O.Insert( pData, nPos );

        nIndex++;
    }

    // when we have fetched all the elements, we can create the
    // original to sorted mapping list from the s2o list
    maO2S.Clear();
    maO2S.Insert( NULL, (ULONG) 0 );

    // insert some dummy entries first and replace then
    // the entries with the right ones
    long i;

    for ( i=1; i<maS2O.Count(); i++ )
        maO2S.Insert( (void*) 0, (ULONG) i );   // Insert( data, pos )
    for ( i=1; i<maS2O.Count(); i++ )
        maO2S.Replace( (void*) i, (ULONG) maS2O[ i ] ); // Insert( data, pos )

    mnCount = maS2O.Count() - 1;
}

//--------------------------------------------------------------------------
void SortedResultSet::CheckProperties( long nOldCount, BOOL bWasFinal )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpPropChangeListeners )
        return;

    // check for propertyChangeEvents
    if ( nOldCount != GetCount() )
    {
        BOOL bIsFinal;
        PropertyChangeEvent aEvt;

        aEvt.PropertyName = OUString::createFromAscii( "RowCount" );
        aEvt.Further = FALSE;
        aEvt.PropertyHandle = -1;
        aEvt.OldValue <<= nOldCount;
        aEvt.NewValue <<= GetCount();

        PropertyChanged( aEvt );

        OUString aName = OUString::createFromAscii( "IsRowCountFinal" );
        Any aRet = getPropertyValue( aName );
        aRet >>= bIsFinal;
        if ( bIsFinal != bWasFinal )
        {
            aEvt.PropertyName = aName;
            aEvt.Further = FALSE;
            aEvt.PropertyHandle = -1;
            aEvt.OldValue <<= (sal_Bool) bWasFinal;
            aEvt.NewValue <<= (sal_Bool) bIsFinal;
            PropertyChanged( aEvt );
        }
    }
}

//-------------------------------------------------------------------------
void SortedResultSet::InsertNew( long nPos, long nCount )
{
    // in der maS2O Liste alle Einträge, die >= nPos sind, um nCount
    // erhöhen
    SortListData    *pData;
    long            i, nEnd;

    nEnd = maS2O.Count();
    for ( i=1; i<=nEnd; i++ )
    {
        pData = maS2O.GetData( i );
        if ( pData->mnCurPos >= nPos )
        {
            pData->mnCurPos += nCount;
        }
    }

    // und die neuen einträge hinten an die maS2O Liste anhängen bzw
    // an der Position nPos in der maO2S Liste einfügen
    for ( i=0; i<nCount; i++ )
    {
        nEnd += 1;
        pData = new SortListData( nEnd );

        maS2O.Insert( pData, nEnd );    // Insert( Wert, Position )
        maO2S.Insert( (void*)nEnd, (ULONG)(nPos+i) );   // Insert( Wert, Position )
    }

    mnCount += nCount;
}

//-------------------------------------------------------------------------
void SortedResultSet::Remove( long nPos, long nCount, EventList *pEvents )
{
    long i, j, nOldLastSort;
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
    for ( i=0; i<nCount; i++ )
    {
        long nSortPos = (long) maO2S.GetObject( nPos );
        maO2S.Remove( (ULONG) nPos );

        for ( j=1; j<=maO2S.Count(); j++ )
        {
            long nVal = (long) maO2S.GetObject( (ULONG) j );
            if ( nVal > nSortPos )
            {
                --nVal;
                maO2S.Replace( (void*) nVal, j );
            }
        }

        SortListData *pData = maS2O.Remove( nSortPos );
        if ( pData->mbModified )
            maModList.Remove( (void*) pData );
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

//-------------------------------------------------------------------------
void SortedResultSet::Move( long nPos, long nCount, long nOffset )
{
    if ( !nOffset )
        return;

    long i, nSortPos, nTo;
    SortListData *pData;

    for ( i=0; i<nCount; i++ )
    {
        nSortPos = (long) maO2S.GetObject( nPos+i );
        pData = maS2O.GetData( nSortPos );
        pData->mnCurPos += nOffset;
    }

    if ( nOffset < 0 )
    {
        for ( i=nPos+nOffset; i<nPos; i++ )
        {
            nSortPos = (long) maO2S.GetObject( i );
            pData = maS2O.GetData( nSortPos );
            pData->mnCurPos += nCount;
        }
    }
    else
    {
        long nStart = nPos + nCount;
        long nEnd = nStart + nOffset;
        for ( i=nStart; i<nEnd; i++ )
        {
            nSortPos = (long) maO2S.GetObject( i );
            pData = maS2O.GetData( nSortPos );
            pData->mnCurPos -= nCount;
        }
    }

    // remember the to be moved entries
    long *pTmpArr = new long[ nCount ];
    for ( i=0; i<nCount; i++ )
        pTmpArr[i] = (long)maO2S.GetObject( (ULONG)( nPos+i ) );

    // now move the entries, which are in the way
    if ( nOffset < 0 )
    {
        // be carefully here, because nOffset is negative here, so an
        // addition is a subtraction
        long nFrom = nPos - 1;
        nTo = nPos + nCount - 1;

        // same for i here
        for ( i=0; i>nOffset; i-- )
        {
            long nVal = (long) maO2S.GetObject( (ULONG)( nFrom+i ) );
            maO2S.Replace( (void*) nVal, (ULONG)( nTo+i ) );
        }

    }
    else
    {
        long nStart = nPos + nCount;
        for ( i=0; i<nOffset; i++ )
        {
            long nVal = (long) maO2S.GetObject( (ULONG)( nStart+i ) );
            maO2S.Replace( (void*) nVal, (ULONG)( nPos+i ) );
        }
    }

    // finally put the remembered entries at there new location
    nTo = nPos + nOffset;
    for ( i=0; i<nCount; i++ );
    {
        maO2S.Replace( (void*)pTmpArr[ i ], (ULONG)( nTo+i ) );
    }
}

//--------------------------------------------------------------------------
void SortedResultSet::BuildSortInfo(
                Reference< XResultSet > aResult,
                const Sequence < NumberedSortingInfo > &xSortInfo,
                const Reference< XAnyCompareFactory > &xCompFactory )
{
    Reference < XResultSetMetaDataSupplier > xMeta ( aResult, UNO_QUERY );

    if ( ! xMeta.is() )
    {
        OSL_ENSURE( sal_False, "No MetaData, No Sorting!" );
        return;
    }

    Reference < XResultSetMetaData > xData = xMeta->getMetaData();
    const NumberedSortingInfo *pSortInfo = xSortInfo.getConstArray();

    sal_Int32   nColumn;
    OUString    aPropName;
    SortInfo    *pInfo;

    for ( long i=xSortInfo.getLength(); i > 0; )
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
            pInfo->mbUseOwnCompare = FALSE;
            pInfo->mnType = 0;
        }
        else
        {
            pInfo->mbUseOwnCompare = TRUE;
            pInfo->mnType = xData->getColumnType( nColumn );
        }

        pInfo->mnColumn = nColumn;
        pInfo->mbAscending = pSortInfo[ i ].Ascending;
        pInfo->mbCaseSensitive = xData->isCaseSensitive( nColumn );
        pInfo->mpNext = mpSortInfo;
        mpSortInfo = pInfo;
    }
}

//-------------------------------------------------------------------------
void SortedResultSet::SetChanged( long nPos, long nCount )
{
    for ( long i=0; i<nCount; i++ )
    {
        long nSortPos = (long) maO2S.GetObject( nPos );
        if ( nSortPos < mnLastSort )
        {
            SortListData *pData = maS2O.GetData( nSortPos );
            if ( ! pData->mbModified )
            {
                pData->mbModified = TRUE;
                maModList.Insert( pData, LIST_APPEND );
            }
        }
        nPos += 1;
    }
}

//-------------------------------------------------------------------------
void SortedResultSet::ResortModified( EventList* pList )
{
    ULONG i, j;
    long nCompare, nCurPos, nNewPos;
    long nStart, nEnd, nOffset, nVal;
    SortListData *pData;
    ListAction *pAction;

    for ( i=0; i<maModList.Count(); i++ )
    {
        pData = (SortListData*) maModList.GetObject( i );
        nCompare = CompareImpl( mxOther, mxOriginal,
                                pData->mnOldPos, pData->mnCurPos );
        pData->mbModified = FALSE;
        if ( nCompare != 0 )
        {
            nCurPos = (long) maO2S.GetObject( (ULONG) pData->mnCurPos );
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
                maS2O.Remove( (ULONG) nCurPos );
                maS2O.Insert( pData, nNewPos );
                for ( j=1; j<maO2S.Count(); j++ )
                {
                    nVal = (long) maO2S.GetObject( (ULONG)( j ) );
                    if ( ( nStart <= nVal ) && ( nVal <= nEnd ) )
                    {
                        nVal += nOffset;
                        maO2S.Replace( (void*) (nVal), (ULONG)( j ) );
                    }
                }

                maO2S.Replace( (void*) nNewPos, (ULONG) pData->mnCurPos );

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
    maModList.Clear();
}

//-------------------------------------------------------------------------
void SortedResultSet::ResortNew( EventList* pList )
{
    long            i, j, nNewPos, nVal;
    SortListData    *pData;

    for ( i = mnLastSort; i<maS2O.Count(); i++ )
    {
        pData = (SortListData*) maModList.GetObject( i );
        nNewPos = FindPos( pData, 1, mnLastSort );
        if ( nNewPos != i )
        {
            maS2O.Remove( (ULONG) i );
            maS2O.Insert( pData, nNewPos );
            // maO2S liste korigieren
            for ( j=1; j<maO2S.Count(); j++ )
            {
                nVal = (long) maO2S.GetObject( (ULONG)( j ) );
                if ( nVal >= nNewPos )
                    maO2S.Replace( (void*) (nVal+1), (ULONG)( j ) );
            }
            maO2S.Replace( (void*) nNewPos, (ULONG) pData->mnCurPos );
        }
        mnLastSort++;
        pList->AddEvent( ListActionType::INSERTED, nNewPos, 1 );
    }
}

//-------------------------------------------------------------------------
//
// SortListData
//
//-------------------------------------------------------------------------
SortListData::SortListData( long nPos, BOOL bModified )
{
    mbModified = bModified;
    mnCurPos = nPos;
    mnOldPos = nPos;
};


//=========================================================================
SortedEntryList::SortedEntryList()
    : List( 64, 128 )
{}

//-------------------------------------------------------------------------
SortedEntryList::~SortedEntryList()
{
    Clear();
}

//-------------------------------------------------------------------------
void SortedEntryList::Clear()
{
    SortListData *pData = (SortListData*) List::First();

    while ( pData )
    {
        delete pData;
        pData = (SortListData*) List::Next();
    }

    List::Clear();
}

//-------------------------------------------------------------------------
void SortedEntryList::Insert( SortListData *pEntry, long nPos )
{
    List::Insert( pEntry, (ULONG) nPos );
}

//-------------------------------------------------------------------------
SortListData* SortedEntryList::Remove( long nPos )
{
    SortListData *pData = (SortListData*) List::Remove( nPos );
    return pData;
}

//-------------------------------------------------------------------------
SortListData* SortedEntryList::GetData( long nPos )
{
    SortListData *pData = (SortListData*) List::GetObject( nPos );
    return pData;
}

//-------------------------------------------------------------------------
long SortedEntryList::operator [] ( long nPos ) const
{
    SortListData *pData = (SortListData*) List::GetObject( nPos );
    if ( pData )
        if ( ! pData->mbModified )
            return pData->mnCurPos;
        else
        {
            OSL_ENSURE( sal_False, "SortedEntryList: Can't get value for modified entry!");
            return 0;
        }
    else
    {
        OSL_ENSURE( sal_False, "SortedEntryList: invalid pos!");
        return 0;
    }
}

//-------------------------------------------------------------------------
//
// class SRSPropertySetInfo.
//
//-------------------------------------------------------------------------

SRSPropertySetInfo::SRSPropertySetInfo()
{
    maProps[0].Name = OUString::createFromAscii( "RowCount" );
    maProps[0].Handle = -1;
    maProps[0].Type = ::getCppuType( (const OUString*) NULL );
    maProps[0].Attributes = -1;

    maProps[1].Name = OUString::createFromAscii( "IsRowCountFinal" );
    maProps[1].Handle = -1;
    maProps[1].Type = ::getBooleanCppuType();
    maProps[1].Attributes = -1;
}

//-------------------------------------------------------------------------
SRSPropertySetInfo::~SRSPropertySetInfo()
{}

//-------------------------------------------------------------------------
// XInterface methods.
//-------------------------------------------------------------------------

XINTERFACE_IMPL_2( SRSPropertySetInfo,
                   XTypeProvider,
                   XPropertySetInfo );

//-------------------------------------------------------------------------
// XTypeProvider methods.
//-------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_2( SRSPropertySetInfo,
                          XTypeProvider,
                          XPropertySetInfo );

//-------------------------------------------------------------------------
// XPropertySetInfo methods.
//-------------------------------------------------------------------------
Sequence< Property > SAL_CALL
SRSPropertySetInfo::getProperties() throw( RuntimeException )
{
    return Sequence < Property > ( maProps, 2 );
}

//-------------------------------------------------------------------------
Property SAL_CALL
SRSPropertySetInfo::getPropertyByName( const OUString& Name )
    throw( UnknownPropertyException, RuntimeException )
{
    if ( Name.compareToAscii( "RowCount" ) == 0 )
        return maProps[0];
    else if ( Name.compareToAscii( "IsRowCountFinal" ) == 0 )
        return maProps[1];
    else
        throw UnknownPropertyException();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL
SRSPropertySetInfo::hasPropertyByName( const OUString& Name )
    throw( RuntimeException )
{
    if ( Name.compareToAscii( "RowCount" ) == 0 )
        return TRUE;
    else if ( Name.compareToAscii( "IsRowCountFinal" ) == 0 )
        return TRUE;
    else
        return FALSE;
}

