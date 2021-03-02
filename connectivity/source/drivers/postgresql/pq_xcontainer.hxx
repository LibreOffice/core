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

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>

#include <com/sun/star/util/XRefreshable.hpp>

#include <comphelper/refcountedmutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <unordered_map>

#include "pq_statics.hxx"

namespace pq_sdbc_driver
{

struct ConnectionSettings;

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
   sal_Int32
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
    ::rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
    ConnectionSettings *m_pSettings;
    css::uno::Reference< css::sdbc::XConnection > m_origin;
    String2IntMap m_name2index;  // maps the element name to an index
    std::vector< css::uno::Any > m_values; // contains the real values
    OUString m_type;

public:
    Container(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString & type  // for exception messages
        );

public: // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

public: // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >
    SAL_CALL createEnumeration(  ) override;

public: // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;
    // Methods
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;


public: // XAppend
    // Must be overridden in Non-Descriptors. May be overridden in descriptors, when
    // PropertySet.NAME != container name
    virtual void SAL_CALL appendByDescriptor(
        const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

    // helper method !
    /// @throws css::container::ElementExistException
    void append(
        const OUString & str,
        const css::uno::Reference< css::beans::XPropertySet >& descriptor );


public: // XDrop
    virtual void SAL_CALL dropByName( const OUString& elementName ) override;
    virtual void SAL_CALL dropByIndex( sal_Int32 index ) override;

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override = 0;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) override {}
    virtual void SAL_CALL addRefreshListener(
        const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL removeRefreshListener(
        const css::uno::Reference< css::util::XRefreshListener >& l ) override;

public:
    // Methods
    virtual void SAL_CALL addContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener(
        const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

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
