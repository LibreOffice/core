/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumeration.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:17:26 $
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

#ifndef _ENUMERATION_HXX
#define _ENUMERATION_HXX


#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace uno { class Any; }
    namespace container { class NoSuchElementException; }
    namespace lang { class WrappedTargetException; }
    namespace uno { class RuntimeException; }
} } }

/** implement XEnumeration based on container::XIndexAccess */
class Enumeration
    : public cppu::WeakImplHelper1<com::sun::star::container::XEnumeration>
{
    com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> mxContainer;
    sal_Int32 mnIndex;

public:
    Enumeration( const com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess>& );

    Enumeration( com::sun::star::container::XIndexAccess* );

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL nextElement()
        throw( com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
};

#endif
