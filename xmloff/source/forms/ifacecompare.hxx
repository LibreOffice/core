/*************************************************************************
 *
 *  $RCSfile: ifacecompare.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-12-13 10:37:19 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_IFACECOMPARE_HXX_
#define _XMLOFF_FORMS_IFACECOMPARE_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OInterfaceCompare
    //=====================================================================
    /** is stl-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
    */
    template < class IAFCE >
    struct OInterfaceCompare
        :public ::std::binary_function  <   ::com::sun::star::uno::Reference< IAFCE >
                                        ,   ::com::sun::star::uno::Reference< IAFCE >
                                        ,   bool
                                        >
    {
        bool operator() (const ::com::sun::star::uno::Reference< IAFCE >& lhs, const ::com::sun::star::uno::Reference< IAFCE >& rhs) const
        {
            return lhs.get() < rhs.get();
                // this does not make any sense if you see the semantics of the pointer returned by get:
                // It's a pointer to a point in memory where an interface implementation lies.
                // But for our purpose (provide a reliable less-operator which can be used with the STL), this is
                // sufficient ....
        }
    };

    typedef OInterfaceCompare< ::com::sun::star::beans::XPropertySet >  OPropertySetCompare;
    typedef OInterfaceCompare< ::com::sun::star::drawing::XDrawPage >   ODrawPageCompare;

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_IFACECOMPARE_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 13.12.00 10:06:14  fs
 ************************************************************************/

