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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <ucbhelper/getcomponentcontext.hxx>
#include <ucbhelper/resultset.hxx>
#include <ucbhelper/resultsetmetadata.hxx>

using namespace com::sun::star;



namespace ucbhelper_impl
{

struct PropertyInfo
{
    const char* pName;
    sal_Int32   nHandle;
    sal_Int16   nAttributes;
    const uno::Type& (*pGetCppuType)();
};

static const uno::Type& sal_Int32_getCppuType()
{
    return cppu::UnoType<sal_Int32>::get();
}

static const uno::Type& sal_Bool_getCppuType()
{
    return cppu::UnoType<bool>::get();
}

static const PropertyInfo aPropertyTable[] =
{
    { "IsRowCountFinal",
      1000,
      beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY,
      &sal_Bool_getCppuType
    },
    { "RowCount",
      1001,
      beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY,
      &sal_Int32_getCppuType
    },
    { nullptr,
      0,
      0,
      nullptr
    }
};

#define RESULTSET_PROPERTY_COUNT 2



// class PropertySetInfo



class PropertySetInfo :
        public cppu::OWeakObject,
        public lang::XTypeProvider,
        public beans::XPropertySetInfo
{
    uno::Sequence< beans::Property >*            m_pProps;

private:
    bool queryProperty(
        const OUString& aName, beans::Property& rProp );

public:
    PropertySetInfo(
        const PropertyInfo* pProps,
        sal_Int32 nProps );
    virtual ~PropertySetInfo();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertySetInfo
    virtual uno::Sequence< beans::Property > SAL_CALL getProperties()
        throw( uno::RuntimeException, std::exception ) override;
    virtual beans::Property SAL_CALL getPropertyByName(
            const OUString& aName )
        throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw( uno::RuntimeException, std::exception ) override;
};

typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
    PropertyChangeListenerContainer;

class PropertyChangeListeners : public PropertyChangeListenerContainer
{
public:
    explicit PropertyChangeListeners( osl::Mutex& rMtx )
        : PropertyChangeListenerContainer( rMtx ) {}
};

} // namespace ucbhelper_impl

using namespace ucbhelper_impl;

namespace ucbhelper
{



// struct ResultSet_Impl.



struct ResultSet_Impl
{
    uno::Reference< uno::XComponentContext >        m_xContext;
    uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;
    uno::Reference< beans::XPropertySetInfo >       m_xPropSetInfo;
    uno::Reference< sdbc::XResultSetMetaData >      m_xMetaData;
    uno::Sequence< beans::Property >                m_aProperties;
    rtl::Reference< ResultSetDataSupplier >         m_xDataSupplier;
    osl::Mutex                          m_aMutex;
    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    PropertyChangeListeners*            m_pPropertyChangeListeners;
    sal_Int32                           m_nPos;
    bool                            m_bWasNull;
    bool                            m_bAfterLast;

