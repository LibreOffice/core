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
#ifndef INCLUDED_SFX2_MODULE_HXX
#define INCLUDED_SFX2_MODULE_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/app.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/shell.hxx>
#include <sfx2/styfitem.hxx>
#include <svtools/imgdef.hxx>
#include <sal/types.h>
#include <tools/fldunit.hxx>
#include <com/sun/star/uno/Reference.hxx>

class SfxBindings;
class SfxObjectFactory;
class ModalDialog;
class SfxObjectFactory;
class SfxModule;
class SfxModule_Impl;
class SfxSlotPool;
struct SfxChildWinContextFactory;
struct SfxChildWinFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SfxTabPage;
namespace vcl { class Window; }

namespace com { namespace sun { namespace star { namespace frame {
    class XFrame;
} } } }


class SFX2_DLLPUBLIC SfxModule : public SfxShell
{
private:
    std::locale                 m_aResLocale;

    // Warning this cannot be turned into a unique_ptr.
    // SfxInterface destruction in the SfxSlotPool refers again to pImpl after deletion of pImpl has commenced. See tdf#100270
    SfxModule_Impl*             pImpl;

    SAL_DLLPRIVATE void Construct_Impl();

public:
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXMODULE)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl() {}

public:

    SfxModule(const std::locale& rResLocale, std::initializer_list<SfxObjectFactory*> pFactoryList);
    virtual ~SfxModule() override;

    const std::locale&          GetResLocale() const { return m_aResLocale; }
    SfxSlotPool*                GetSlotPool() const;

    void                        RegisterToolBoxControl(const SfxTbxCtrlFactory&);
    void                        RegisterChildWindow(SfxChildWinFactory*);
    void                        RegisterStatusBarControl(const SfxStbCtrlFactory&);

    virtual VclPtr<SfxTabPage>  CreateTabPage( sal_uInt16 nId,
                                               vcl::Window* pParent,
                                               const SfxItemSet& rSet );
    virtual void                Invalidate(sal_uInt16 nId = 0) override;

    virtual SfxStyleFamilies*   CreateStyleFamilies() { return nullptr; }

    static SfxModule*           GetActiveModule( SfxViewFrame* pFrame=nullptr );
    static FieldUnit            GetCurrentFieldUnit();
    /** retrieves the field unit of the module belonging to the document displayed in the given frame

        Effectively, this method looks up the SfxViewFrame belonging to the given XFrame, then the SfxModule belonging to
        the document in this frame, then this module's field unit.

        Failures in any of those steps are reported as assertion in non-product builds, and then FUNIT_100TH_MM is returned.
     */
    static FieldUnit            GetModuleFieldUnit( css::uno::Reference< css::frame::XFrame > const & i_frame );
    FieldUnit                   GetFieldUnit() const;

    SAL_DLLPRIVATE SfxTbxCtrlFactArr_Impl* GetTbxCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxStbCtrlFactArr_Impl* GetStbCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxChildWinFactArr_Impl* GetChildWinFactories_Impl() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
