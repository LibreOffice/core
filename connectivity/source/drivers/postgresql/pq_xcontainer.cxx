/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/implbase.hxx>

#include "pq_xcontainer.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;

using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;

using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XContainerListener;
using com::sun::star::container::ContainerEvent;
using com::sun::star::lang::IndexOutOfBoundsException;
using com::sun::star::lang::XEventListener;

using com::sun::star::lang::WrappedTargetException;


namespace pq_sdbc_driver
{

class ReplacedBroadcaster : public EventBroadcastHelper
{
    ContainerEvent m_event;
public:
    ReplacedBroadcaster(
        const Reference< XInterface > & source,
        const OUString & name,
        const Any & newElement,
        const OUString & oldElement ) :
        m_event( source, makeAny( name ), newElement, makeAny(oldElement) )
    {}

    virtual void fire( XEventListener * listener ) const override
    {
        static_cast<XContainerListener*>(listener)->elementReplaced( m_event );
    }
    virtual Type getType() const override
    {
        return cppu::UnoType<XContainerListener>::get();
    }
};

class InsertedBroadcaster : public EventBroadcastHelper
{
public:
    ContainerEvent m_event;
    InsertedBroadcaster(
        const Reference< XInterface > & source,
        const OUString & name,
        const Any & newElement ) :
        m_event( source, makeAny( name ), newElement, Any() )
    {}

    virtual void fire( XEventListener * listener ) const override
    {
        static_cast<XContainerListener*>(listener)->elementInserted( m_event );
    }

    virtual Type getType() const override
    {
        return cppu::UnoType<XContainerListener>::get();
    }
};

class RemovedBroadcaster : public EventBroadcastHelper
{
public:
    ContainerEvent m_event;
    RemovedBroadcaster(
        const Reference< XInterface > & source,
        const OUString & name) :
        m_event( source, makeAny( name ), Any(), Any() )
    {}

    virtual void fire( XEventListener * listener ) const override
    {
        static_cast<XContainerListener*>(listener)->elementRemoved( m_event );
    }

    virtual Type getType() const override
    {
        return cppu::UnoType<XContainerListener>::get();
    }
};

Container::Container(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString &type)
    : ContainerBase( refMutex->mutex ),
      m_refMutex( refMutex ),
      m_pSettings( pSettings ),
      m_origin( origin ),
      m_type( type )
{
}

Any Container::getByName( const OUString& aName )
    throw (NoSuchElementException,WrappedTargetException,RuntimeException, std::exception)
{
    String2IntMap::const_iterator ii = m_name2index.find( aName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf(128);
        buf.append( "Element "  );
        buf.append( aName );
        buf.append( " unknown in " );
        buf.append( m_type );
        buf.append( "-Container" );
        throw NoSuchElementException( buf.makeStringAndClear() , *this );
    }
    OSL_ASSERT( ii->second >= 0 && ii->second < (int)m_values.size() );
    return m_values[ ii->second ];
}

Sequence< OUString > Container::getElementNames(  )
        throw (css::uno::RuntimeException, std::exception)
{
    Sequence< OUString > ret( m_values.size() );
    for( String2IntMap::const_iterator ii = m_name2index.begin();
         ii != m_name2index.end() ;
         ++ ii )
    {
        // give element names in index order !
        ret[ii->second] = ii->first;
//         ret[i] = ii->first;
    }
    return ret;
}

sal_Bool Container::hasByName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception)
{
    return m_name2index.find( aName ) != m_name2index.end();
}
    // Methods
Type Container::getElementType(  )
        throw (css::uno::RuntimeException, std::exception)
{
    return Type();
}

sal_Bool Container::hasElements(  )
        throw (css::uno::RuntimeException, std::exception)
{
    return ! m_name2index.empty();
}

Any Container::getByIndex( sal_Int32 Index )
    throw (css::lang::IndexOutOfBoundsException,
           css::lang::WrappedTargetException,
           css::uno::RuntimeException, std::exception)
{
    if( Index < 0 || Index >= (sal_Int32)m_values.size() )
    {
        OUStringBuffer buf(128);
        buf.append( "Index " );
        buf.append( Index );
        buf.append(" out of range for " );
        buf.append( m_type );
        buf.append("-Container, expected 0 <= x <= " );
        buf.append( (sal_Int32 ) (m_values.size() -1));
        throw IndexOutOfBoundsException( buf.makeStringAndClear(), *this );
    }
    return m_values[Index];
}

sal_Int32 Container::getCount()
        throw (css::uno::RuntimeException, std::exception)
{
    return m_values.size();
}


class ContainerEnumeration : public ::cppu::WeakImplHelper< XEnumeration >
{
    std::vector< css::uno::Any > m_vec;
    sal_Int32 m_index;
public:
    explicit ContainerEnumeration( const std::vector< css::uno::Any > &vec )
        : m_vec( vec ),
          m_index( -1 )
    {}

public:
    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL nextElement(  )
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

};

sal_Bool ContainerEnumeration::hasMoreElements()
        throw (css::uno::RuntimeException, std::exception)
{
    return (int)m_vec.size() > m_index +1;
}

css::uno::Any ContainerEnumeration::nextElement()
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception)
{
    if( ! hasMoreElements() )
    {
        throw NoSuchElementException(
            "NoSuchElementException during enumeration", *this );
    }
    m_index ++;
    return m_vec[m_index];
}

