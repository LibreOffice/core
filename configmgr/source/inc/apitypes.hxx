/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef CONFIGMGR_API_APITYPES_HXX_
#define CONFIGMGR_API_APITYPES_HXX_

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif  //  INCLUDED_VECTOR

namespace configmgr
{
    namespace configapi
    {
        namespace uno = ::com::sun::star::uno;

        inline
        uno::Type getAnyType( )
        {
            return ::getCppuType( static_cast< uno::Any const * >(0) );
        }

        inline
        uno::Type getUnoInterfaceType( )
        {
            return ::getCppuType( static_cast< uno::Reference< uno::XInterface > const * >(0) );
        }

        template <typename Interface>
        inline
        uno::Type getReferenceType( Interface const* )
        {
            return ::getCppuType( static_cast< uno::Reference<Interface> const * >(0) );
        }

        template <typename Type>
        inline
        uno::Type getSequenceType( Type const* )
        {
            return ::getCppuType( static_cast< uno::Sequence<Type> const * >(0) );
        }

        template <typename T>
        inline
        uno::Sequence<T> makeSequence(::std::vector<T> const& aVector)
        {
            if (aVector.empty())
                return uno::Sequence<T>();
            return uno::Sequence<T>(&aVector[0],aVector.size());
        }
    }

}

#endif // CONFIGMGR_API_APITYPES_HXX_


