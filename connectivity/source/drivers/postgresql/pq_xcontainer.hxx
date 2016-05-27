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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XCONTAINER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XCONTAINER_HXX
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>

#include <com/sun/star/util/XRefreshable.hpp>

#include <cppuhelper/compbase.hxx>

#include <unordered_map>

#include "pq_connection.hxx"
#include "pq_statics.hxx"

namespace pq_sdbc_driver
{

class EventBroadcastHelper
{
public:
    virtual void fire(css::lang::XEventListener * listener) const = 0;
    virtual css::uno::Type getType() const = 0;
    virtual ~EventBroadcastHelper(){};
};

class RefreshedBroadcaster : public EventBroadcastHelper
{
    css::lang::EventObject m_event;
public:
    explicit RefreshedBroadcaster(const css::uno::Reference< css::uno::XInterface > & source ) :
        m_event( source )
    {}

    virtual void fire( css::lang::XEventListener * listener ) const override
    {
        static_cast<css::util::XRefreshListener*>(listener)->refreshed( m_event );
    }

    virtual css::uno::Type getType() const override
    {
        return cppu::UnoType<
            css::util::XRefreshListener>::get();
    }
};

typedef std::unordered_map
<
   OUString,
   sal_Int32,
   OUStringHash
> String2IntMap;

typedef ::cppu::WeakComponentImplHelper
<
    css::container::XNameAccess,
    css::container::XIndexAccess,
    css::container::XEnumerationAccess,
    css::sdbcx::XAppend,
    css::sdbcx::XDrop,
    css::util::XRefreshable,
    css::sdbcx::XDataDescriptorFactory,
    css::container::XContainer
> ContainerBase;

class /* abstract */ Container : public ContainerBase
{
protected:
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ConnectionSettings *m_pSettings;
    css::uno::Reference< css::sdbc::XConnection > m_origin;
    String2IntMap m_name2index;  // maps the element name to an index
    std::vector< css::uno::Any > m_values; // contains the real values
    OUString m_type;

public:
    Container(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString & type  // for exception messages
        );

public: // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

public: // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >
    SAL_CALL createEnumeration(  ) throw (css::uno::RuntimeException, std::exception) override;

public: // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception) override;
    // Methods
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw (css::uno::RuntimeException, std::exception) override;


public: // XAppend
    // Must be overridden in Non-Descriptors. May be overridden in descriptors, when
    // PropertySet.NAME != container name
    virtual void SAL_CALL appendByDescriptor(
        const css::uno::Reference< css::beans::XPropertySet >& descriptor )
        throw (css::sdbc::SQLException,
               css::container::ElementExistException,
               css::uno::RuntimeException, std::exception) override;

    // helper method !
    void append(
        const OUString & str,
        const css::uno::Reference< css::beans::XPropertySet >& descriptor )
        throw ( css::container::ElementExistException );


public: // XDrop
    virtual void SAL_CALL dropByName( const OUString& elementName )
        throw (css::sdbc::SQLException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dropByIndex( sal_Int32 index )
        throw (css::sdbc::SQLException,
               css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (css::uno::RuntimeException, std::exception) override = 0;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL addRefreshListener(
        const css::uno::Reference< css::util::XRefreshListener >& l )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRefreshListener(
        const css::uno::Reference< css::util::XRefreshListener >& l )
        throw (css::uno::RuntimeException, std::exception) override;

public:
    // Methods
    virtual void SAL_CALL addContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

public:
    virtual void SAL_CALL disposing() override;

public:
    void rename( const OUString & oldName, const OUString &newName );

protected:
    void fire( const EventBroadcastHelper & helper );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
