/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
                        OSL_ENSURE( sal_False, "THeaderCellMapFunctorDispose: caught an exception!" );
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
