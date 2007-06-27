/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brwimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:49:01 $
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
#ifndef _SVTOOLS_BRWIMPL_HXX
#define _SVTOOLS_BRWIMPL_HXX

#ifndef SVTOOLS_ACCESSIBLE_FACTORY_ACCESS_HXX
#include "svtaccessiblefactory.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

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
