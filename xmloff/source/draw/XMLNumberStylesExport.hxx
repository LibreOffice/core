/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLNumberStylesExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:41:19 $
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

#ifndef _XMLOFF_NUMBERSTYLESEXPORT_HXX
#define _XMLOFF_NUMBERSTYLESEXPORT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

class SdXMLExport;

const sal_Int16 SdXMLDateFormatCount = 8;
const sal_Int16 SdXMLTimeFormatCount = 7;

class SdXMLNumberStylesExporter
{
public:
    static void exportTimeStyle( SdXMLExport& rExport, sal_Int32 nStyle );
    static void exportDateStyle( SdXMLExport& rExport, sal_Int32 nStyle );

    static sal_Int32 getDateStyleCount() { return SdXMLDateFormatCount; }
    static sal_Int32 getTimeStyleCount() { return SdXMLTimeFormatCount; }

    static rtl::OUString getTimeStyleName(const sal_Int32 nTimeFormat );
    static rtl::OUString getDateStyleName(const sal_Int32 nDateFormat );
};

#endif  //  _XMLOFF_NUMBERSTYLESEXPORT_HXX

