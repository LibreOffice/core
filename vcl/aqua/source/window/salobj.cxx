/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-19 02:37:03 $
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

#include <string.h>

#define _SV_SALOBJ_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif

// =======================================================================

static long ImplSalObjectCallbackDummy( void*, SalObject*, USHORT, const void* )
{
    return 0;
}

// =======================================================================

SalObject::SalObject()
{
    SalData* pSalData = GetSalData();

    maObjectData.mhWnd              = 0;
    maObjectData.mhWndChild         = 0;
    maObjectData.mhLastFocusWnd     = 0;
    maObjectData.maSysData.nSize    = sizeof( SystemEnvData );
    maObjectData.mpInst             = NULL;
    maObjectData.mpProc             = ImplSalObjectCallbackDummy;

    // Insert object in objectlist
    maObjectData.mpNextObject = pSalData->mpFirstObject;
    pSalData->mpFirstObject = this;
}

// -----------------------------------------------------------------------

SalObject::~SalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
        pSalData->mpFirstObject = maObjectData.mpNextObject;
    else
    {
        SalObject* pTempObject = pSalData->mpFirstObject;
        while ( pTempObject->maObjectData.mpNextObject != this )
            pTempObject = pTempObject->maObjectData.mpNextObject;

        pTempObject->maObjectData.mpNextObject = maObjectData.mpNextObject;
    }
}

// -----------------------------------------------------------------------

void SalObject::ResetClipRegion()
{
}

// -----------------------------------------------------------------------

USHORT SalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void SalObject::BeginSetClipRegion( ULONG nRectCount )
{
}

// -----------------------------------------------------------------------

void SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void SalObject::EndSetClipRegion()
{
}

// -----------------------------------------------------------------------

void SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void SalObject::Show( BOOL bVisible )
{
}

// -----------------------------------------------------------------------

void SalObject::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void SalObject::GrabFocus()
{
}

// -----------------------------------------------------------------------

void SalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void SalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* SalObject::GetSystemData() const
{
}

// -----------------------------------------------------------------------

void SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
}