    inline ResultSet_Impl(
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Sequence< beans::Property >& rProperties,
        const rtl::Reference< ResultSetDataSupplier >& rDataSupplier,
        const uno::Reference< css::ucb::XCommandEnvironment >& rxEnv );
    inline ~ResultSet_Impl();
};

inline ResultSet_Impl::ResultSet_Impl(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Sequence< beans::Property >& rProperties,
    const rtl::Reference< ResultSetDataSupplier >& rDataSupplier,
    const uno::Reference< css::ucb::XCommandEnvironment >& rxEnv )
: m_xContext( rxContext ),
  m_xEnv( rxEnv ),
  m_aProperties( rProperties ),
  m_xDataSupplier( rDataSupplier ),
  m_pDisposeEventListeners( nullptr ),
  m_pPropertyChangeListeners( nullptr ),
  m_nPos( 0 ), // Position is one-based. Zero means: before first element.
  m_bWasNull( false ),
  m_bAfterLast( false )
{
}


inline ResultSet_Impl::~ResultSet_Impl()
{
    delete m_pDisposeEventListeners;
    delete m_pPropertyChangeListeners;
}




// ResultSet Implementation.




ResultSet::ResultSet(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Sequence< beans::Property >& rProperties,
    const rtl::Reference< ResultSetDataSupplier >& rDataSupplier )
: m_pImpl( new ResultSet_Impl(
               rxContext,
               rProperties,
               rDataSupplier,
               uno::Reference< css::ucb::XCommandEnvironment >() ) )
{
    rDataSupplier->m_pResultSet = this;
}


ResultSet::ResultSet(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Sequence< beans::Property >& rProperties,
    const rtl::Reference< ResultSetDataSupplier >& rDataSupplier,
    const uno::Reference< css::ucb::XCommandEnvironment >& rxEnv )
: m_pImpl( new ResultSet_Impl( rxContext, rProperties, rDataSupplier, rxEnv ) )
{
    rDataSupplier->m_pResultSet = this;
}


// virtual
ResultSet::~ResultSet()
{
}



// XInterface methods.

void SAL_CALL ResultSet::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ResultSet::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ResultSet::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< lang::XComponent* >(this)),
                                               (static_cast< css::ucb::XContentAccess* >(this)),
                                               (static_cast< sdbc::XResultSet* >(this)),
                                               (static_cast< sdbc::XResultSetMetaDataSupplier* >(this)),
                                               (static_cast< sdbc::XRow* >(this)),
                                               (static_cast< sdbc::XCloseable* >(this)),
                                               (static_cast< beans::XPropertySet* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.



XTYPEPROVIDER_IMPL_9( ResultSet,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      lang::XComponent,
                      css::ucb::XContentAccess,
                      sdbc::XResultSet,
                      sdbc::XResultSetMetaDataSupplier,
                      sdbc::XRow,
                      sdbc::XCloseable,
                      beans::XPropertySet );



// XServiceInfo methods.



XSERVICEINFO_NOFACTORY_IMPL_1( ResultSet,
                    OUString("ResultSet"),
                    RESULTSET_SERVICE_NAME );



// XComponent methods.



// virtual
void SAL_CALL ResultSet::dispose()
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners &&
         m_pImpl->m_pDisposeEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pImpl->m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< beans::XPropertySet * >( this );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }

    m_pImpl->m_xDataSupplier->close();
}


// virtual
void SAL_CALL ResultSet::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_pImpl->m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL ResultSet::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );
}



// XResultSetMetaDataSupplier methods.



// virtual
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL ResultSet::getMetaData()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xMetaData.is() )
        m_pImpl->m_xMetaData = new ResultSetMetaData( m_pImpl->m_xContext,
                                                      m_pImpl->m_aProperties );

    return m_pImpl->m_xMetaData;
}



// XResultSet methods.



// virtual
sal_Bool SAL_CALL ResultSet::next()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    // Note: Cursor is initially positioned before the first row.
    //       First call to 'next()' moves it to first row.

    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    // getResult works zero-based!
    if ( !m_pImpl->m_xDataSupplier->getResult( m_pImpl->m_nPos ) )
    {
        m_pImpl->m_bAfterLast = true;
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_nPos++;
    m_pImpl->m_xDataSupplier->validate();
    return sal_True;
}


// virtual
sal_Bool SAL_CALL ResultSet::isBeforeFirst()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
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


// virtual
sal_Bool SAL_CALL ResultSet::isAfterLast()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_bAfterLast;
}


// virtual
sal_Bool SAL_CALL ResultSet::isFirst()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == 1 );
}


// virtual
sal_Bool SAL_CALL ResultSet::isLast()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( !nCount )
    {
        m_pImpl->m_xDataSupplier->validate();
        return sal_False;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == nCount );
}


// virtual
void SAL_CALL ResultSet::beforeFirst()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = false;
    m_pImpl->m_nPos = 0;
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
void SAL_CALL ResultSet::afterLast()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = true;
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
sal_Bool SAL_CALL ResultSet::first()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    // getResult works zero-based!
    if ( m_pImpl->m_xDataSupplier->getResult( 0 ) )
    {
        osl::MutexGuard aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = false;
        m_pImpl->m_nPos = 1;
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }

    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
sal_Bool SAL_CALL ResultSet::last()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( nCount )
    {
        osl::MutexGuard aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = false;
        m_pImpl->m_nPos = nCount;
        m_pImpl->m_xDataSupplier->validate();
        return sal_True;
    }

    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
sal_Int32 SAL_CALL ResultSet::getRow()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return 0;
    }

    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_nPos;
}


// virtual
sal_Bool SAL_CALL ResultSet::absolute( sal_Int32 row )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
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
        sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();

        if ( ( row * -1 ) > nCount )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = 0;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
        else // |row| <= nCount
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( nCount + row + 1 );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
    }
    else if ( row == 0 )
    {
        // @throws SQLException
        //      ... if row is 0 ...
        throw sdbc::SQLException();
    }
    else // row > 0
    {
        sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();

        if ( row <= nCount )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = row;
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else // row > nCount
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = true;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
    }

    // unreachable...
}


