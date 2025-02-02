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
#pragma once

#include <memory>
#include <optional>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/shell.hxx>
#include <sfx2/styfitem.hxx>
#include <sal/types.h>
#include <tools/fldunit.hxx>
#include <com/sun/star/uno/Reference.hxx>

class SfxObjectFactory;
class SfxModule_Impl;
class SfxSlotPool;
struct SfxChildWinFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SfxTabPage;

namespace com::sun::star::frame {
    class XFrame;
}

namespace weld {
    class Container;
    class DialogController;
}

class SFX2_DLLPUBLIC SfxModule : public SfxShell
{
private:
    // Warning this cannot be turned into a unique_ptr.
    // SfxInterface destruction in the SfxSlotPool refers again to pImpl after deletion of pImpl has commenced. See tdf#100270
    SfxModule_Impl*             pImpl;

    SAL_DLLPRIVATE void Construct_Impl(const OString& rResName);

public:
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXMODULE)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl() {}

public:

    SfxModule(const OString& rResName, std::initializer_list<SfxObjectFactory*> pFactoryList);
    virtual ~SfxModule() override;

    std::locale                 GetResLocale() const;
    SfxSlotPool*                GetSlotPool() const;

    void                        RegisterToolBoxControl(const SfxTbxCtrlFactory&);
    void                        RegisterChildWindow(const SfxChildWinFactory&);
    void                        RegisterStatusBarControl(const SfxStbCtrlFactory&);

    virtual std::unique_ptr<SfxTabPage>  CreateTabPage( sal_uInt16 nId,
                                               weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet& rSet );
    virtual void                Invalidate(sal_uInt16 nId = 0) override;

    virtual SfxStyleFamilies CreateStyleFamilies() { return {}; }

    static SfxModule*           GetActiveModule( SfxViewFrame* pFrame=nullptr );
    static FieldUnit            GetCurrentFieldUnit();
    /** retrieves the field unit of the module belonging to the document displayed in the given frame

        Effectively, this method looks up the SfxViewFrame belonging to the given XFrame, then the SfxModule belonging to
        the document in this frame, then this module's field unit.

        Failures in any of those steps are reported as assertion in non-product builds, and then FieldUnit::MM_100TH is returned.
     */
    static FieldUnit            GetModuleFieldUnit( css::uno::Reference< css::frame::XFrame > const & i_frame );
    FieldUnit                   GetFieldUnit() const;

    SAL_DLLPRIVATE SfxTbxCtrlFactory* GetTbxCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const;
    SAL_DLLPRIVATE SfxStbCtrlFactory* GetStbCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const;
    SAL_DLLPRIVATE SfxChildWinFactory* GetChildWinFactoryById(sal_uInt16 nId) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
