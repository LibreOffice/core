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

#pragma once

#include "pq_xcontainer.hxx"

namespace pq_sdbc_driver
{
class Keys final : public Container
{
    OUString m_schemaName;
    OUString m_tableName;

public: // instances Columns 'exception safe'
    static css::uno::Reference< css::container::XIndexAccess > create(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName);

private:
    Keys(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName);

    virtual ~Keys() override;

public: // XAppend
    virtual void SAL_CALL appendByDescriptor(
        const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

public: // XDrop
    virtual void SAL_CALL dropByIndex( sal_Int32 index ) override;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) override;

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override;
};


class KeyDescriptors final : public Container
{
public: // instances Columns 'exception safe'
    static css::uno::Reference< css::container::XIndexAccess > create(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings );

private:
    KeyDescriptors(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings );

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
