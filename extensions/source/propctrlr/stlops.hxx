/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stlops.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:28:56 $
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

#ifndef _EXTENSIONS_PROPCTRLR_STLOPS_HXX_
#define _EXTENSIONS_PROPCTRLR_STLOPS_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    struct PropertyLessByName
                :public ::std::binary_function  <   ::com::sun::star::beans::Property,
                                                    ::com::sun::star::beans::Property,
                                                    bool
                                                >
    {
        bool operator() (::com::sun::star::beans::Property _rLhs, ::com::sun::star::beans::Property _rRhs) const
        {
            return _rLhs.Name < _rRhs.Name ? true : false;
        }
    };

    //========================================================================
    struct TypeLessByName
                :public ::std::binary_function  <   ::com::sun::star::uno::Type,
                                                    ::com::sun::star::uno::Type,
                                                    bool
                                                >
    {
        bool operator() (::com::sun::star::uno::Type _rLhs, ::com::sun::star::uno::Type _rRhs) const
        {
            return _rLhs.getTypeName() < _rRhs.getTypeName() ? true : false;
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_STLOPS_HXX_