Reference< XEnumeration > Container::createEnumeration(  )
    throw (css::uno::RuntimeException, std::exception)
{
    return new ContainerEnumeration( m_values );
}

void Container::addRefreshListener(
    const css::uno::Reference< css::util::XRefreshListener >& l )
    throw (css::uno::RuntimeException, std::exception)
{
    rBHelper.addListener( cppu::UnoType<decltype(l)>::get() , l );
}

void Container::removeRefreshListener(
    const css::uno::Reference< css::util::XRefreshListener >& l )
    throw (css::uno::RuntimeException, std::exception)
{
    rBHelper.removeListener( cppu::UnoType<decltype(l)>::get() , l );
}

void Container::disposing()
{
    m_origin.clear();
}

void Container::rename( const OUString &oldName, const OUString &newName )
{
    Any newValue;
    {
        osl::MutexGuard guard ( m_refMutex->mutex );
        String2IntMap::iterator ii = m_name2index.find( oldName );
        if( ii != m_name2index.end() )
        {
            sal_Int32 nIndex = ii->second;
            newValue = m_values[nIndex];
            m_name2index.erase( ii );
            m_name2index[ newName ] = nIndex;
        }
    }
    fire( ReplacedBroadcaster( *this, newName, newValue, oldName ) );
    fire( RefreshedBroadcaster( *this ) );
}

void Container::dropByName( const OUString& elementName )
    throw (css::sdbc::SQLException,
           css::container::NoSuchElementException,
           css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    String2IntMap::const_iterator ii = m_name2index.find( elementName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "Column " );
        buf.append( elementName );
        buf.append( " is unknown in " );
        buf.append( m_type );
//         buf.appendAscii( " " );
//         buf.append( m_schemaName );
//         buf.appendAscii( "." );
//         buf.append( m_tableName );
        buf.append( " container, so it can't be dropped" );
        throw css::container::NoSuchElementException(
            buf.makeStringAndClear(), *this );
    }
    dropByIndex( ii->second );
}

void Container::dropByIndex( sal_Int32 index )
    throw (css::sdbc::SQLException,
           css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >=(sal_Int32)m_values.size() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "Index out of range (allowed 0 to " );
        buf.append((sal_Int32)(m_values.size() -1) );
        buf.append( ", got " );
        buf.append( index );
        buf.append( ") in " );
        buf.append( m_type );
        throw css::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }

    OUString name;
    for( String2IntMap::iterator ii = m_name2index.begin() ;
         ii != m_name2index.end() ;
         ++ ii )
    {
        if( ii->second == index )
        {
            name = ii->first;
            m_name2index.erase( ii );
            break;
        }
    }

    for( int i = index +1 ; i < (int)m_values.size() ; i ++ )
    {
        m_values[i-1] = m_values[i];

        // I know, this is expensive, but don't want to maintain another map ...
        for( String2IntMap::iterator ii = m_name2index.begin() ;
             ii != m_name2index.end() ;
             ++ ii )
        {
            if( ii->second == i )
            {
                ii->second = i-1;
                break;
            }
        }
    }
    m_values.resize( m_values.size() - 1 );

    fire( RemovedBroadcaster( *this, name ) );
}

void Container::append(
    const OUString & name,
    const css::uno::Reference< css::beans::XPropertySet >& descriptor )
    throw ( css::container::ElementExistException )

{
    osl::MutexGuard guard( m_refMutex->mutex );

    if( hasByName( name ) )
    {
        OUStringBuffer buf( 128 );
        buf.append( "a ");
        buf.append( m_type );
        buf.append( " with name " );
        buf.append( name );
        buf.append( " already exists in this container" );
        throw css::container::ElementExistException(
            buf.makeStringAndClear() , *this );
    }

    int index = m_values.size();
    m_values.push_back( makeAny( descriptor ) );
    m_name2index[name] = index;

    fire( InsertedBroadcaster( *this, name, makeAny( descriptor ) ) );
}

void Container::appendByDescriptor(
    const css::uno::Reference< css::beans::XPropertySet >& descriptor)
    throw (css::sdbc::SQLException,
           css::container::ElementExistException,
           css::uno::RuntimeException, std::exception)
{
    append( extractStringProperty( descriptor, getStatics().NAME ), descriptor );
}


void Container::addContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& l )
        throw (css::uno::RuntimeException, std::exception)
{
    rBHelper.addListener( cppu::UnoType<decltype(l)>::get() , l );
}

void Container::removeContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& l )
        throw (css::uno::RuntimeException, std::exception)
{
    rBHelper.removeListener( cppu::UnoType<decltype(l)>::get() , l );
}


void Container::fire( const EventBroadcastHelper &helper )
{
    Reference< css::util::XRefreshListener > l;
    cppu::OInterfaceContainerHelper *container = rBHelper.getContainer( helper.getType() );
    if( container )
    {
        cppu::OInterfaceIteratorHelper iterator( * container );
        while( iterator.hasMoreElements() )
        {
            try
            {
                helper.fire( static_cast<XEventListener *>(iterator.next()) );
            }
            catch ( css::uno::RuntimeException & )
            {
                OSL_ENSURE( false, "exception catched" );
                // loose coupling, a runtime exception shall not break anything
                // TODO: log away as warning !
            }
            catch( css::uno::Exception & )
            {
                OSL_ENSURE( false, "exception from listener flying through" );
                throw;
            }
        }
    }

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
