/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:46:32 $
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

    maObjectData.mpFrame            = NULL;
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
    return NULL;
}

// -----------------------------------------------------------------------

void SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
    maObjectData.mpInst = pInst;
    if ( pProc )
        maObjectData.mpProc = pProc;
    else
        maObjectData.mpProc = ImplSalObjectCallbackDummy;
}
