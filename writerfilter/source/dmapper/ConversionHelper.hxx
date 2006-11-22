/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConversionHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2006-11-22 14:03:57 $
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
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#define INCLUDED_DMAPPER_CONVERSIONHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace com{ namespace sun{ namespace star{
    namespace lang{
        struct Locale;
    }
    namespace table{
        struct BorderLine;
}}}}


namespace dmapper{
namespace ConversionHelper{

    // create a border line and return the distance value
    sal_Int32 MakeBorderLine( sal_Int32 nSprmValue, ::com::sun::star::table::BorderLine& rToFill );
    sal_Int32 MakeBorderLine( sal_Int32 nLineThickness,   sal_Int32 nLineType,
                              sal_Int32 nLineColor,           sal_Int32 nLineDistance,
                              ::com::sun::star::table::BorderLine& rToFill );
    //convert the number format string form MS format to SO format
    ::rtl::OUString ConvertMSFormatStringToSO(
            const ::rtl::OUString& rFormat, ::com::sun::star::lang::Locale& rLocale, bool bHijri);
    sal_Int32 convertToMM100(sal_Int32 _t);
    sal_Int32 ConvertColor(sal_Int32 nWordColor );

} // namespace ConversionHelper
} //namespace dmapper
#endif
