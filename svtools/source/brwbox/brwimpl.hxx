/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brwimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:29:55 $
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

#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOX_HXX
#include "AccessibleBrowseBox.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXHEADERCELL_HXX
#include "AccessibleBrowseBoxHeaderCell.hxx"
#endif
#include <map>
#include <functional>

namespace svt
{
    class BrowseBoxImpl
    {
    // member
    public:
        typedef ::std::map< sal_Int32,AccessibleBrowseBoxHeaderCell*> THeaderCellMap;
        struct  THeaderCellMapFunctorDispose : ::std::unary_function<THeaderCellMap::value_type,void>
        {
            inline void operator()(const THeaderCellMap::value_type& _aType)
            {
                _aType.second->dispose();
            }
        };

    public:
        AccessibleBrowseBoxAccess*  m_pAccessible;
        THeaderCellMap              m_aColHeaderCellMap;
        THeaderCellMap              m_aRowHeaderCellMap;

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
