/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OEnumeration.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART_OENUMERATION_HXX
#define CHART_OENUMERATION_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XEnumeration.hpp>

#include <vector>

namespace comphelper
{

class OEnumeration :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::container::XEnumeration >
{
public:
    OEnumeration( const ::std::vector< ::com::sun::star::uno::Any > & rContainer );
    virtual ~OEnumeration();

protected:
    // ____ XEnumeration ____
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

private:
    ::std::vector< ::com::sun::star::uno::Any > m_aContainer;
    ::std::vector< ::com::sun::star::uno::Any >::const_iterator m_aIter;
};

} //  namespace comphelper

// CHART_OENUMERATION_HXX
#endif
