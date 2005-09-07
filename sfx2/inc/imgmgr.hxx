/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgmgr.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:58:13 $
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

#ifndef _SFXIMGMGR_HXX
#define _SFXIMGMGR_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#include <sal/types.h>

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
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
    static sal_Int16         GetCurrentSymbolSet();
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
