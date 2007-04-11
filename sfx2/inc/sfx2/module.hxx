/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: module.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:24:13 $
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
#ifndef _SFXMODULE_HXX
#define _SFXMODULE_HXX  // intern
#define _SFXMOD_HXX     // extern

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SFX_SHELL_HXX
#include <sfx2/shell.hxx>
#endif

#ifndef _SFX_IMGDEF_HXX
#include <sfx2/imgdef.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
class ImageList;

class SfxAcceleratorManager;
class SfxBindings;
class SfxObjectFactory;
class ModalDialog;
class SfxObjectFactory;
class SfxModuleArr_Impl;
class SfxModule_Impl;
class SfxSlotPool;
struct SfxChildWinContextFactory;
struct SfxChildWinFactory;
struct SfxMenuCtrlFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SfxTbxCtrlFactArr_Impl;
class SfxChildWinFactArr_Impl;
class SfxMenuCtrlFactArr_Impl;
class SfxStbCtrlFactArr_Impl;
class SfxTabPage;
class Window;

//====================================================================

class SFX2_DLLPUBLIC SfxModule : public SfxShell
{
private:
    ResMgr*                     pResMgr;
    sal_Bool                    bDummy : 1;
    SfxModule_Impl*             pImpl;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void Construct_Impl();
//#endif

public:
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXMODULE)

                                SfxModule( ResMgr* pMgrP, sal_Bool bDummy,
                                    SfxObjectFactory* pFactoryP, ... );
                                ~SfxModule();

    ResMgr*                     GetResMgr() const { return SfxShell::GetResMgr(); }
    virtual ResMgr*             GetResMgr();
    SfxSlotPool*                GetSlotPool() const;

    void                        RegisterToolBoxControl(SfxTbxCtrlFactory*);
    void                        RegisterChildWindow(SfxChildWinFactory*);
    void                        RegisterChildWindowContext( sal_uInt16, SfxChildWinContextFactory* );
    void                        RegisterStatusBarControl(SfxStbCtrlFactory*);
    void                        RegisterMenuControl(SfxMenuCtrlFactory*);

    virtual SfxTabPage*         CreateTabPage( sal_uInt16 nId,
                                               Window* pParent,
                                               const SfxItemSet& rSet );
    virtual void                Invalidate(USHORT nId = 0);
    BOOL                        IsActive() const;

    static SfxModule*           GetActiveModule( SfxViewFrame* pFrame=NULL );

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE static SfxModuleArr_Impl& GetModules_Impl();
    SAL_DLLPRIVATE static void DestroyModules_Impl();
    SAL_DLLPRIVATE SfxTbxCtrlFactArr_Impl* GetTbxCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxStbCtrlFactArr_Impl* GetStbCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxMenuCtrlFactArr_Impl* GetMenuCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxChildWinFactArr_Impl* GetChildWinFactories_Impl() const;
    SAL_DLLPRIVATE ImageList* GetImageList_Impl( BOOL bBig );
    SAL_DLLPRIVATE ImageList* GetImageList_Impl( BOOL bBig, BOOL bHiContrast );
//#endif
};

#endif

