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

#include <memory>
#include <mutex>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <ucbhelper/resultset.hxx>
#include <ucbhelper/resultsetmetadata.hxx>
#include <ucbhelper/macros.hxx>
#include <utility>
#include <osl/diagnose.h>

using namespace com::sun::star;


namespace ucbhelper_impl
{

namespace {

struct PropertyInfo
{
    OUString    aName;
    sal_Int32   nHandle;
    sal_Int16   nAttributes;
    const uno::Type& (*pGetCppuType)();
};

}

static const uno::Type& sal_Int32_getCppuType()
{
    return cppu::UnoType<sal_Int32>::get();
}

static const uno::Type& sal_Bool_getCppuType()
{
    return cppu::UnoType<bool>::get();
}

constexpr PropertyInfo aPropertyTable[] =
{
    { u"IsRowCountFinal"_ustr,
      1000,
      beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY,
      &sal_Bool_getCppuType
    },
    { u"RowCount"_ustr,
      1001,
      beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY,
      &sal_Int32_getCppuType
    },
    { u""_ustr,
      0,
      0,
      nullptr
    }
};

#define RESULTSET_PROPERTY_COUNT 2



namespace {

class PropertySetInfo :
        public cppu::OWeakObject,
        public lang::XTypeProvider,
        public beans::XPropertySetInfo
{
    uno::Sequence< beans::Property >  m_aProps;

private:
    bool queryProperty(
        std::u16string_view aName, beans::Property& rProp ) const;

public:
    PropertySetInfo(
        const PropertyInfo* pProps,
        sal_Int32 nProps );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        noexcept override;
    virtual void SAL_CALL release()
        noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XPropertySetInfo
    virtual uno::Sequence< beans::Property > SAL_CALL getProperties() override;
    virtual beans::Property SAL_CALL getPropertyByName(
            const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;
};

}

typedef comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XPropertyChangeListener>
    PropertyChangeListeners;

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
    std::mutex                          m_aMutex;
    comphelper::OInterfaceContainerHelper4<lang::XEventListener> m_aDisposeEventListeners;
    std::unique_ptr<PropertyChangeListeners>        m_pPropertyChangeListeners;
    sal_Int32                           m_nPos;
    bool                            m_bWasNull;
    bool                            m_bAfterLast;

    inline ResultSet_Impl(
        uno::Reference< uno::XComponentContext > xContext,
        const uno::Sequence< beans::Property >& rProperties,
        rtl::Reference< ResultSetDataSupplier > xDataSupplier,
        uno::Reference< css::ucb::XCommandEnvironment > xEnv );
};

inline ResultSet_Impl::ResultSet_Impl(
    uno::Reference< uno::XComponentContext > xContext,
    const uno::Sequence< beans::Property >& rProperties,
    rtl::Reference< ResultSetDataSupplier > xDataSupplier,
    uno::Reference< css::ucb::XCommandEnvironment > xEnv )
: m_xContext(std::move( xContext )),
  m_xEnv(std::move( xEnv )),
  m_aProperties( rProperties ),
  m_xDataSupplier(std::move( xDataSupplier )),
  m_nPos( 0 ), // Position is one-based. Zero means: before first element.
  m_bWasNull( false ),
  m_bAfterLast( false )
{
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


// XServiceInfo methods.

OUString SAL_CALL ResultSet::getImplementationName()
{
    return u"ResultSet"_ustr;
}

sal_Bool SAL_CALL ResultSet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL ResultSet::getSupportedServiceNames()
{
    return { RESULTSET_SERVICE_NAME };
}


// XComponent methods.


// virtual
void SAL_CALL ResultSet::dispose()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_aDisposeEventListeners.getLength(aGuard) )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pImpl->m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< beans::XPropertySet * >( this );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aGuard, aEvt );
    }

    m_pImpl->m_xDataSupplier->close();
}


// virtual
void SAL_CALL ResultSet::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    m_pImpl->m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL ResultSet::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    m_pImpl->m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


// XResultSetMetaDataSupplier methods.


