/*************************************************************************
 *
 *  $RCSfile: dtint.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:35:52 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <tools/color.hxx>
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

    // executes pCommand and parses its output
    // to get system look information
    // different DtIntegrators can rely
    // on native programs to query system settings
    // pass NULL as command to read the VCL_SYSTEM_SETTINGS property
    void GetSystemLook( const char* pCommand, AllSettings& rSettings );
    bool StartSystemLookProcess( const char* pCommand );

    Color parseColor( const ByteString& );
    Font parseFont( const ByteString& );

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
