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

#pragma once
#if 1
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

    virtual void fire( com::sun::star::lang::XEventListener * listener ) const
    {
        ((com::sun::star::util::XRefreshListener*)listener)->refreshed( m_event );
    }

    virtual com::sun::star::uno::Type getType() const
    {
        return getCppuType(
            (com::sun::star::uno::Reference< com::sun::star::util::XRefreshListener > *)0 );
    }
};

typedef ::boost::unordered_map
<
   rtl::OUString,
   sal_Int32,
   rtl::OUStringHash,
   ::std::equal_to< rtl::OUString >,
   Allocator< ::std::pair< const ::rtl::OUString , sal_Int32 > >
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
    ::rtl::OUString m_type;

public:
    Container(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const ::rtl::OUString & type  // for exception messages
        );

public: // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

public: // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
    SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

public: // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);
    // Methods
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (::com::sun::star::uno::RuntimeException);


public: // XAppend
    // Must be overriden in Non-Descriptors. May be overriden in descriptors, when
    // PropertySet.NAME != container name
    virtual void SAL_CALL appendByDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException);

    // helper method !
    void append(
        const rtl::OUString & str,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw ( ::com::sun::star::container::ElementExistException );


public: // XDrop
    virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropByIndex( sal_Int32 index )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException) = 0;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL addRefreshListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw (::com::sun::star::uno::RuntimeException);

public:
    // Methods
    virtual void SAL_CALL addContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

public:
    virtual void SAL_CALL disposing();

public:
    void rename( const rtl::OUString & oldName, const rtl::OUString &newName );

protected:
    void fire( const EventBroadcastHelper & helper );
};

}
#endif
