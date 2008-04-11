/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dtint.hxx,v $
 * $Revision: 1.17 $
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
#ifndef _SV_DTINT_HXX
#define _SV_DTINT_HXX

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/color.hxx>
#include <vcl/font.hxx>

class SalBitmap;
class SalDisplay;
class AllSettings;

#ifndef _XLIB_H_
// forwards from X
struct Display;
struct XEvent;
#define Atom UINT32
#define XLIB_Window UINT32
#endif

enum DtType {
    DtGeneric,
    DtCDE,
    DtMACOSX
};

class DtIntegrator
{
protected:
    DtType              meType;
    Display*            mpDisplay;
    SalDisplay*         mpSalDisplay;
    int                 mnSystemLookCommandProcess;


    DtIntegrator();

    static String           aHomeDir;

public:
    static DtIntegrator* CreateDtIntegrator();

    virtual ~DtIntegrator();

    // SystemLook
    virtual void GetSystemLook( AllSettings& rSettings );

    DtType          GetDtType() { return meType; }
    SalDisplay*     GetSalDisplay() { return mpSalDisplay; }
    Display*        GetDisplay() { return mpDisplay; }
};

#endif
