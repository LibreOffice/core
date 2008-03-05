/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apptypes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:02:44 $
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

#ifndef _VCL_APPTYPES_HXX
#define _VCL_APPTYPES_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

// ---------------------
// - Application-Types -
// ---------------------

#define EXC_RSCNOTLOADED            ((USHORT)0x0100)
#define EXC_SYSOBJNOTCREATED        ((USHORT)0x0200)
#define EXC_SYSTEM                  ((USHORT)0x0300)
#define EXC_DISPLAY                 ((USHORT)0x0400)
#define EXC_REMOTE                  ((USHORT)0x0500)
#define EXC_USER                    ((USHORT)0x1000)
#define EXC_MAJORTYPE               ((USHORT)0xFF00)
#define EXC_MINORTYPE               ((USHORT)0x00FF)

#define UNIQUEID_SV_BEGIN           64000

class VCL_DLLPUBLIC ApplicationProperty
{
public:

    TYPEINFO();
};

#define INPUT_MOUSE                 0x0001
#define INPUT_KEYBOARD              0x0002
#define INPUT_PAINT                 0x0004
#define INPUT_TIMER                 0x0008
#define INPUT_OTHER                 0x0010
#define INPUT_APPEVENT              0x0020
#define INPUT_MOUSEANDKEYBOARD      (INPUT_MOUSE | INPUT_KEYBOARD)
#define INPUT_ANY                   (INPUT_MOUSEANDKEYBOARD | INPUT_PAINT | INPUT_TIMER | INPUT_OTHER | INPUT_APPEVENT)

#define DISPATCH_OPEN               0x0001
#define DISPATCH_PRINT              0x0002
#define DISPATCH_SERVER             0x0004

// --------------
// - UserActive -
// --------------

#define USERACTIVE_MOUSEDRAG            ((USHORT)0x0001)
#define USERACTIVE_INPUT                ((USHORT)0x0002)
#define USERACTIVE_MODALDIALOG          ((USHORT)0x0004)
#define USERACTIVE_ALL                  ((USHORT)0xFFFF)

#endif // _VCL_APPTYPES_HXX
