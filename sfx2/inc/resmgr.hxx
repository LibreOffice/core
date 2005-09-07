/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resmgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:13:44 $
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
#ifndef _SFX_RESMGR_HXX
#define _SFX_RESMGR_HXX

#include "app.hxx"
#include "minarray.hxx"

class SfxMessageDescription;
class SfxMessageTable;
class Bitmap;

DECL_PTRARRAY(SfxResMgrArr, ResMgr *, 1, 1);

// INCLUDE ---------------------------------------------------------------

class SfxResourceManager
{
    SfxResMgrArr                aResMgrArr;
    SfxResMgrArr                aResMgrBmpArr;
    USHORT                      nEnterCount;
    SfxMessageTable*            pMessageTable;

private:
    void                        ClearMsgTable_Impl();
    SfxMessageDescription*      MakeDesc_Impl(USHORT);

public:
                                SfxResourceManager();
                                ~SfxResourceManager();

    USHORT                      RegisterResource( const char *pFileName);
    void                        ReleaseResource( USHORT nRegisterId );

    USHORT                      RegisterBitmap(const char *pMono, const char *pColor);

    USHORT                      RegisterBitmap( const char *pSingleFile );
    void                        ReleaseBitmap( USHORT nRegisterId );

    Bitmap                      GetAllBitmap( USHORT nBmpsPerRow );

    void                        Enter();
    void                        Leave();
    SfxMessageDescription*      CreateDescription( USHORT nId );
};


#define SFX_RESMANAGER() SFX_APP()->GetResourceManager()

#endif

