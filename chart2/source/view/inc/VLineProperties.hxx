/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VLineProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:45:48 $
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

#ifndef _CHART2_VLINEPROPERTIES_HXX
#define _CHART2_VLINEPROPERTIES_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct VLineProperties
{
    com::sun::star::uno::Any Color; //type sal_Int32 UNO_NAME_LINECOLOR
    com::sun::star::uno::Any LineStyle; //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    com::sun::star::uno::Any Transparence;//type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    com::sun::star::uno::Any Width;//type sal_Int32 for property UNO_NAME_LINEWIDTH
    com::sun::star::uno::Any Dash;//type drawing::LineDash for property UNO_NAME_LINEDASH

    VLineProperties();
    void initFromPropertySet( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >& xProp
                              , bool bUseSeriesPropertyNames=false );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
