/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  $RCSfile: pq_xcolumns.hxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: jbu $ $Date: 2004/06/10 15:27:00 $
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
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *              2011 by Lionel Elie Mamane <lionel@mamane.lu>
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *
 ************************************************************************/

#ifndef _PQ_COLUMNS_HXX_
#define _PQ_COLUMNS_HXX_

#include "pq_xcontainer.hxx"
#include "pq_xbase.hxx"

namespace pq_sdbc_driver
{

void alterColumnByDescriptor(
    const rtl::OUString & schemaName,
    const rtl::OUString & tableName,
    ConnectionSettings *settings,
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > &stmt,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & past,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & future);

rtl::OUString columnMetaData2SDBCX(
    ReflectionBase *pBase, const com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > &xRow );

class Columns : public Container
{
    rtl::OUString m_schemaName;
    rtl::OUString m_tableName;

public: // instances Columns 'exception safe'
    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > create(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const rtl::OUString &schemaName,
        const rtl::OUString &tableName,
        Columns **pColumns);

protected:
    Columns(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const rtl::OUString &schemaName,
        const rtl::OUString &tableName);


    virtual ~Columns();

public: // XAppend
    virtual void SAL_CALL appendByDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException);

// public: // XDrop
//     virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName )
//         throw (::com::sun::star::sdbc::SQLException,
//                ::com::sun::star::container::NoSuchElementException,
//                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropByIndex( sal_Int32 index )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) throw (::com::sun::star::uno::RuntimeException);

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException);
};


class ColumnDescriptors : public Container
{
public:
    ColumnDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings );

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException);
};

}
#endif
