/*************************************************************************
 *
 *  $RCSfile: imgmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:23 $
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

#include <imgdef.hxx>


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif

class ToolBox;

class SfxModule;
class SfxToolBoxManager;
class SfxBitmapList_Impl;
class SfxToolBoxArr_Impl;

class SfxImageManager: public SfxConfigItem
{
    SfxBitmapList_Impl* pUserDefList;
    ImageList*      pImageList;
    ImageList*      pUserImageList;
    ImageList*      pOffImageList;
    SfxSymbolSet    eSymbolSet;
    BOOL            bImageDefault;
    SfxToolBoxArr_Impl* pToolBoxList;
    USHORT          nOutStyle;

protected:

    virtual int     Load(SvStream&);
    virtual BOOL    Store(SvStream&);
    virtual void    UseDefault();
    virtual String  GetName() const;

#if __PRIVATE
    void            MakeDefaultImageList_Impl();
    void            MakeLists_Impl( SfxSymbolSet );
#endif

public:
                    SfxImageManager();
                    ~SfxImageManager();

                    // Allgemeine Properties
    void            SetSymbolSet(SfxSymbolSet);
    void            SetOutStyle(USHORT);

    SfxSymbolSet    GetSymbolSet() const
                            { return eSymbolSet; }
    USHORT          GetOutStyle() const
                            { return nOutStyle; }
    Size            GetImageSize() const
                            {return pImageList->GetImageSize(); }
    Color           GetMaskColor() const;

                    // Image(s) aus einem Modul oder der OFA-Liste
    Image           GetImage(USHORT nId, SfxModule* pMod = 0) const;
    void            SetImages( ToolBox& rToolBox, SfxModule* );
    void            LockImage(USHORT nNewId, ToolBox *pBox);

                    // Zugriff auf die Userdef-Liste
    void            ReplaceImage(USHORT nId, Bitmap* pBmp=0);
    void            AddImage(USHORT nId, const Image& rImage);

                    // Umkonfigurieren
    void            StartCustomize();
    void            EndCustomize();
    Image           SeekImage(USHORT nId, SfxModule* pModule = 0) const;

                    // Toolbox-Registrierung/Abmeldung
    void            RegisterToolBox(ToolBox*, USHORT nFlags=0xFFFF );
    void            RegisterToolBox(ToolBox*, SfxModule*, USHORT nFlags=0xFFFF );
    void            ReleaseToolBox(ToolBox*);
    void            RegisterToolBoxManager(SfxToolBoxManager*, USHORT nFlags=0xFFFF );
    void            ReleaseToolBoxManager(SfxToolBoxManager*);

#if __PRIVATE
    void            ExchangeItemImage_Impl(USHORT nId, const Image& rImage);
    BOOL            IsUserDef_Impl(USHORT nId) const;
    const Bitmap&   GetUserDefBitmap_Impl(USHORT nId) const;
    Image           GetAndLockImage_Impl(USHORT nId, SfxModule* pMod = 0);
    Image           GetImageFromModule_Impl( USHORT nId, SfxModule *pMod );
#endif

};

#define SFX_IMAGEMANAGER() SfxGetpApp()->GetImageManager()

#endif
