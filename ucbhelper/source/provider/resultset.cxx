/*************************************************************************
 *
 *  $RCSfile: resultset.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-06 14:03:48 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef __HASH_MAP__
#include <stl/hash_map>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _UCBHELPER_RESULTSET_HXX
#include <ucbhelper/resultset.hxx>
#endif
#ifndef _UCBHELPER_RESULTSETMETADATA_HXX
#include <ucbhelper/resultsetmetadata.hxx>
#endif

using namespace cppu;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace rtl;
using namespace ucb;

//=========================================================================

namespace ucb_impl
{

struct PropertyInfo
{
    const char* pName;
    sal_Int32   nHandle;
    sal_Int16   nAttributes;
    const com::sun::star::uno::Type& (*pGetCppuType)();
};

static const com::sun::star::uno::Type& sal_uInt32_getCppuType()
{
    // ! uInt -> Int, because of Java !!!
    return getCppuType( static_cast< const sal_Int32 * >( 0 ) );
}

static const com::sun::star::uno::Type& sal_Bool_getCppuType()
{
    return getCppuBooleanType();
}

static PropertyInfo aPropertyTable[] =
{
    { "IsRowCountFinal",
      1000,
      PropertyAttribute::BOUND | PropertyAttribute::READONLY,
      &sal_Bool_getCppuType
    },
    { "RowCount",
      1001,
      PropertyAttribute::BOUND | PropertyAttribute::READONLY,
      &sal_uInt32_getCppuType
    },
    { 0,
      0,
      0,
      0
    }
};

#define RESULTSET_PROPERTY_COUNT 2

//=========================================================================
//
// class PropertySetInfo
//
//=========================================================================

class PropertySetInfo :
        public OWeakObject, public XTypeProvider, public XPropertySetInfo
{
    Reference< XMultiServiceFactory > m_xSMgr;
    Sequence< Property >*             m_pProps;

private:
    sal_Bool queryProperty( const rtl::OUString& aName, Property& rProp );

public:
    PropertySetInfo( const Reference< XMultiServiceFactory >& rxSMgr,
                     const PropertyInfo* pProps,
                     sal_uInt32 nProps );
    virtual ~PropertySetInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties()
        throw( RuntimeException );
    virtual Property SAL_CALL getPropertyByName( const rtl::OUString& aName )
        throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName( const rtl::OUString& Name )
        throw( RuntimeException );
};

//=========================================================================
//
// PropertyChangeListenerContainer.
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
> PropertyChangeListenerContainer;

//=========================================================================
//
// class PropertyChangeListeners.
//
//=========================================================================

class PropertyChangeListeners : public PropertyChangeListenerContainer
{
public:
    PropertyChangeListeners( osl::Mutex& rMutex )
    : PropertyChangeListenerContainer( rMutex ) {}
};

} // namespace ucb_impl

using namespace ucb_impl;

namespace ucb
{

//=========================================================================
//
// struct ResultSet_Impl.
//
//=========================================================================

struct ResultSet_Impl
{
    Reference< XMultiServiceFactory >   m_xSMgr;
    Reference< XCommandEnvironment >    m_xEnv;
    Reference< XPropertySetInfo >       m_xPropSetInfo;
    Reference< XResultSetMetaData >     m_xMetaData;
    Sequence< Property >                m_aProperties;
    vos::ORef< ResultSetDataSupplier >  m_xDataSupplier;
    osl::Mutex                          m_aMutex;
    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    PropertyChangeListeners*            m_pPropertyChangeListeners;
    sal_Int32                           m_nPos;
    sal_Bool                            m_bWasNull;
    sal_Bool                            m_bAfterLast;

    inline ResultSet_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                           const Sequence< Property >& rProperties,
                           const vos::ORef<
                                       ResultSetDataSupplier >& rDataSupplier,
                           const Reference< XCommandEnvironment >& rxEnv );
    inline ~ResultSet_Impl();
};

} // namespace ucb

inline ResultSet_Impl::ResultSet_Impl(
                    const Reference< XMultiServiceFactory >& rxSMgr,
                    const Sequence< Property >& rProperties,
                    const vos::ORef< ResultSetDataSupplier >& rDataSupplier,
                    const Reference< XCommandEnvironment >& rxEnv )
: m_xSMgr( rxSMgr ),
  m_xEnv( rxEnv ),
  m_aProperties( rProperties ),
  m_xDataSupplier( rDataSupplier ),
  m_pDisposeEventListeners( 0 ),
  m_pPropertyChangeListeners( 0 ),
  m_nPos( 0 ), // Position is one-based. Zero means: before first element.
  m_bWasNull( sal_False ),
  m_bAfterLast( sal_False )
{
}

//=========================================================================
inline ResultSet_Impl::~ResultSet_Impl()
{
    delete m_pDisposeEventListeners;
    delete m_pPropertyChangeListeners;
}

//=========================================================================
//=========================================================================
//
// ResultSet Implementation.
//
//=========================================================================
//=========================================================================

ResultSet::ResultSet( const Reference< XMultiServiceFactory >& rxSMgr,
                      const Sequence< Property >& rProperties,
                      const vos::ORef< ResultSetDataSupplier >& rDataSupplier )
: m_pImpl( new ResultSet_Impl( rxSMgr,
                               rProperties,
                               rDataSupplier,
                               Reference< XCommandEnvironment >() ) )
{
    rDataSupplier->m_pResultSet = this;
}

//=========================================================================
ResultSet::ResultSet( const Reference< XMultiServiceFactory >& rxSMgr,
                      const Sequence< Property >& rProperties,
                      const vos::ORef< ResultSetDataSupplier >& rDataSupplier,
                      const Reference< XCommandEnvironment >& rxEnv )
: m_pImpl( new ResultSet_Impl( rxSMgr, rProperties, rDataSupplier, rxEnv ) )
{
    rDataSupplier->m_pResultSet = this;
}

//=========================================================================
// virtual
ResultSet::~ResultSet()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_9( ResultSet,
                   XTypeProvider,
                   XServiceInfo,
                   XComponent,
                   XContentAccess,
                   XResultSet,
                   XResultSetMetaDataSupplier,
                   XRow,
                   XCloseable,
                   XPropertySet );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_9( ResultSet,
                      XTypeProvider,
                         XServiceInfo,
                      XComponent,
                      XContentAccess,
                      XResultSet,
                      XResultSetMetaDataSupplier,
                      XRow,
                      XCloseable,
                      XPropertySet );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( ResultSet,
                    OUString::createFromAscii( "ResultSet" ),
                    OUString::createFromAscii( RESULTSET_SERVICE_NAME ) );

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
void SAL_CALL ResultSet::dispose()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners &&
         m_pImpl->m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        m_pImpl->m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }

    m_pImpl->m_xDataSupplier->close();
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_pImpl->m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XResultSetMetaDataSupplier methods.
//
//=========================================================================

// virtual
Reference< XResultSetMetaData > SAL_CALL ResultSet::getMetaData()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xMetaData.is() )
        m_pImpl->m_xMetaData = new ResultSetMetaData( m_pImpl->m_xSMgr,
                                                      m_pImpl->m_aProperties );

    return m_pImpl->m_xMetaData;
}

//=========================================================================
//
// XResultSet methods.
//
//=========================================================================

// virtual
sal_Bool SAL_CALL ResultSet::next()
    throw( SQLException, RuntimeException )
{
    // Note: Cursor is initially positioned before the first row.
    //       First call to 'next()' moves it to first row.

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    // getResult works zero-based!
    if ( !m_pImpl->m_xDataSupplier->getResult( m_pImpl->m_nPos ) )
    {
        m_pImpl->m_bAfterLast = sal_True;
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_nPos++;
    m_pImpl->m_xDataSupplier->validate();
    return sal_True;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::isBeforeFirst()
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    // getResult works zero-based!
    if ( !m_pImpl->m_xDataSupplier->getResult( 0 ) )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == 0 );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::isAfterLast()
    throw( SQLException, RuntimeException )
{
    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_bAfterLast;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::isFirst()
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == 1 );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::isLast()
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( !nCount )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == nCount );
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::beforeFirst()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = sal_False;
    m_pImpl->m_nPos = 0;
    m_pImpl->m_xDataSupplier->validate();
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::afterLast()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = sal_True;
    m_pImpl->m_xDataSupplier->validate();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::first()
    throw( SQLException, RuntimeException )
{
    // getResult works zero-based!
    if ( m_pImpl->m_xDataSupplier->getResult( 0 ) )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = sal_False;
        m_pImpl->m_nPos = 1;
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }

    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::last()
    throw( SQLException, RuntimeException )
{
    sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( nCount )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = sal_False;
        m_pImpl->m_nPos = nCount;
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }

    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSet::getRow()
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return 0;
    }

    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_nPos;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::absolute( sal_Int32 row )
    throw( SQLException, RuntimeException )
{
/*
    If the row number is positive, the cursor moves to the given row number
    with respect to the beginning of the result set. The first row is row 1,
    the second is row 2, and so on.

    If the given row number is negative, the cursor moves to an absolute row
    position with respect to the end of the result set. For example, calling
    absolaute( -1 ) positions the cursor on the last row, absolaute( -2 )
    indicates the next-to-last row, and so on.

    An attempt to position the cursor beyond the first/last row in the result
    set leaves the cursor before/after the first/last row, respectively.

    Calling absolute( 1 ) is the same as calling first().

    Calling absolute( -1 ) is the same as calling last().
*/
    if ( row < 0 )
    {
        sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();

        if ( ( row * -1 ) > nCount )
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = 0;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
        else // |row| <= nCount
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = ( nCount + row + 1 );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
    }
    else if ( row == 0 )
    {
        // @throws SQLException
        //      ... if row is 0 ...
        throw SQLException();
    }
    else // row > 0
    {
        sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();

        if ( row <= nCount )
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = row;
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else // row > nCount
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_True;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
    }

    // unreachable...
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::relative( sal_Int32 rows )
    throw( SQLException, RuntimeException )
{
/*
    Attempting to move beyond the first/last row in the result set
    positions the cursor before/after the the first/last row.

    Calling relative( 0 ) is valid, but does not change the cursor position.

    Calling relative( 1 ) is different from calling next() because it makes
    sense to call next() when there is no current row, for example, when
    the cursor is positioned before the first row or after the last row of
    the result set.
*/
    if ( m_pImpl->m_bAfterLast || ( m_pImpl->m_nPos == 0 ) )
    {
        // "No current row".
        throw SQLException();
    }

    if ( rows < 0 )
    {
        if ( ( m_pImpl->m_nPos + rows ) > 0 )
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = 0;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
    }
    else if ( rows == 0 )
    {
        // nop.
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }
    else // rows > 0
    {
        sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();
        if ( ( m_pImpl->m_nPos + rows ) <= nCount )
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_False;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else
        {
            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = sal_True;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
    }

    // unreachable...
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::previous()
    throw( SQLException, RuntimeException )
{
/*
    previous() is not the same as relative( -1 ) because it makes sense
    to call previous() when there is no current row.
*/
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_bAfterLast = sal_False;
        sal_uInt32 nCount = m_pImpl->m_xDataSupplier->totalCount();
        m_pImpl->m_nPos = nCount;
    }
    else if ( m_pImpl->m_nPos )
        m_pImpl->m_nPos--;

    if ( m_pImpl->m_nPos )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }

    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::refreshRow()
    throw( SQLException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    if ( m_pImpl->m_bAfterLast || ( m_pImpl->m_nPos == 0 ) )
        return;

    m_pImpl->m_xDataSupplier->releasePropertyValues( m_pImpl->m_nPos );
    m_pImpl->m_xDataSupplier->validate();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::rowUpdated()
    throw( SQLException, RuntimeException )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::rowInserted()
    throw( SQLException, RuntimeException )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::rowDeleted()
    throw( SQLException, RuntimeException )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
Reference< XInterface > SAL_CALL ResultSet::getStatement()
    throw( SQLException, RuntimeException )
{
/*
    returns the Statement that produced this ResultSet object. If the
    result set was generated some other way, ... this method returns null.
*/
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XInterface >();
}

//=========================================================================
//
// XRow methods.
//
//=========================================================================

// virtual
sal_Bool SAL_CALL ResultSet::wasNull()
    throw( SQLException, RuntimeException )
{
    // This method can not be implemented correctly!!! Imagine different
    // threads doing a getXYZ - wasNull calling sequence on the same
    // implementation object...

    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_xDataSupplier->validate();
            return xValues->wasNull();
        }
    }

    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_bWasNull;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSet::getString( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getString( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return OUString();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSet::getBoolean( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBoolean( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}

//=========================================================================
// virtual
sal_Int8 SAL_CALL ResultSet::getByte( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getByte( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
sal_Int16 SAL_CALL ResultSet::getShort( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getShort( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSet::getInt( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getInt( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
sal_Int64 SAL_CALL ResultSet::getLong( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getLong( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
float SAL_CALL ResultSet::getFloat( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getFloat( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
double SAL_CALL ResultSet::getDouble( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getDouble( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}

//=========================================================================
// virtual
Sequence< sal_Int8 > SAL_CALL
ResultSet::getBytes( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBytes( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Sequence< sal_Int8 >();
}

//=========================================================================
// virtual
Date SAL_CALL ResultSet::getDate( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getDate( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Date();
}

//=========================================================================
// virtual
Time SAL_CALL ResultSet::getTime( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getTime( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Time();
}

//=========================================================================
// virtual
com::sun::star::util::DateTime SAL_CALL
ResultSet::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getTimestamp( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return com::sun::star::util::DateTime();
}

//=========================================================================
// virtual
Reference< XInputStream > SAL_CALL
ResultSet::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBinaryStream( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XInputStream >();
}

//=========================================================================
// virtual
Reference< XInputStream > SAL_CALL
ResultSet::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getCharacterStream( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XInputStream >();
}

//=========================================================================
// virtual
Any SAL_CALL ResultSet::getObject( sal_Int32 columnIndex,
                                       const Reference< XNameAccess >& typeMap )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getObject( columnIndex, typeMap );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Any();
}

//=========================================================================
// virtual
Reference< XRef > SAL_CALL ResultSet::getRef( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getRef( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XRef >();
}

//=========================================================================
// virtual
Reference< XBlob > SAL_CALL ResultSet::getBlob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBlob( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XBlob >();
}

//=========================================================================
// virtual
Reference< XClob > SAL_CALL ResultSet::getClob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getClob( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XClob >();
}

//=========================================================================
// virtual
Reference< XArray > SAL_CALL ResultSet::getArray( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        Reference< XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = sal_False;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getArray( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = sal_True;
    m_pImpl->m_xDataSupplier->validate();
    return Reference< XArray >();
}

//=========================================================================
//
// XCloseable methods.
//
//=========================================================================

// virtual
void SAL_CALL ResultSet::close()
    throw( SQLException, RuntimeException )
{
    m_pImpl->m_xDataSupplier->close();
    m_pImpl->m_xDataSupplier->validate();
}

//=========================================================================
//
// XContentAccess methods.
//
//=========================================================================

// virtual
OUString SAL_CALL ResultSet::queryContentIdentifierString()
    throw( RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifierString(
                                                        m_pImpl->m_nPos - 1 );

    return OUString();
}

//=========================================================================
// virtual
Reference< XContentIdentifier > SAL_CALL ResultSet::queryContentIdentifier()
    throw( RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifier(
                                                        m_pImpl->m_nPos - 1 );

    return Reference< XContentIdentifier >();
}

//=========================================================================
// virtual
Reference< XContent > SAL_CALL ResultSet::queryContent()
    throw( RuntimeException )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContent( m_pImpl->m_nPos - 1 );

    return Reference< XContent >();
}

//=========================================================================
//
// XPropertySet methods.
//
//=========================================================================

// virtual
Reference< XPropertySetInfo > SAL_CALL ResultSet::getPropertySetInfo()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.is() )
        m_pImpl->m_xPropSetInfo
            = new PropertySetInfo( m_pImpl->m_xSMgr,
                                   aPropertyTable,
                                   RESULTSET_PROPERTY_COUNT );
    return m_pImpl->m_xPropSetInfo;
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::setPropertyValue( const OUString& aPropertyName,
                                           const Any& aValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    if ( !aPropertyName.getLength() )
        throw UnknownPropertyException();

    if ( aPropertyName.equals(
                OUString::createFromAscii( "RowCount" ) ) )
    {
        // property is read-only.
        throw IllegalArgumentException();
    }
    else if ( aPropertyName.equals(
                OUString::createFromAscii( "IsRowCountFinal" ) ) )
    {
        // property is read-only.
        throw IllegalArgumentException();
    }
    else
    {
        throw UnknownPropertyException();
    }
}

//=========================================================================
// virtual
Any SAL_CALL ResultSet::getPropertyValue( const OUString& PropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    if ( !PropertyName.getLength() )
        throw UnknownPropertyException();

    Any aValue;

    if ( PropertyName.equals(
                OUString::createFromAscii( "RowCount" ) ) )
    {
        aValue <<= m_pImpl->m_xDataSupplier->currentCount();
    }
    else if ( PropertyName.equals(
                OUString::createFromAscii( "IsRowCountFinal" ) ) )
    {
        aValue <<= m_pImpl->m_xDataSupplier->isCountFinal();
    }
    else
    {
        throw UnknownPropertyException();
    }

    return aValue;
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::addPropertyChangeListener(
                        const OUString& aPropertyName,
                           const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    // Note: An empty property name means a listener for "all" properties.

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( aPropertyName.getLength() &&
         !aPropertyName.equals(
                OUString::createFromAscii( "RowCount" ) ) &&
         !aPropertyName.equals(
                OUString::createFromAscii( "IsRowCountFinal" ) ) )
        throw UnknownPropertyException();

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners
            = new PropertyChangeListeners( m_pImpl->m_aMutex );

    m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                aPropertyName, xListener );
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::removePropertyChangeListener(
                        const OUString& aPropertyName,
                        const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( aPropertyName.getLength() &&
         !aPropertyName.equals(
                OUString::createFromAscii( "RowCount" ) ) &&
         !aPropertyName.equals(
                OUString::createFromAscii( "IsRowCountFinal" ) ) )
        throw UnknownPropertyException();

    if ( m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                    aPropertyName, xListener );

}

//=========================================================================
// virtual
void SAL_CALL ResultSet::addVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    //  No constrained props. at the moment.
}

//=========================================================================
// virtual
void SAL_CALL ResultSet::removeVetoableChangeListener(
                        const OUString& PropertyName,
                        const Reference< XVetoableChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    //  No constrained props. at the moment.
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void ResultSet::propertyChanged( const PropertyChangeEvent& rEvt )
{
//  osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper* pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer(
                                                        rEvt.PropertyName );
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
    pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer( OUString() );
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

//=========================================================================
void ResultSet::rowCountChanged( sal_uInt32 nOld, sal_uInt32 nNew )
{
    VOS_ENSURE( nOld < nNew, "ResultSet::rowCountChanged - nOld >= nNew!" );

//  osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        PropertyChangeEvent( static_cast< OWeakObject * >( this ),
                              OUString::createFromAscii( "RowCount" ),
                              sal_False,
                              1001,
                              makeAny( nOld ),    // old value
                              makeAny( nNew ) ) ); // new value
}

//=========================================================================
void ResultSet::rowCountFinal()
{
//  osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        PropertyChangeEvent( static_cast< OWeakObject * >( this ),
                              OUString::createFromAscii( "IsRowCountFinal" ),
                              sal_False,
                              1000,
                              makeAny( sal_False ),   // old value
                              makeAny( sal_True ) ) ); // new value
}

//=========================================================================
const Sequence< Property >& ResultSet::getProperties()
{
    return m_pImpl->m_aProperties;
}

//=========================================================================
const Reference< XCommandEnvironment >& ResultSet::getEnvironment()
{
    return m_pImpl->m_xEnv;
}

//=========================================================================
//=========================================================================
//
// PropertySetInfo Implementation.
//
//=========================================================================
//=========================================================================

PropertySetInfo::PropertySetInfo(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const PropertyInfo* pProps,
                        sal_uInt32 nProps )
: m_xSMgr( rxSMgr )
{
    m_pProps = new Sequence< Property >( nProps );

    if ( nProps )
    {
        const PropertyInfo* pEntry = pProps;
        Property* pProperties = m_pProps->getArray();

        for ( sal_uInt32 n = 0; n < nProps; ++n )
        {
            Property& rProp = pProperties[ n ];

            rProp.Name       = rtl::OUString::createFromAscii( pEntry->pName );
            rProp.Handle     = pEntry->nHandle;
            rProp.Type       = pEntry->pGetCppuType();
            rProp.Attributes = pEntry->nAttributes;

            pEntry++;
        }
    }
}

//=========================================================================
// virtual
PropertySetInfo::~PropertySetInfo()
{
    delete m_pProps;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( PropertySetInfo,
                   XTypeProvider,
                   XPropertySetInfo );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( PropertySetInfo,
                         XTypeProvider,
                         XPropertySetInfo );

//=========================================================================
//
// XPropertySetInfo methods.
//
//=========================================================================

// virtual
Sequence< Property > SAL_CALL PropertySetInfo::getProperties()
    throw( RuntimeException )
{
    return Sequence< Property >( *m_pProps );
}

//=========================================================================
// virtual
Property SAL_CALL PropertySetInfo::getPropertyByName( const OUString& aName )
    throw( UnknownPropertyException, RuntimeException )
{
    Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const OUString& Name )
    throw( RuntimeException )
{
    Property aProp;
    return queryProperty( Name, aProp );
}

//=========================================================================
sal_Bool PropertySetInfo::queryProperty(
                                const OUString& aName, Property& rProp )
{
    sal_uInt32 nCount = m_pProps->getLength();
    const Property* pProps = m_pProps->getConstArray();
    for ( sal_uInt32 n = 0; n < nCount; ++n )
    {
        const Property& rCurr = pProps[ n ];
        if ( rCurr.Name == aName )
        {
            rProp = rCurr;
            return sal_True;
        }
    }

    return sal_False;
}