// virtual
uno::Reference< sdbc::XResultSetMetaData > SAL_CALL ResultSet::getMetaData()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xMetaData.is() )
        m_pImpl->m_xMetaData = new ResultSetMetaData( m_pImpl->m_xContext,
                                                      m_pImpl->m_aProperties );

    return m_pImpl->m_xMetaData;
}


// XResultSet methods.


// virtual
sal_Bool SAL_CALL ResultSet::next()
{
    // Note: Cursor is initially positioned before the first row.
    //       First call to 'next()' moves it to first row.

    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    // getResult works zero-based!
    if ( !m_pImpl->m_xDataSupplier->getResult( m_pImpl->m_nPos ) )
    {
        m_pImpl->m_bAfterLast = true;
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    m_pImpl->m_nPos++;
    m_pImpl->m_xDataSupplier->validate();
    return true;
}


// virtual
sal_Bool SAL_CALL ResultSet::isBeforeFirst()
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    // getResult works zero-based!
    if ( !m_pImpl->m_xDataSupplier->getResult( 0 ) )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == 0 );
}


// virtual
sal_Bool SAL_CALL ResultSet::isAfterLast()
{
    m_pImpl->m_xDataSupplier->validate();
    return m_pImpl->m_bAfterLast;
}


// virtual
sal_Bool SAL_CALL ResultSet::isFirst()
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == 1 );
}


// virtual
sal_Bool SAL_CALL ResultSet::isLast()
{
    if ( m_pImpl->m_bAfterLast )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( !nCount )
    {
        m_pImpl->m_xDataSupplier->validate();
        return false;
    }

    m_pImpl->m_xDataSupplier->validate();
    return ( m_pImpl->m_nPos == nCount );
}


// virtual
void SAL_CALL ResultSet::beforeFirst()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = false;
    m_pImpl->m_nPos = 0;
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
void SAL_CALL ResultSet::afterLast()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_bAfterLast = true;
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
sal_Bool SAL_CALL ResultSet::first()
{
    // getResult works zero-based!
    if ( m_pImpl->m_xDataSupplier->getResult( 0 ) )
    {
        std::unique_lock aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = false;
        m_pImpl->m_nPos = 1;
        m_pImpl->m_xDataSupplier->validate();
        return true;
    }

    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSet::last()
{
    sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
    if ( nCount )
    {
        std::unique_lock aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_bAfterLast = false;
        m_pImpl->m_nPos = nCount;
        m_pImpl->m_xDataSupplier->validate();
        return true;
    }

    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
sal_Int32 SAL_CALL ResultSet::getRow()
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
{
/*
    If the row number is positive, the cursor moves to the given row number
    with respect to the beginning of the result set. The first row is row 1,
    the second is row 2, and so on.

    If the given row number is negative, the cursor moves to an absolute row
    position with respect to the end of the result set. For example, calling
    absolute( -1 ) positions the cursor on the last row, absolute( -2 )
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
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = 0;
            m_pImpl->m_xDataSupplier->validate();
            return false;
        }
        else // |row| <= nCount
        {
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( nCount + row + 1 );
            m_pImpl->m_xDataSupplier->validate();
            return true;
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
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = row;
            m_pImpl->m_xDataSupplier->validate();
            return true;
        }
        else // row > nCount
        {
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = true;
            m_pImpl->m_xDataSupplier->validate();
            return false;
        }
    }

    // unreachable...
}


// virtual
sal_Bool SAL_CALL ResultSet::relative( sal_Int32 rows )
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
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return true;
        }
        else
        {
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = 0;
            m_pImpl->m_xDataSupplier->validate();
            return false;
        }
    }
    else if ( rows == 0 )
    {
        // nop.
        m_pImpl->m_xDataSupplier->validate();
        return true;
    }
    else // rows > 0
    {
        sal_Int32 nCount = m_pImpl->m_xDataSupplier->totalCount();
        if ( ( m_pImpl->m_nPos + rows ) <= nCount )
        {
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = false;
            m_pImpl->m_nPos = ( m_pImpl->m_nPos + rows );
            m_pImpl->m_xDataSupplier->validate();
            return true;
        }
        else
        {
            std::unique_lock aGuard( m_pImpl->m_aMutex );
            m_pImpl->m_bAfterLast = true;
            m_pImpl->m_xDataSupplier->validate();
            return false;
        }
    }

    // unreachable...
}


// virtual
sal_Bool SAL_CALL ResultSet::previous()
{
/*
    previous() is not the same as relative( -1 ) because it makes sense
    to call previous() when there is no current row.
*/
    std::unique_lock aGuard( m_pImpl->m_aMutex );

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
        return true;
    }

    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
void SAL_CALL ResultSet::refreshRow()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );
    if ( m_pImpl->m_bAfterLast || ( m_pImpl->m_nPos == 0 ) )
        return;

    m_pImpl->m_xDataSupplier->releasePropertyValues( m_pImpl->m_nPos );
    m_pImpl->m_xDataSupplier->validate();
}


