/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dtint.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:39:15 $
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
#ifndef _SV_DTINT_HXX
#define _SV_DTINT_HXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#include <font.hxx>

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

class DtIntegrator;

DECLARE_LIST( DtIntegratorList, DtIntegrator* );

enum DtType {
    DtGeneric,
    DtCDE,
    DtKDE,
    DtGNOME,
    DtSCO,
    DtIRIX
};

class DtIntegrator
{
protected:
    DtType              meType;
    Display*            mpDisplay;
    SalDisplay*         mpSalDisplay;
    int                 mnRefCount;
    int                 mnSystemLookCommandProcess;


    DtIntegrator();

    static DtIntegratorList aIntegratorList;
    static String           aHomeDir;

public:
    static DtIntegrator* CreateDtIntegrator();

    virtual ~DtIntegrator();

    // SystemLook
    virtual void GetSystemLook( AllSettings& rSettings );

    DtType          GetDtType() { return meType; }
    SalDisplay*     GetSalDisplay() { return mpSalDisplay; }
    Display*        GetDisplay() { return mpDisplay; }

    void Acquire() { mnRefCount++; }
    inline void Release();
};

inline void DtIntegrator::Release()
{
    mnRefCount--;
    if( ! mnRefCount )
    {
        aIntegratorList.Remove( this );
        delete this;
    }
}

#endif
