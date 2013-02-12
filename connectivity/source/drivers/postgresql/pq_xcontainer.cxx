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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>


#include <cppuhelper/implbase1.hxx>

#include "pq_xcontainer.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
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

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbcx::XDataDescriptorFactory;

namespace pq_sdbc_driver
{

class ReplacedBroadcaster : public EventBroadcastHelper
{
    ContainerEvent m_event;
public:
    ReplacedBroadcaster(
        const Reference< XInterface > & source,
        const rtl::OUString & name,
        const Any & newElement,
        const rtl::OUString & oldElement ) :
        m_event( source, makeAny( name ), newElement, makeAny(oldElement) )
    {}

    virtual void fire( XEventListener * listener ) const
    {
        ((XContainerListener*)listener)->elementReplaced( m_event );
    }
    virtual Type getType() const
    {
        return getCppuType( (Reference< XContainerListener > *)0 );
    }
};

class InsertedBroadcaster : public EventBroadcastHelper
{
public:
    ContainerEvent m_event;
    InsertedBroadcaster(
        const Reference< XInterface > & source,
        const rtl::OUString & name,
        const Any & newElement ) :
        m_event( source, makeAny( name ), newElement, Any() )
    {}

    virtual void fire( XEventListener * listener ) const
    {
        ((XContainerListener*)listener)->elementInserted( m_event );
    }

    virtual Type getType() const
    {
        return getCppuType( (Reference< XContainerListener > *)0 );
    }
};

class RemovedBroadcaster : public EventBroadcastHelper
{
public:
    ContainerEvent m_event;
    RemovedBroadcaster(
        const Reference< XInterface > & source,
        const rtl::OUString & name) :
        m_event( source, makeAny( name ), Any(), Any() )
    {}

    virtual void fire( XEventListener * listener ) const
    {
        ((XContainerListener*)listener)->elementRemoved( m_event );
    }

    virtual Type getType() const
    {
        return getCppuType( (Reference< XContainerListener > *)0 );
    }
};

Container::Container(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const ::rtl::OUString &type)
    : ContainerBase( refMutex->mutex ),
      m_refMutex( refMutex ),
      m_pSettings( pSettings ),
      m_origin( origin ),
      m_type( type )
{
}

Any Container::getByName( const ::rtl::OUString& aName )
    throw (NoSuchElementException,WrappedTargetException,RuntimeException)
{
    String2IntMap::const_iterator ii = m_name2index.find( aName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "Element "  );
        buf.append( aName );
        buf.appendAscii( " unknown in " );
        buf.append( m_type );
        buf.appendAscii( "-Container" );
        throw NoSuchElementException( buf.makeStringAndClear() , *this );
    }
    OSL_ASSERT( ii->second >= 0 && ii->second < m_values.getLength() );
    return m_values[ ii->second ];
}

Sequence< OUString > Container::getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > ret( m_values.getLength() );
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

sal_Bool Container::hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException)
{
    return m_name2index.find( aName ) != m_name2index.end();
}
    // Methods
Type Container::getElementType(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return Type();
}

sal_Bool Container::hasElements(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return ! m_name2index.empty();
}

Any Container::getByIndex( sal_Int32 Index )
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException)
{
    if( Index < 0 || Index >= m_values.getLength() )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "Index " );
        buf.append( Index );
        buf.appendAscii(" out of range for " );
        buf.append( m_type );
        buf.appendAscii("-Container, expected 0 <= x <= " );
        buf.append( (sal_Int32 ) (m_values.getLength() -1));
        throw IndexOutOfBoundsException( buf.makeStringAndClear(), *this );
    }
    return m_values[Index];
}

sal_Int32 Container::getCount()
        throw (::com::sun::star::uno::RuntimeException)
{
    return m_values.getLength();
}


class ContainerEnumeration : public ::cppu::WeakImplHelper1< XEnumeration >
{
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_vec;
    sal_Int32 m_index;
public:
    ContainerEnumeration( const  com::sun::star::uno::Sequence< com::sun::star::uno::Any > &vec )
        : m_vec( vec ),
          m_index( -1 )
    {}

public:
    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

};

