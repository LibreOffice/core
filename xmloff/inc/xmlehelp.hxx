/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlehelp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:13:24 $
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

#ifndef _XMLOFF_XMLEHELP_HXX
#define _XMLOFF_XMLEHELP_HXX

#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

namespace rtl { class OUStringBuffer; }

class SvXMLExportHelper
{
public:
    static void AddLength( long nValue, MapUnit eValueUnit,
                           ::rtl::OUStringBuffer& rOut,
                           MapUnit eOutUnit );
    static void AddPercentage( long nVal, ::rtl::OUStringBuffer& rOut );
    static double GetConversionFactor(::rtl::OUStringBuffer& rUnit,
        const MapUnit eCoreUnit, const MapUnit eDestUnit);
    static MapUnit GetUnitFromString(const ::rtl::OUString& rString,
        MapUnit eDefaultUnit);
};



#endif  //  _XMLOFF_XMLEHELP_HXX

