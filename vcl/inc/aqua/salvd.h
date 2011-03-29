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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

#include "aqua/salconst.h"
#include "aqua/salcolorutils.hxx"
#include "aqua/salgdi.h"

#include "salvd.hxx"

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
    CGContextRef mxBitmapContext;
    int mnBitmapDepth;
    CGLayerRef mxLayer;              // Quartz layer
    AquaSalGraphics* mpGraphics;     // current VirDev graphics

    void Destroy();

public:
    AquaSalVirtualDevice( AquaSalGraphics* pGraphic, long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData );
    virtual ~AquaSalVirtualDevice();

    virtual SalGraphics*            GetGraphics();
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                    SetSize( long nNewDX, long nNewDY );
    virtual void                       GetSize( long& rWidth, long& rHeight );
};

// =======================================================================

#endif // _SV_SALVD_H