sal_Bool ContainerEnumeration::hasMoreElements()
        throw (::com::sun::star::uno::RuntimeException)
{
    return m_vec.getLength() > m_index +1;
}

com::sun::star::uno::Any ContainerEnumeration::nextElement()
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException)
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
    throw (::com::sun::star::uno::RuntimeException)
{
    return new ContainerEnumeration( m_values );
}

void Container::addRefreshListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
    throw (::com::sun::star::uno::RuntimeException)
{
    rBHelper.addListener( getCppuType(&l) , l );
}

void Container::removeRefreshListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
    throw (::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( getCppuType(&l) , l );
}

void Container::disposing()
{
    m_origin.clear();
}

void Container::rename( const rtl::OUString &oldName, const rtl::OUString &newName )
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

void Container::dropByName( const ::rtl::OUString& elementName )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::NoSuchElementException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    String2IntMap::const_iterator ii = m_name2index.find( elementName );
    if( ii == m_name2index.end() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "Column " );
        buf.append( elementName );
        buf.appendAscii( " is unknown in " );
        buf.append( m_type );
//         buf.appendAscii( " " );
//         buf.append( m_schemaName );
//         buf.appendAscii( "." );
//         buf.append( m_tableName );
        buf.appendAscii( " container, so it can't be dropped" );
        throw com::sun::star::container::NoSuchElementException(
            buf.makeStringAndClear(), *this );
    }
    dropByIndex( ii->second );
}

void Container::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "Index out of range (allowed 0 to " );
        buf.append((sal_Int32)(m_values.getLength() -1) );
        buf.appendAscii( ", got " );
        buf.append( index );
        buf.appendAscii( ") in " );
        buf.append( m_type );
        throw com::sun::star::lang::IndexOutOfBoundsException(
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

    Any oldElement = m_values[index];
    for( int i = index +1 ; i < m_values.getLength() ; i ++ )
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
    m_values.realloc( m_values.getLength() - 1 );

    fire( RemovedBroadcaster( *this, name ) );
}

void Container::append(
    const rtl::OUString & name,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw ( ::com::sun::star::container::ElementExistException )

{
    osl::MutexGuard guard( m_refMutex->mutex );

    if( hasByName( name ) )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "a ");
        buf.append( m_type );
        buf.appendAscii( " with name " );
        buf.append( name );
        buf.appendAscii( " already exists in this container" );
        throw com::sun::star::container::ElementExistException(
            buf.makeStringAndClear() , *this );
    }

    int index = m_values.getLength();
    m_values.realloc( m_values.getLength() + 1 );
    m_values[index] = makeAny( descriptor );
    m_name2index[name] = index;

    fire( InsertedBroadcaster( *this, name, makeAny( descriptor ) ) );
}

void Container::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor)
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    append( extractStringProperty( descriptor, getStatics().NAME ), descriptor );
}


void Container::addContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l )
        throw (::com::sun::star::uno::RuntimeException)
{
    rBHelper.addListener( getCppuType(&l) , l );
}

void Container::removeContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l )
        throw (::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( getCppuType(&l) , l );
}


void Container::fire( const EventBroadcastHelper &helper )
{
    Reference< ::com::sun::star::util::XRefreshListener > l;
    cppu::OInterfaceContainerHelper *container = rBHelper.getContainer( helper.getType() );
    if( container )
    {
        cppu::OInterfaceIteratorHelper iterator( * container );
        while( iterator.hasMoreElements() )
        {
            try
            {
                helper.fire( (XEventListener * ) iterator.next() );
            }
            catch ( com::sun::star::uno::RuntimeException & )
            {
                OSL_ENSURE( 0, "exception catched" );
                // loose coupling, a runtime exception shall not break anything
                // TODO: log away as warning !
            }
            catch( com::sun::star::uno::Exception & )
            {
                OSL_ENSURE( 0, "exception from listener flying through" );
                throw;
            }
        }
    }

}

}
