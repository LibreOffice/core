/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFXMODULE_HXX
#define _SFXMODULE_HXX  // internal
#define _SFXMOD_HXX     // external

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sfx2/shell.hxx>
#include <sfx2/imgdef.hxx>
#include <sal/types.h>
#include <tools/fldunit.hxx>
#include <com/sun/star/uno/Reference.hxx>

class ImageList;

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

namespace com { namespace sun { namespace star { namespace frame {
    class XFrame;
} } } }
//====================================================================

class SFX2_DLLPUBLIC SfxModule : public SfxShell
{
private:
    ResMgr*                     pResMgr;
    sal_Bool                    bDummy : 1;
    SfxModule_Impl*             pImpl;

    SAL_DLLPRIVATE void Construct_Impl();

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
    virtual void                Invalidate(sal_uInt16 nId = 0);
    sal_Bool                        IsActive() const;

    /*virtual*/ bool            IsChildWindowAvailable( const sal_uInt16 i_nId, const SfxViewFrame* i_pViewFrame ) const;

    static SfxModule*           GetActiveModule( SfxViewFrame* pFrame=NULL );
    static FieldUnit            GetCurrentFieldUnit();
    /** retrieves the field unit of the module belonging to the document displayed in the given frame

        Effectively, this method looks up the SfxViewFrame belonging to the given XFrame, then the SfxModule belonging to
        the document in this frame, then this module's field unit.

        Failures in any of those steps are reported as assertion in non-product builds, and then FUNIT_100TH_MM is returned.
     */
    static FieldUnit            GetModuleFieldUnit( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > const & i_frame );
    FieldUnit                   GetFieldUnit() const;

    SAL_DLLPRIVATE static SfxModuleArr_Impl& GetModules_Impl();
    SAL_DLLPRIVATE static void DestroyModules_Impl();
    SAL_DLLPRIVATE SfxTbxCtrlFactArr_Impl* GetTbxCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxStbCtrlFactArr_Impl* GetStbCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxMenuCtrlFactArr_Impl* GetMenuCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxChildWinFactArr_Impl* GetChildWinFactories_Impl() const;
    SAL_DLLPRIVATE ImageList* GetImageList_Impl( sal_Bool bBig );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
