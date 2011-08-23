/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_NUMBERSTYLESEXPORT_HXX
#define _XMLOFF_NUMBERSTYLESEXPORT_HXX

#include <sal/types.h>

#include "rtl/ustring.hxx"
namespace binfilter {

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

    static ::rtl::OUString getTimeStyleName(const sal_Int32 nTimeFormat );
    static ::rtl::OUString getDateStyleName(const sal_Int32 nDateFormat );
};

}//end of namespace binfilter
#endif	//  _XMLOFF_NUMBERSTYLESEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
