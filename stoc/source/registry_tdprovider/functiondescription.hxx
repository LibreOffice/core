/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: functiondescription.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:03:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
 ************************************************************************/

#ifndef INCLUDED_stoc_source_registry_tdprovider_functiondescription_hxx
#define INCLUDED_stoc_source_registry_tdprovider_functiondescription_hxx

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "osl/mutex.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace reflection {
        class XCompoundTypeDescription;
    }
} } }
namespace typereg { class Reader; }

namespace stoc { namespace registry_tdprovider {

class FunctionDescription {
public:
    FunctionDescription(
        com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess > const &
            manager,
        com::sun::star::uno::Sequence< sal_Int8 > const & bytes,
        sal_uInt16 index);

    ~FunctionDescription();

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XCompoundTypeDescription > >
    getExceptions() const;

protected:
    typereg::Reader getReader() const;

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > m_manager;
    com::sun::star::uno::Sequence< sal_Int8 > m_bytes;
    sal_uInt16 m_index;

    mutable osl::Mutex m_mutex;
    mutable com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XCompoundTypeDescription > >
                m_exceptions;
    mutable bool m_exceptionsInit;

private:
    FunctionDescription(FunctionDescription &); // not implemented
    void operator =(FunctionDescription); // not implemented
};

} }

#endif
