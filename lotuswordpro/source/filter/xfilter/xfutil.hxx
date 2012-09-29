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
 * Tool funtions for xml filter.
 * Don't use any template for the ***TOOUString function list for easy.
 ************************************************************************/
#ifndef     _XFUTIL_HXX
#define     _XFUTIL_HXX

#include    "xfdefs.hxx"
#include    <rtl/ustring.hxx>
#include    <string>


#define A2OUSTR(str)    rtl::OUString::createFromAscii(str)

rtl::OUString   Int32ToOUString(sal_Int32 num);

rtl::OUString   Int16ToOUString(sal_Int16 num);

rtl::OUString   FloatToOUString(float num, sal_Int32 precision=6);

rtl::OUString   DoubleToOUString(double num, sal_Int32 precision=6);

rtl::OUString   DateTimeToOUString(XFDateTime& dt);

rtl::OUString   GetTableColName(sal_Int32 col);

rtl::OUString   GetUnderlineName(enumXFUnderline type);

rtl::OUString   GetReliefName(enumXFRelief type);

rtl::OUString   GetCrossoutName(enumXFCrossout type);

rtl::OUString   GetTransformName(enumXFTransform type);

rtl::OUString   GetEmphasizeName(enumXFEmphasize type);

rtl::OUString   GetTextDirName(enumXFTextDir dir);

rtl::OUString   GetFrameXPos(enumXFFrameXPos pos);

rtl::OUString   GetFrameXRel(enumXFFrameXRel rel);

rtl::OUString   GetFrameYPos(enumXFFrameYPos pos);

rtl::OUString   GetFrameYRel(enumXFFrameYRel rel);

rtl::OUString   GetAlignName(enumXFAlignType align);

rtl::OUString   GetDrawKind(enumXFDrawKind kind);

rtl::OUString   GetPageUsageName(enumXFPageUsage usage);

rtl::OUString   GetValueType(enumXFValueType type);

rtl::OUString   GetColorMode(enumXFColorMode mode);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
