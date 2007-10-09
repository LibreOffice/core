/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.h,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:12:17 $
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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

#include "vcl/sv.h"
#include "vcl/salgdi.hxx"
#include "salconst.h"
#include "salcolorutils.hxx"
#include "vcl/salvd.hxx"
#include "salgdi.h"

#if PRAGMA_ONCE
    #pragma once
#endif

// =======================================================================

// =======================================================================

// -----------------
// - SalVirDevData -
// -----------------

struct SalVirDevData
{
};

typedef struct SalVirDevData   SalVirDevData;
typedef SalVirDevData         *SalVirDevDataPtr;
typedef SalVirDevDataPtr      *SalVirDevDataHandle;

// =======================================================================

class AquaSalGraphics;

// -----------------
// - SalVirDevData -
// -----------------

class AquaSalVirtualDevice : public SalVirtualDevice
{
private:
    bool mbGraphicsUsed;             // is Graphics used
    bool mbForeignContext;           // is mxContext from outside VCL
    CGContextRef mxContext;          // native graphics context
    AquaSalGraphics* mpGraphics;     // current VirDev graphics

    void Destroy();

public:
    AquaSalVirtualDevice( AquaSalGraphics* pGraphic, long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData );
    virtual ~AquaSalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                    SetSize( long nNewDX, long nNewDY );
    virtual void                       GetSize( long& rWidth, long& rHeight );
};

// =======================================================================

#endif // _SV_SALVD_H
