/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imgmgr.hxx,v $
 * $Revision: 1.18 $
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

#ifndef _SFXIMGMGR_HXX
#define _SFXIMGMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sal/types.h>
#include <tools/gen.hxx>
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif

class ToolBox;
class SfxModule;
class SfxImageManager_Impl;
class SFX2_DLLPUBLIC SfxImageManager
{
    SfxImageManager_Impl* pImp;

public:
    static SfxImageManager*  GetImageManager( SfxModule* );

                    SfxImageManager( SfxModule* pModule = 0 );
                    ~SfxImageManager();

    void            RegisterToolBox( ToolBox *pBox, USHORT nFlags=0xFFFF);
    void            ReleaseToolBox( ToolBox *pBox );

                    // get images from resources
    void            SetImages( ToolBox& rToolBox );
    void            SetImages( ToolBox& rToolBox, BOOL bHiContrast, BOOL bLarge );
    void            SetImagesForceSize( ToolBox& rToolBox, BOOL bHiContrast, BOOL bLarge );

    Image           GetImage( USHORT nId, BOOL bLarge, BOOL bHiContrast ) const;
    Image           GetImage( USHORT nId, BOOL bHiContrast ) const;
    Image           SeekImage( USHORT nId, BOOL bLarge, BOOL bHiContrast ) const;
    Image           SeekImage( USHORT nId, BOOL bHiContrast ) const;
};

#endif
