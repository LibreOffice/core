/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_FMMODEL_HXX
#define INCLUDED_SVX_FMMODEL_HXX

#include <svx/svdmodel.hxx>
#include <svx/svxdllapi.h>
#include <memory>

class SfxObjectShell;
class SfxItemPool;
class FmXUndoEnvironment;

struct FmFormModelImplData;
class SVXCORE_DLLPUBLIC FmFormModel :
    public SdrModel
{
private:
    std::unique_ptr<FmFormModelImplData>    m_pImpl;
    SfxObjectShell*         m_pObjShell;

    bool            m_bOpenInDesignMode : 1;
    bool            m_bAutoControlFocus : 1;

    FmFormModel( const FmFormModel& ) = delete;
    void operator=(const FmFormModel& rSrcModel) = delete;

public:
    FmFormModel(
        SfxItemPool* pPool = nullptr,
        SfxObjectShell* pPers = nullptr);

    virtual ~FmFormModel() override;

    virtual rtl::Reference<SdrPage> AllocPage(bool bMasterPage) override;
    virtual void     InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF) override;
    virtual rtl::Reference<SdrPage> RemovePage(sal_uInt16 nPgNum) override;
    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF) override;
    virtual rtl::Reference<SdrPage> RemoveMasterPage(sal_uInt16 nPgNum) override;

    SfxObjectShell*         GetObjectShell() const { return m_pObjShell; }
    void                    SetObjectShell( SfxObjectShell* pShell );

    bool GetOpenInDesignMode() const { return m_bOpenInDesignMode; }
    void SetOpenInDesignMode( bool _bOpenDesignMode );

    bool    GetAutoControlFocus() const { return m_bAutoControlFocus; }
    void    SetAutoControlFocus( bool _bAutoControlFocus );

    /** check whether the OpenInDesignMode has been set explicitly or been loaded (<FALSE/>)
        or if it still has the default value from construction (<TRUE/>)
    */
    bool    OpenInDesignModeIsDefaulted();

    /** determines whether form controls should use the SdrModel's reference device for text rendering
    */
    bool    ControlsUseRefDevice() const;

    FmXUndoEnvironment& GetUndoEnv();

private:
    void        implSetOpenInDesignMode( bool _bOpenDesignMode );
};

#endif          // _FM_FMMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
