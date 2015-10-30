/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVTOOLS_SOURCE_BRWBOX_BRWIMPL_HXX
#define INCLUDED_SVTOOLS_SOURCE_BRWBOX_BRWIMPL_HXX

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
        typedef ::std::map< sal_Int32, css::uno::Reference< css::accessibility::XAccessible > >  THeaderCellMap;

        struct  THeaderCellMapFunctorDispose : ::std::unary_function<THeaderCellMap::value_type,void>
        {
            inline void operator()(const THeaderCellMap::value_type& _aType)
            {
                css::uno::Reference< css::lang::XComponent > xComp( _aType.second, css::uno::UNO_QUERY );
                OSL_ENSURE( xComp.is() || !_aType.second.is(), "THeaderCellMapFunctorDispose: invalid accessible cell (no XComponent)!" );
                if ( xComp.is() )
                    try
                    {
                        xComp->dispose();
                    }
                    catch( const css::uno::Exception& )
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
        css::uno::Reference< css::accessibility::XAccessible >
            getAccessibleHeaderBar( AccessibleBrowseBoxObjType _eObjType );

        /// @see AccessibleBrowseBox::getTable
        css::uno::Reference< css::accessibility::XAccessible >
            getAccessibleTable( );

    };
}

#endif // INCLUDED_SVTOOLS_SOURCE_BRWBOX_BRWIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
