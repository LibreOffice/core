/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormattedStringHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:17:03 $
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
#ifndef _CHART2_TOOLS_FORMATTEDSTRINGHELPER_HXX
#define _CHART2_TOOLS_FORMATTEDSTRINGHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XFORMATTEDSTRING_HPP_
#include <com/sun/star/chart2/XFormattedString.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class FormattedStringHelper
{
public:
    static ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XFormattedString > >
            createFormattedStringSequence(
                     const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext > & xContext
                    , const ::rtl::OUString & rString
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > & xTextProperties ) throw();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