// virtual
sal_Bool SAL_CALL ResultSet::rowUpdated()
{
    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSet::rowInserted()
{
    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSet::rowDeleted()
{
    m_pImpl->m_xDataSupplier->validate();
    return false;
}


// virtual
uno::Reference< uno::XInterface > SAL_CALL ResultSet::getStatement()
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
    return false;
}


// virtual
sal_Int8 SAL_CALL ResultSet::getByte( sal_Int32 columnIndex )
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
{
    m_pImpl->m_xDataSupplier->close();
    m_pImpl->m_xDataSupplier->validate();
}


// XContentAccess methods.


// virtual
OUString SAL_CALL ResultSet::queryContentIdentifierString()
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifierString(
                                                        m_pImpl->m_nPos - 1 );

    return OUString();
}


// virtual
uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
ResultSet::queryContentIdentifier()
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContentIdentifier(
                                                        m_pImpl->m_nPos - 1 );

    return uno::Reference< css::ucb::XContentIdentifier >();
}


// virtual
uno::Reference< css::ucb::XContent > SAL_CALL
ResultSet::queryContent()
{
    if ( m_pImpl->m_nPos && !m_pImpl->m_bAfterLast )
        return m_pImpl->m_xDataSupplier->queryContent( m_pImpl->m_nPos - 1 );

    return uno::Reference< css::ucb::XContent >();
}


// XPropertySet methods.


// virtual
uno::Reference< beans::XPropertySetInfo > SAL_CALL
ResultSet::getPropertySetInfo()
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.is() )
        m_pImpl->m_xPropSetInfo
            = new PropertySetInfo( aPropertyTable,
                                   RESULTSET_PROPERTY_COUNT );
    return m_pImpl->m_xPropSetInfo;
}


// virtual
void SAL_CALL ResultSet::setPropertyValue( const OUString& aPropertyName,
                                           const uno::Any& )
{
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
        throw beans::UnknownPropertyException(aPropertyName);
    }
}


// virtual
uno::Any SAL_CALL ResultSet::getPropertyValue(
        const OUString& PropertyName )
{
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
        throw beans::UnknownPropertyException(PropertyName);
    }

    return aValue;
}


// virtual
void SAL_CALL ResultSet::addPropertyChangeListener(
        const OUString& aPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    // Note: An empty property name means a listener for "all" properties.

    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( !aPropertyName.isEmpty() &&
         aPropertyName != "RowCount" &&
         aPropertyName != "IsRowCountFinal" )
        throw beans::UnknownPropertyException(aPropertyName);

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners.reset(
             new PropertyChangeListeners());

    m_pImpl->m_pPropertyChangeListeners->addInterface(aGuard,
                                                aPropertyName, xListener );
}