// virtual
sal_Bool SAL_CALL ResultSet::relative( sal_Int32 rows )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
/*
    Attempting to move beyond the first/last row in the result set
    positions the cursor before/after the first/last row.

    Calling relative( 0 ) is valid, but does not change the cursor position.

    Calling relative( 1 ) is different from calling next() because it makes
    sense to call next() when there is no current row, for example, when
    the cursor is positioned before the first row or after the last row of
    the result set.
*/
    if ( m_pImpl->m_bAfterLast || ( m_pImpl->m_nPos == 0 ) )
    {
        // "No current row".
        throw sdbc::SQLException();
    }

    if ( rows < 0 )
    {
        if ( ( m_pImpl->m_nPos + rows ) > 0 )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
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
        sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
        if ( ( m_pImpl->m_nPos + rows ) <= nCount )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return sal_True;
        }
        else
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = true;
            m_pImpl->m_xDataSupplier->validate();
            return sal_False;
        }
    }

    // unreachable...
}


// virtual
sal_Bool SAL_CALL ResultSet::previous()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
/*
    previous() is not the same as relative( -1 ) because it makes sense
    to call previous() when there is no current row.
*/
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_bAfterLast = false;
        sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
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


// virtual
void SAL_CALL ResultSet::refreshRow()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );
    if ( m_pImpl->m_bAfterLast || ( m_pImpl->m_nPos == 0 ) )
        return;

    m_pImpl->m_xDataSupplier->releasePropertyValues( m_pImpl->m_nPos );
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
sal_Bool SAL_CALL ResultSet::rowUpdated()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
sal_Bool SAL_CALL ResultSet::rowInserted()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
sal_Bool SAL_CALL ResultSet::rowDeleted()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
uno::Reference< uno::XInterface > SAL_CALL ResultSet::getStatement()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
/*
    returns the Statement that produced this ResultSet object. If the
    result set was generated some other way, ... this method returns null.
*/
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< uno::XInterface >();
}



// XRow methods.



// virtual
sal_Bool SAL_CALL ResultSet::wasNull()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    // This method can not be implemented correctly!!! Imagine different
    // threads doing a getXYZ - wasNull calling sequence on the same
    // implementation object...

    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
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


