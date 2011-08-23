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

#ifndef _HTMLOUT_HXX
#define _HTMLOUT_HXX

#include "bf_svtools/svtdllapi.h"

#include <tools/solar.h>

#include <rtl/textenc.h>
#include <bf_svtools/macitem.hxx>

class Color;
class String;
class SvStream;
class SvDataObject;

namespace binfilter
{
class ImageMap;

class SvxMacroTableDtor;
class SvNumberFormatter;

struct HTMLOutEvent
{
    const sal_Char *pBasicName;
    const sal_Char *pJavaName;
    USHORT nEvent;
};

struct HTMLOutFuncs
{
#if defined(MAC) || defined(UNX)
    static const sal_Char sNewLine;		// nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];	// \015\012
#endif
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