// virtual
void SAL_CALL ResultSet::removePropertyChangeListener(
        const OUString& aPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( !aPropertyName.isEmpty() &&
         aPropertyName != "RowCount" &&
         aPropertyName != "IsRowCountFinal" )
        throw beans::UnknownPropertyException(aPropertyName);

    if ( m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners->removeInterface(aGuard,
                                                    aPropertyName, xListener );

}


// virtual
void SAL_CALL ResultSet::addVetoableChangeListener(
        const OUString&,
        const uno::Reference< beans::XVetoableChangeListener >& )
{
    //  No constrained props, at the moment.
}


// virtual
void SAL_CALL ResultSet::removeVetoableChangeListener(
        const OUString&,
        const uno::Reference< beans::XVetoableChangeListener >& )
{
    //  No constrained props, at the moment.
}


// Non-interface methods.


void ResultSet::propertyChanged( const beans::PropertyChangeEvent& rEvt ) const
{
    std::unique_lock aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    // Notify listeners interested especially in the changed property.
    comphelper::OInterfaceContainerHelper4<beans::XPropertyChangeListener>* pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer(aGuard,
                                                        rEvt.PropertyName );
    if ( pPropsContainer )
    {
        pPropsContainer->notifyEach(aGuard, &beans::XPropertyChangeListener::propertyChange, rEvt);
    }

    // Notify listeners interested in all properties.
    pPropsContainer
        = m_pImpl->m_pPropertyChangeListeners->getContainer( aGuard, OUString() );
    if ( pPropsContainer )
    {
        pPropsContainer->notifyEach( aGuard, &beans::XPropertyChangeListener::propertyChange, rEvt);
    }
}


void ResultSet::rowCountChanged( sal_uInt32 nOld, sal_uInt32 nNew )
{
    OSL_ENSURE( nOld < nNew, "ResultSet::rowCountChanged - nOld >= nNew!" );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        beans::PropertyChangeEvent(
            getXWeak(),
            u"RowCount"_ustr,
            false,
            1001,
            uno::Any( nOld ),     // old value
            uno::Any( nNew ) ) ); // new value
}


void ResultSet::rowCountFinal()
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    propertyChanged(
        beans::PropertyChangeEvent(
            getXWeak(),
            u"IsRowCountFinal"_ustr,
            false,
            1000,
            uno:: Any( false ),   // old value
            uno::Any( true ) ) ); // new value
}


const uno::Sequence< beans::Property >& ResultSet::getProperties() const
{
    return m_pImpl->m_aProperties;
}


const uno::Reference< css::ucb::XCommandEnvironment >&
ResultSet::getEnvironment() const
{
    return m_pImpl->m_xEnv;
}

} // namespace ucbhelper

namespace ucbhelper_impl {


// PropertySetInfo Implementation.


PropertySetInfo::PropertySetInfo(
    const PropertyInfo* pProps,
    sal_Int32 nProps )
    : m_aProps( nProps )
{

    if ( !nProps )
        return;

    const PropertyInfo* pEntry = pProps;
    beans::Property* pProperties = m_aProps.getArray();

    for ( sal_Int32 n = 0; n < nProps; ++n )
    {
        beans::Property& rProp = pProperties[ n ];

        rProp.Name       = pEntry->aName;
        rProp.Handle     = pEntry->nHandle;
        rProp.Type       = pEntry->pGetCppuType();
        rProp.Attributes = pEntry->nAttributes;

        pEntry++;
    }
}



// XInterface methods.
void SAL_CALL PropertySetInfo::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL PropertySetInfo::release()
    noexcept
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL PropertySetInfo::queryInterface(
                                const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< lang::XTypeProvider* >(this),
                                               static_cast< beans::XPropertySetInfo* >(this)
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
{
    return m_aProps;
}


// virtual
beans::Property SAL_CALL PropertySetInfo::getPropertyByName(
        const OUString& aName )
{
    beans::Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw beans::UnknownPropertyException(aName);
}


// virtual
sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName(
        const OUString& Name )
{
    beans::Property aProp;
    return queryProperty( Name, aProp );
}


bool PropertySetInfo::queryProperty(
    std::u16string_view aName, beans::Property& rProp ) const
{
    sal_Int32 nCount = m_aProps.getLength();
    const beans::Property* pProps = m_aProps.getConstArray();
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
