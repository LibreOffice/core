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

#ifndef _PQ_CONTAINER_HXX_
#define _PQ_CONTAINER_HXX_
#include <boost/unordered_map.hpp>
#include <vector>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>

#include <com/sun/star/util/XRefreshable.hpp>

#include <cppuhelper/compbase8.hxx>

#include "pq_connection.hxx"
#include "pq_statics.hxx"

namespace pq_sdbc_driver
{

class EventBroadcastHelper
{
public:
    virtual void fire(com::sun::star::lang::XEventListener * listener) const = 0;
    virtual com::sun::star::uno::Type getType() const = 0;
    virtual ~EventBroadcastHelper(){};
};

class RefreshedBroadcaster : public EventBroadcastHelper
{
    com::sun::star::lang::EventObject m_event;
public:
    RefreshedBroadcaster(const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > & source ) :
        m_event( source )
    {}

    virtual void fire( com::sun::star::lang::XEventListener * listener ) const SAL_OVERRIDE
    {
        ((com::sun::star::util::XRefreshListener*)listener)->refreshed( m_event );
    }

    virtual com::sun::star::uno::Type getType() const SAL_OVERRIDE
    {
        return getCppuType(
            (com::sun::star::uno::Reference< com::sun::star::util::XRefreshListener > *)0 );
    }
};

typedef ::boost::unordered_map
<
   OUString,
   sal_Int32,
   OUStringHash,
   ::std::equal_to< OUString >,
   Allocator< ::std::pair< const OUString , sal_Int32 > >
> String2IntMap;

typedef ::cppu::WeakComponentImplHelper8
<
    com::sun::star::container::XNameAccess,
    com::sun::star::container::XIndexAccess,
    com::sun::star::container::XEnumerationAccess,
    com::sun::star::sdbcx::XAppend,
    com::sun::star::sdbcx::XDrop,
    com::sun::star::util::XRefreshable,
    com::sun::star::sdbcx::XDataDescriptorFactory,
    com::sun::star::container::XContainer
> ContainerBase;

class /* abstract */ Container : public ContainerBase
{
protected:
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ConnectionSettings *m_pSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_origin;
    String2IntMap m_name2index;  // maps the element name to an index
    ::com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_values; // contains the real values
    OUString m_type;

public:
    Container(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString & type  // for exception messages
        );

public: // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public: // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
    SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public: // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // Methods
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


public: // XAppend
    // Must be overridden in Non-Descriptors. May be overridden in descriptors, when
    // PropertySet.NAME != container name
    virtual void SAL_CALL appendByDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // helper method !
    void append(
        const OUString & str,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw ( ::com::sun::star::container::ElementExistException );


public: // XDrop
    virtual void SAL_CALL dropByName( const OUString& elementName )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL dropByIndex( sal_Int32 index )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {}
    virtual void SAL_CALL addRefreshListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRefreshListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    // Methods
    virtual void SAL_CALL addContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

public:
    void rename( const OUString & oldName, const OUString &newName );

protected:
    void fire( const EventBroadcastHelper & helper );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
