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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

#ifndef MATH_COMMAND_HXX
#include "command.hxx"
#endif
namespace binfilter {

SmCommandDesc::SmCommandDesc(const ResId& rResId) :
    Resource(rResId)
{
    if (IsAvailableRes(ResId(1).SetRT(RSC_STRING)))
        pSample = new String(ResId(1));
    else
        pSample = new String;

    if (IsAvailableRes(ResId(2).SetRT(RSC_STRING)))
        pCommand = new String(ResId(2));
    else
        pCommand = new String;

    if (IsAvailableRes(ResId(3).SetRT(RSC_STRING)))
        pText = new String(ResId(3));
    else
        pText = new String("sorry, no help available");

    if (IsAvailableRes(ResId(4).SetRT(RSC_STRING)))
        pHelp = new String(ResId(4));
    else
        pHelp = new String("sorry, no help available");

    if (IsAvailableRes(ResId(1).SetRT(RSC_BITMAP)))
        pGraphic =	new Bitmap(ResId(1));
    else
        pGraphic = new Bitmap();

    FreeResource();
}

SmCommandDesc::~SmCommandDesc()
{
    if (pText)		delete pText;
    if (pHelp)		delete pHelp;
    if (pSample)	delete pSample;
    if (pCommand)	delete pCommand;
    if (pGraphic)	delete pGraphic;
}


}
