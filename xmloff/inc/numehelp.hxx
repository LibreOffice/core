/*************************************************************************
 *
 *  $RCSfile: numehelp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:01 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef XMLOFF_NUMEHELP_HXX
#define XMLOFF_NUMEHELP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

class SvXMLExport;
namespace rtl
{
    class OUString;
}

class XMLNumberFormatAttributesExportHelper
{
public :
    static sal_Bool GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& sCurrencySymbol,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const sal_Int32 nNumberFormat,
                                          const double& fValue,
                                          sal_uInt16 nNamespace,
                                          sal_Bool bExportValue = sal_True);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const rtl::OUString& sValue,
                                          const rtl::OUString& sCharacters,
                                          sal_uInt16 nNamespace,
                                          sal_Bool bExportValue = sal_True);
};

#endif