// virtual
OUString SAL_CALL ResultSet::getString( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getString( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return OUString();
}


// virtual
sal_Bool SAL_CALL ResultSet::getBoolean( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBoolean( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return sal_False;
}


// virtual
sal_Int8 SAL_CALL ResultSet::getByte( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getByte( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
sal_Int16 SAL_CALL ResultSet::getShort( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getShort( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
sal_Int32 SAL_CALL ResultSet::getInt( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getInt( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
sal_Int64 SAL_CALL ResultSet::getLong( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getLong( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
float SAL_CALL ResultSet::getFloat( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getFloat( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
double SAL_CALL ResultSet::getDouble( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getDouble( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return 0;
}


// virtual
uno::Sequence< sal_Int8 > SAL_CALL
ResultSet::getBytes( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBytes( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Sequence< sal_Int8 >();
}


// virtual
util::Date SAL_CALL ResultSet::getDate( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getDate( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return util::Date();
}


// virtual
util::Time SAL_CALL ResultSet::getTime( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getTime( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return util::Time();
}


// virtual
util::DateTime SAL_CALL
ResultSet::getTimestamp( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getTimestamp( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return util::DateTime();
}


// virtual
uno::Reference< io::XInputStream > SAL_CALL
ResultSet::getBinaryStream( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBinaryStream( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< io::XInputStream >();
}


// virtual
uno::Reference< io::XInputStream > SAL_CALL
ResultSet::getCharacterStream( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getCharacterStream( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< io::XInputStream >();
}


// virtual
uno::Any SAL_CALL ResultSet::getObject(
        sal_Int32 columnIndex,
        const uno::Reference< container::XNameAccess >& typeMap )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getObject( columnIndex, typeMap );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Any();
}


// virtual
uno::Reference< sdbc::XRef > SAL_CALL
ResultSet::getRef( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getRef( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< sdbc::XRef >();
}


// virtual
uno::Reference< sdbc::XBlob > SAL_CALL
ResultSet::getBlob( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getBlob( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< sdbc::XBlob >();
}


// virtual
uno::Reference< sdbc::XClob > SAL_CALL
ResultSet::getClob( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getClob( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< sdbc::XClob >();
}


// virtual
uno::Reference< sdbc::XArray > SAL_CALL
ResultSet::getArray( sal_Int32 columnIndex )
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
    {
        uno::Reference< sdbc::XRow > xValues
            = m_pImpl->m_xDataSupplier->queryPropertyValues(
                                                        m_pImpl->m_nPos - 1 );
        if ( xValues.is() )
        {
            m_pImpl->m_bWasNull = false;
            m_pImpl->m_xDataSupplier->validate();
            return xValues->getArray( columnIndex );
        }
    }

    m_pImpl->m_bWasNull = true;
    m_pImpl->m_xDataSupplier->validate();
    return uno::Reference< sdbc::XArray >();
}



// XCloseable methods.



// virtual
void SAL_CALL ResultSet::close()
    throw( sdbc::SQLException, uno::RuntimeException, std::exception )
{
    m_pImpl->m_xDataSupplier->close();
    m_pImpl->m_xDataSupplier->validate();
}



// XContentAccess methods.



// virtual
OUString SAL_CALL ResultSet::queryContentIdentifierString()
    throw( uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifierString(
                                                        m_pImpl->m_nPos - 1 );

    return OUString();
}


// virtual
uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
ResultSet::queryContentIdentifier()
    throw( uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifier(
                                                        m_pImpl->m_nPos - 1 );

    return uno::Reference< css::ucb::XContentIdentifier >();
}


// virtual
uno::Reference< css::ucb::XContent > SAL_CALL
ResultSet::queryContent()
    throw( uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContent( m_pImpl->m_nPos - 1 );

    return uno::Reference< css::ucb::XContent >();
}



// XPropertySet methods.



// virtual
uno::Reference< beans::XPropertySetInfo > SAL_CALL
ResultSet::getPropertySetInfo()
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.is() )
        m_pImpl->m_xPropSetInfo
            = new PropertySetInfo( aPropertyTable,
                                   RESULTSET_PROPERTY_COUNT );
    return m_pImpl->m_xPropSetInfo;
}


// virtual
void SAL_CALL ResultSet::setPropertyValue( const OUString& aPropertyName,
                                           const uno::Any& )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    if ( aPropertyName.isEmpty() )
        throw beans::UnknownPropertyException();

    if ( aPropertyName == "RowCount" )
    {
        // property is read-only.
        throw lang::IllegalArgumentException();
    }
    else if ( aPropertyName == "IsRowCountFinal" )
    {
        // property is read-only.
        throw lang::IllegalArgumentException();
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
}


// virtual
uno::Any SAL_CALL ResultSet::getPropertyValue(
        const OUString& PropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    if ( PropertyName.isEmpty() )
        throw beans::UnknownPropertyException();

    uno::Any aValue;

    if ( PropertyName == "RowCount" )
    {
        aValue <<= m_pImpl->m_xDataSupplier->currentCount();
    }
    else if ( PropertyName == "IsRowCountFinal" )
    {
        aValue <<= m_pImpl->m_xDataSupplier->isCountFinal();
    }
    else
    {
        throw beans::UnknownPropertyException();
    }

    return aValue;
}


// virtual
void SAL_CALL ResultSet::addPropertyChangeListener(
        const OUString& aPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    // Note: An empty property name means a listener for "all" properties.

    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( !aPropertyName.isEmpty() &&
         aPropertyName != "RowCount" &&
         aPropertyName != "IsRowCountFinal" )
        throw beans::UnknownPropertyException();

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners
            = new PropertyChangeListeners( m_pImpl->m_aMutex );

    m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                aPropertyName, xListener );
}


// virtual
void SAL_CALL ResultSet::removePropertyChangeListener(
        const OUString& aPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    if ( !aPropertyName.isEmpty() &&
         aPropertyName != "RowCount" &&
         aPropertyName != "IsRowCountFinal" )
        throw beans::UnknownPropertyException();

    if ( m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                    aPropertyName, xListener );

}


// virtual
void SAL_CALL ResultSet::addVetoableChangeListener(
        const OUString&,
        const uno::Reference< beans::XVetoableChangeListener >& )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    //  No constrained props, at the moment.
}


// virtual
void SAL_CALL ResultSet::removeVetoableChangeListener(
        const OUString&,
        const uno::Reference< beans::XVetoableChangeListener >& )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    //  No constrained props, at the moment.
}



// Non-interface methods.



void ResultSet::propertyChanged( const beans::PropertyChangeEvent& rEvt )
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    // Notify listeners interested especially in the changed property.
    cppu::OInterfaceContainerHelper* pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer(
                                                        rEvt.PropertyName );
    if ( pPropsContainer )
    {
        cppu::OInterfaceIteratorHelper aIter( *pPropsContainer );
        while ( aIter.hasMoreElements() )
        {
            uno::Reference< beans::XPropertyChangeListener > xListener(
                aIter.next(), uno::UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }

    // Notify listeners interested in all properties.
    pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer( OUString() );
    if ( pPropsContainer )
    {
        cppu::OInterfaceIteratorHelper aIter( *pPropsContainer );
        while ( aIter.hasMoreElements() )
        {
            uno::Reference< beans::XPropertyChangeListener > xListener(
                aIter.next(), uno::UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }
}


void ResultSet::rowCountChanged( sal_uInt32 nOld, sal_uInt32 nNew )
{
    OSL_ENSURE( nOld < nNew, "ResultSet::rowCountChanged - nOld >= nNew!" );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        beans::PropertyChangeEvent(
            static_cast< cppu::OWeakObject * >( this ),
            OUString("RowCount"),
            sal_False,
            1001,
            uno::makeAny( nOld ),     // old value
            uno::makeAny( nNew ) ) ); // new value
}


void ResultSet::rowCountFinal()
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        beans::PropertyChangeEvent(
            static_cast< cppu::OWeakObject * >( this ),
            OUString("IsRowCountFinal"),
            sal_False,
            1000,
            uno:: makeAny( sal_False ),   // old value
            uno::makeAny( sal_True ) ) ); // new value
}


const uno::Sequence< beans::Property >& ResultSet::getProperties()
{
    return m_pImpl->m_aProperties;
}


const uno::Reference< css::ucb::XCommandEnvironment >&
ResultSet::getEnvironment()
{
    return m_pImpl->m_xEnv;
}

} // namespace ucbhelper

namespace ucbhelper_impl {




// PropertySetInfo Implementation.




PropertySetInfo::PropertySetInfo(
    const PropertyInfo* pProps,
    sal_Int32 nProps )
{
    m_pProps = new uno::Sequence< beans::Property >( nProps );

    if ( nProps )
    {
        const PropertyInfo* pEntry = pProps;
        beans::Property* pProperties = m_pProps->getArray();

        for ( sal_Int32 n = 0; n < nProps; ++n )
        {
            beans::Property& rProp = pProperties[ n ];

            rProp.Name       = OUString::createFromAscii( pEntry->pName );
            rProp.Handle     = pEntry->nHandle;
            rProp.Type       = pEntry->pGetCppuType();
            rProp.Attributes = pEntry->nAttributes;

            pEntry++;
        }
    }
}


// virtual
PropertySetInfo::~PropertySetInfo()
{
    delete m_pProps;
}



// XInterface methods.
void SAL_CALL PropertySetInfo::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL PropertySetInfo::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL PropertySetInfo::queryInterface(
                                const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< beans::XPropertySetInfo* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.



XTYPEPROVIDER_IMPL_2( PropertySetInfo,
                         lang::XTypeProvider,
                         beans::XPropertySetInfo );



// XPropertySetInfo methods.



// virtual
uno::Sequence< beans::Property > SAL_CALL PropertySetInfo::getProperties()
    throw( uno::RuntimeException, std::exception )
{
    return uno::Sequence< beans::Property >( *m_pProps );
}


// virtual
beans::Property SAL_CALL PropertySetInfo::getPropertyByName(
        const OUString& aName )
    throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    beans::Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw beans::UnknownPropertyException();
}


// virtual
sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName(
        const OUString& Name )
    throw( uno::RuntimeException, std::exception )
{
    beans::Property aProp;
    return queryProperty( Name, aProp );
}


bool PropertySetInfo::queryProperty(
    const OUString& aName, beans::Property& rProp )
{
    sal_Int32 nCount = m_pProps->getLength();
    const beans::Property* pProps = m_pProps->getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property& rCurr = pProps[ n ];
        if ( rCurr.Name == aName )
        {
            rProp = rCurr;
            return true;
        }
    }

    return false;
}

} // namespace ucbhelper_impl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
