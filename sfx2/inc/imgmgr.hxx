/*************************************************************************
 *
 *  $RCSfile: imgmgr.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:31:38 $
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
#ifndef _SFXIMGMGR_HXX
#define _SFXIMGMGR_HXX

#include "cfgitem.hxx"

#include "imgdef.hxx"

#include <sal/types.h>
#include <tools/link.hxx>

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif

class ToolBox;
class SfxObjectShell;
class SfxModule;
class SfxToolBoxManager;
class SfxImageManager_Impl;
struct SfxImageManagerData_Impl;
class SfxImageManager
{
    SfxImageManager_Impl*       pImp;
    SfxImageManagerData_Impl*   pData;
    void*                       pHCImp; // internal use

    void            SetSymbolSet_Impl( sal_Int16 );
    void            SetOutStyle_Impl( sal_Int16 );
    void            ExchangeItemImage_Impl( USHORT nId, const Image& rImage );

    DECL_LINK(      OptionsChanged_Impl, void* );
    DECL_LINK(      ConfigChanged_Impl, void* );
    DECL_LINK(      SettingsChanged_Impl, void* );

public:
    static BOOL         Import( SvStream& rInStream, SotStorage& rOutStorage );
    static BOOL         Export( SotStorage& rInStorage, SvStream& rOutStream );
    static BOOL         Copy( SotStorage& rIn, SotStorage& rOut );
    static Image        GetGlobalImage( USHORT nId, BOOL bBig );
    static Image        GetGlobalImage( USHORT nId, BOOL bBig, BOOL bHiContrast );
    static ImageList*   GetGlobalDefaultImageList( BOOL bBig, BOOL bHiContrast );
    static sal_Int16    GetCurrentSymbolSet();

                    // each document may have its own imagemanager, but all documents without an own
                    // image configuration share the same instance
                    SfxImageManager( SfxObjectShell* pDoc );
                    ~SfxImageManager();

    SfxConfigManager* GetConfigManager();
    USHORT            GetType() const;

    Size            GetImageSize() const;
    Color           GetMaskColor() const;

                    // get images from resources
    Image           GetImage(USHORT nId, SfxModule* pMod = 0 ) const;
    Image           GetImage(USHORT nId, BOOL bHiContrast, SfxModule* pMod = 0 ) const;
    void            SetImages( ToolBox& rToolBox, SfxModule* );
    void            SetImages( ToolBox& rToolBox, SfxModule*, BOOL bHiContrast );
    void            LockImage(USHORT nNewId, ToolBox *pBox );
    void            LockImage(USHORT nNewId, ToolBox *pBox, BOOL bHiContrast );
    Image           MakeUserImage(USHORT nNewId, Image& aSourceImage, BOOL bHiContrast = FALSE );
    Image           GetImage(USHORT nId, SfxModule* pMod, BOOL bBig ) const;
    Image           GetImage(USHORT nId, SfxModule* pMod, BOOL bBig, BOOL bHiContrast ) const;
    Image           GetDefaultImage(USHORT nId, SfxModule* pMod, BOOL bBig, BOOL bHiContrast ) const;

                    // add images to configurable user list
    void            ReplaceImage(USHORT nId, Bitmap* pBmp=0);
    void            AddImage(USHORT nId, const Image& rImage);

                    // reconfigure user list
    void            StartCustomize();
    void            EndCustomize();
    Image           SeekImage(USHORT nId, SfxModule* pModule = 0 ) const;
    Image           SeekImage(USHORT nId, BOOL bHiContrast, SfxModule* pModule = 0 ) const;

                    // register/release toolboxes
    void            RegisterToolBox(ToolBox*, USHORT nFlags=0xFFFF );
    void            RegisterToolBox(ToolBox*, SfxModule*, USHORT nFlags=0xFFFF );
    void            ReleaseToolBox(ToolBox*);
    void            RegisterToolBoxManager(SfxToolBoxManager*, USHORT nFlags=0xFFFF );
    void            ReleaseToolBoxManager(SfxToolBoxManager*);

#if _SOLAR__PRIVATE
    BOOL            IsUserDef_Impl(USHORT nId) const;
    const Bitmap&   GetUserDefBitmap_Impl(USHORT nId) const;
    Image           GetAndLockImage_Impl(USHORT nId, SfxModule* pMod = 0 );
    Image           GetAndLockImage_Impl(USHORT nId, BOOL bHiContrast, SfxModule* pMod = 0 );
    Image           GetImageFromModule_Impl( USHORT nId, SfxModule *pMod );
    Image           GetImageFromModule_Impl( USHORT nId, SfxModule *pMod, BOOL bHiContrast );
    static BOOL     CopyConfiguration_Impl( SfxConfigManager& rSource, SfxConfigManager& rDest );
#endif
};

#endif
