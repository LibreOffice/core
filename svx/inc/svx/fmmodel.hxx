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



#ifndef _SVX_FMMODEL_HXX
#define _SVX_FMMODEL_HXX

#include <svx/svdmodel.hxx>
#include "svx/svxdllapi.h"

class SfxObjectShell;
class SfxItemPool;
class VCItemPool;
class FmXUndoEnvironment;
class SfxObjectShell;
class SbxObject;
class SbxArray;
class SbxValue;

struct FmFormModelImplData;
class SVX_DLLPUBLIC FmFormModel :
    public SdrModel
{
private:
    FmFormModelImplData*    m_pImpl;
    SfxObjectShell*         m_pObjShell;

    sal_Bool            m_bOpenInDesignMode : 1;
    sal_Bool            m_bAutoControlFocus : 1;

    SVX_DLLPRIVATE FmFormModel( const FmFormModel& );   // never implemented
    SVX_DLLPRIVATE void operator=(const FmFormModel& rSrcModel);   // never implemented

public:
    FmFormModel(const XubString& rPath = XubString(), SfxItemPool* pPool = 0, SfxObjectShell* pPers = 0, bool bUseExtColorTable = 0);
    virtual ~FmFormModel();

    virtual SdrPage* AllocPage(bool bMasterPage);
    virtual void     InsertPage(SdrPage* pPage, sal_uInt32 nPos=0xffffffff);
    virtual SdrPage* RemovePage(sal_uInt32 nPgNum);
    virtual void     MovePage(sal_uInt32 nPgNum, sal_uInt32 nNewPos);
    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos=0xffffffff);
    virtual SdrPage* RemoveMasterPage(sal_uInt32 nPgNum);

    virtual SdrLayerID      GetControlExportLayerId( const SdrObject& rObj ) const;
    SfxObjectShell*         GetObjectShell() const { return m_pObjShell; }
    void                    SetObjectShell( SfxObjectShell* pShell );

    sal_Bool GetOpenInDesignMode() const { return m_bOpenInDesignMode; }
    void SetOpenInDesignMode( sal_Bool _bOpenDesignMode );

    sal_Bool    GetAutoControlFocus() const { return m_bAutoControlFocus; }
    void        SetAutoControlFocus( sal_Bool _bAutoControlFocus );

    /** check whether the OpenInDesignMode has been set explicitly or been loaded (<FALSE/>)
        or if it still has the default value from construction (<TRUE/>)
    */
    sal_Bool    OpenInDesignModeIsDefaulted();

    /** determines whether form controls should use the SdrModel's reference device for text rendering
    */
    sal_Bool    ControlsUseRefDevice() const;

    FmXUndoEnvironment& GetUndoEnv();

private:
    void        implSetOpenInDesignMode( sal_Bool _bOpenDesignMode, sal_Bool _bForce );
};

#endif          // _FM_FMMODEL_HXX

