/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: enumeration.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
