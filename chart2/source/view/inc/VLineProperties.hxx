/*************************************************************************
 *
 *  $RCSfile: VLineProperties.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 09:50:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
