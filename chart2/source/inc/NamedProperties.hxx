/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:20:20 $
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
#ifndef CHART_NAMEDPROPERTIES_HXX
#define CHART_NAMEDPROPERTIES_HXX

#include "NamedLineProperties.hxx"
#include "NamedFillProperties.hxx"

#include "PropertyHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

class NamedProperties
{
    /** this class combines the classes NamedFillAttributes and NamedLineAttributes
    thus you can handle all named properties with one call if you like
    */
public:

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    //will return e.g. "FillGradientName" for nHandle == PROP_FILL_GRADIENT_NAME
    static ::rtl::OUString GetPropertyNameForHandle( sal_Int32 nHandle );

private:
    // not implemented
    NamedProperties();
};

} //  namespace chart

// CHART_NAMEDPROPERTIES_HXX
#endif
