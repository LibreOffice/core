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

#ifndef INCLUDED_LOTUSWORDPRO_INC_LWPTOOLS_HXX
#define INCLUDED_LOTUSWORDPRO_INC_LWPTOOLS_HXX

#include <config_lgpl.h>

#include "lwpobjstrm.hxx"
#include "localtime.hxx"
#include "xfilter/xfdatestyle.hxx"
#include "xfilter/xftimestyle.hxx"
#include <o3tl/unit_conversion.hxx>
#include <rtl/textenc.h>
#include <stdexcept>
#include <string_view>

/**
 * @brief   tool class (unicode, conversion) for lwp filter.
*/
class LwpTools
{

public:
    static void QuickReadUnicode( LwpObjectStream* pObjStrm,
        OUString& str, sal_uInt16 strlen,  rtl_TextEncoding aEncoding );
    static bool IsUnicodePacked(LwpObjectStream* pObjStrm, sal_uInt16 len);

    inline static double ConvertFromUnits(double nUnits); // to cm
    inline static double ConvertFromTwips(double nTwips); // to cm

    inline static bool IsOddNumber(sal_uInt16 nNumber);
    inline static bool IsEvenNumber(sal_uInt16 nNumber);

    static bool isFileUrl(std::string_view fileName);
    static OUString convertToFileUrl(const OString& fileName);
    static OUString    DateTimeToOUString(const LtTm& dt);

    static std::unique_ptr<XFDateStyle> GetSystemDateStyle(bool bLongFormat);
    static std::unique_ptr<XFTimeStyle> GetSystemTimeStyle();
};

// Convert from "units" to centimeters
inline double LwpTools::ConvertFromUnits(double nUnits)
{
    constexpr sal_uInt32 UNITS_PER_INCH = 65536 * 72;
    constexpr auto mdFromIn = o3tl::getConversionMulDiv(o3tl::Length::in, o3tl::Length::cm);
    constexpr o3tl::detail::m_and_d md(mdFromIn.first, mdFromIn.second * UNITS_PER_INCH);
    return o3tl::convert(nUnits, md.m, md.d);
}
// Convert from twips to centimeters, using definition of point = 72.27 in
inline double LwpTools::ConvertFromTwips(double nTwips)
{
    constexpr auto mdFromIn = o3tl::getConversionMulDiv(o3tl::Length::in, o3tl::Length::cm);
    constexpr o3tl::detail::m_and_d md(mdFromIn.first * 100, mdFromIn.second * 7227 * 20);
    return o3tl::convert(nTwips, md.m, md.d);
}
inline bool LwpTools::IsOddNumber(sal_uInt16 nNumber)
{
    return (nNumber%2) != 0;
}
inline bool LwpTools::IsEvenNumber(sal_uInt16 nNumber)
{
    return (nNumber%2) == 0;
}

class BadSeek : public std::runtime_error
{
public:
    BadSeek() : std::runtime_error("Lotus Word Pro Bad Seek") { }
};

class BadRead: public std::runtime_error
{
public:
    BadRead() : std::runtime_error("Lotus Word Pro Bad Read") { }
};

class BadDecompress : public std::runtime_error
{
public:
    BadDecompress() : std::runtime_error("Lotus Word Pro Bad Decompress") { }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
