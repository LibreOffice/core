/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:17:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>

#include "saldata.hxx"
#include "salobj.h"

// =======================================================================

static long ImplSalObjectCallbackDummy( void*, SalObject*, USHORT, const void* )
{
    return 0;
}

// =======================================================================

AquaSalObject::AquaSalObject()
{
    SalData* pSalData = GetSalData();

    mpFrame         = NULL;
    mpInst          = NULL;
    mpProc          = ImplSalObjectCallbackDummy;

    // Insert object in objectlist
    mpNextObject = (AquaSalObject*)pSalData->mpFirstObject;
    pSalData->mpFirstObject = this;
}

// -----------------------------------------------------------------------

AquaSalObject::~AquaSalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
        pSalData->mpFirstObject = mpNextObject;
    else
    {
        AquaSalObject* pTempObject = (AquaSalObject*)pSalData->mpFirstObject;
        while ( pTempObject->mpNextObject != this )
            pTempObject = pTempObject->mpNextObject;
        pTempObject->mpNextObject = mpNextObject;
    }
}

// -----------------------------------------------------------------------

void AquaSalObject::ResetClipRegion()
{
}

// -----------------------------------------------------------------------

USHORT AquaSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void AquaSalObject::BeginSetClipRegion( ULONG nRectCount )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::EndSetClipRegion()
{
}

// -----------------------------------------------------------------------

void AquaSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::Show( BOOL bVisible )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::GrabFocus()
{
}

// -----------------------------------------------------------------------

void AquaSalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void AquaSalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* AquaSalObject::GetSystemData() const
{
    return NULL;
}

// -----------------------------------------------------------------------

void AquaSalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
    mpInst = pInst;
    if ( pProc )
        mpProc = pProc;
    else
        mpProc = ImplSalObjectCallbackDummy;
}
