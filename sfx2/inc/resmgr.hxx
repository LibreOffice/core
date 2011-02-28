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
#ifndef _SFX_RESMGR_HXX
#define _SFX_RESMGR_HXX

#include <sfx2/minarray.hxx>

class SfxMessageDescription;
class SfxMessageTable;
class Bitmap;
class ResMgr;

DECL_PTRARRAY(SfxResMgrArr, ResMgr *, 1, 1);

// INCLUDE ---------------------------------------------------------------

class SfxResourceManager
{
    SfxResMgrArr                aResMgrArr;
    SfxResMgrArr                aResMgrBmpArr;
    sal_uInt16                      nEnterCount;
    SfxMessageTable*            pMessageTable;

private:
    void                        ClearMsgTable_Impl();
    SfxMessageDescription*      MakeDesc_Impl(sal_uInt16);

public:
                                SfxResourceManager();
                                ~SfxResourceManager();

    sal_uInt16                      RegisterResource( const char *pFileName);
    void                        ReleaseResource( sal_uInt16 nRegisterId );

    sal_uInt16                      RegisterBitmap(const char *pMono, const char *pColor);

    sal_uInt16                      RegisterBitmap( const char *pSingleFile );
    void                        ReleaseBitmap( sal_uInt16 nRegisterId );

    Bitmap                      GetAllBitmap( sal_uInt16 nBmpsPerRow );

    void                        Enter();
    void                        Leave();
    SfxMessageDescription*      CreateDescription( sal_uInt16 nId );
};


#define SFX_RESMANAGER() SFX_APP()->GetResourceManager()

#endif

