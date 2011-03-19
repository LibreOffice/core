/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVTOOLS_BRWIMPL_HXX
#define _SVTOOLS_BRWIMPL_HXX

#include "svtaccessiblefactory.hxx"
#include <com/sun/star/lang/XComponent.hpp>

#include <map>
#include <functional>

namespace svt
{
    class BrowseBoxImpl
    {
    // member
    public:
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >    AccessibleRef;
        typedef ::std::map< sal_Int32, AccessibleRef >                                              THeaderCellMap;

        struct  THeaderCellMapFunctorDispose : ::std::unary_function<THeaderCellMap::value_type,void>
        {
            inline void operator()(const THeaderCellMap::value_type& _aType)
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp(
                    _aType.second, ::com::sun::star::uno::UNO_QUERY );
                OSL_ENSURE( xComp.is() || !_aType.second.is(), "THeaderCellMapFunctorDispose: invalid accessible cell (no XComponent)!" );
                if ( xComp.is() )
                    try
                    {
                        xComp->dispose();
                    }
                    catch( const ::com::sun::star::uno::Exception& )
                    {
                        OSL_FAIL( "THeaderCellMapFunctorDispose: caught an exception!" );
                    }
            }
        };

    public:
        AccessibleFactoryAccess m_aFactoryAccess;
        IAccessibleBrowseBox*   m_pAccessible;
        THeaderCellMap          m_aColHeaderCellMap;
        THeaderCellMap          m_aRowHeaderCellMap;

    public:
        BrowseBoxImpl() : m_pAccessible(NULL)
        {
        }


        /// @see AccessibleBrowseBox::getHeaderBar
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            getAccessibleHeaderBar( AccessibleBrowseBoxObjType _eObjType );

        /// @see AccessibleBrowseBox::getTable
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            getAccessibleTable( );

    };
}

#endif // _SVTOOLS_BRWIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
