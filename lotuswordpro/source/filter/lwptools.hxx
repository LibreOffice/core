/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  to provide basic utilities for word pro filter
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPTOOLS_HXX
#define _LWPTOOLS_HXX

#include "lwpheader.hxx"
#include "lwpobjstrm.hxx"
#include "localtime.hxx"
#include "xfilter/xfdatestyle.hxx"
#include "xfilter/xftimestyle.hxx"
#include <rtl/textenc.h>

// 01/19/2005
const sal_uInt32 UNITS_PER_INCH = 65536L * 72L;
const double CM_PER_INCH = 2.54;
const double INCHT_PER_CM = (1.0 / CM_PER_INCH);
//end

const double POINTS_PER_INCH = 72.27;
const double TWIPS_PER_POINT = 20.0;
const double TWIPS_PER_INCH = (TWIPS_PER_POINT * POINTS_PER_INCH);
const double TWIPS_PER_CM = (TWIPS_PER_INCH/CM_PER_INCH);
/**
 * @brief	tool class (unicode, conversion) for lwp filter.
*/
class LwpTools
{

public:
    static sal_uInt16 QuickReadUnicode( LwpObjectStream* pObjStrm,
        OUString& str, sal_uInt16 strlen,  rtl_TextEncoding aEncoding );
    static BOOL IsUnicodePacked(LwpObjectStream* pObjStrm, sal_uInt16 len);

    // 01/19/2005
    inline static double ConvertFromUnits(const sal_Int32& nUnits);
    inline static double ConvertToMetric(const double& fInch);
    inline static double ConvertFromMetric(const double& fCM);
    inline static double ConvertFromUnitsToMetric(const sal_Int32& nUnits);
    //end

    //add by , 03/11/2005
    inline static sal_Int32 ConvertToUnits(const double& fInch);
    //add end

    //, 02/23/2005
    inline static sal_Bool IsOddNumber(sal_uInt16& nNumber);
    inline static sal_Bool IsEvenNumber(sal_uInt16& nNumber);

    static sal_Bool isFileUrl(const OString& fileName);
    static OUString convertToFileUrl(const OString& fileName);
    static rtl::OUString	DateTimeToOUString(LtTm& dt);

    //add by ,2005/6/1
    static XFDateStyle* GetSystemDateStyle(sal_Bool bLongFormat);
    static XFTimeStyle* GetSystemTimeStyle();
};

inline double LwpTools::ConvertFromUnits(const sal_Int32& nUnits)
{
    return (double)nUnits/UNITS_PER_INCH;
}
inline double LwpTools::ConvertToMetric(const double& fInch)
{
    return fInch*CM_PER_INCH;
}
inline double LwpTools::ConvertFromMetric(const double& fCM)
{
    return fCM/CM_PER_INCH;
}
inline double LwpTools::ConvertFromUnitsToMetric(const sal_Int32& nUnits)
{
    double fInch = ConvertFromUnits(nUnits);
    return ConvertToMetric(fInch);
}
inline sal_Int32 LwpTools::ConvertToUnits(const double& fInch)
{
    return (sal_Int32)fInch*UNITS_PER_INCH;
}
inline sal_Bool LwpTools::IsOddNumber(sal_uInt16& nNumber)
{
    return nNumber%2? sal_True : sal_False;
}
inline sal_Bool LwpTools::IsEvenNumber(sal_uInt16& nNumber)
{
    return nNumber%2? sal_False : sal_True;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
