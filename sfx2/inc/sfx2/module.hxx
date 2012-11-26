/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFXMODULE_HXX
#define _SFXMODULE_HXX  // intern
#define _SFXMOD_HXX     // extern

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

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void Construct_Impl();
//#endif

public:
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

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE static SfxModuleArr_Impl& GetModules_Impl();
    SAL_DLLPRIVATE static void DestroyModules_Impl();
    SAL_DLLPRIVATE SfxTbxCtrlFactArr_Impl* GetTbxCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxStbCtrlFactArr_Impl* GetStbCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxMenuCtrlFactArr_Impl* GetMenuCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxChildWinFactArr_Impl* GetChildWinFactories_Impl() const;
    SAL_DLLPRIVATE ImageList* GetImageList_Impl( sal_Bool bBig );
    SAL_DLLPRIVATE ImageList* GetImageList_Impl( sal_Bool bBig, sal_Bool bHiContrast );
//#endif
};

#endif

