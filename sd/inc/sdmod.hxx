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

#include "glob.hxx"
#include "pres.hxx"

#include <sot/storage.hxx>
#include "sddllapi.h"
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sal/types.h>
#include <vcl/virdev.hxx>
#include <o3tl/span.hxx>
#include <map>
#include <memory>
#include <string_view>

class SdOptions;
class SvxSearchItem;
class EditFieldInfo;
class SdTransferable;
class SvNumberFormatter;
class SfxErrorHandler;
class SfxFrame;
class VclSimpleEvent;
namespace svtools { class ColorConfig; }
enum class SdrObjKind : sal_uInt16;

namespace com::sun::star::frame {
    class XFrame;
}

enum class SdOptionStreamMode
{
    Load = 0,
    Store = 1
};

struct SdExtPropertySetInfoCacheCompare
{
    bool operator()(const o3tl::span<SfxItemPropertyMapEntry const>& lhs, const o3tl::span<SfxItemPropertyMapEntry const>& rhs) const
    {
        return lhs.data() < rhs.data();
    }
};
typedef std::map<
            o3tl::span<SfxItemPropertyMapEntry const>,
            css::uno::Reference<css::beans::XPropertySetInfo>,
            SdExtPropertySetInfoCacheCompare > SdExtPropertySetInfoCache;
typedef std::map< SdrObjKind, css::uno::Sequence< css::uno::Type> > SdTypesCache;

/*

  This subclass of <SfxModule> (which is a subclass of <SfxShell>) is
  linked to the DLL. One instance of this class exists while the DLL is
  loaded.

  SdModule is like to be compared with the <SfxApplication>-subclass.

  Remember: Don`t export this class! It uses DLL-internal symbols.

*/

class SAL_DLLPUBLIC_RTTI SdModule final : public SfxModule, public SfxListener
{
public:
                            SFX_DECL_INTERFACE(SD_IF_SDAPP)
                            DECL_DLLPRIVATE_LINK( CalcFieldValueHdl, EditFieldInfo*, void );

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                            SdModule(SfxObjectFactory* pDrawObjFact, SfxObjectFactory* pGraphicObjFact);
    virtual                 ~SdModule() override;

    SdTransferable*         pTransferClip;
    SdTransferable*         pTransferDrag;
    SdTransferable*         pTransferSelection;

    void                    Execute(SfxRequest& rReq);
    void                    GetState(SfxItemSet&);

    SdOptions*              GetSdOptions(DocumentType eDocType);
    SD_DLLPUBLIC rtl::Reference<SotStorageStream> GetOptionStream( std::u16string_view rOptionName, SdOptionStreamMode eMode );

    bool                    GetWaterCan() const { return bWaterCan; }
    void                    SetWaterCan( bool bWC ) { bWaterCan = bWC; }

    SvxSearchItem*          GetSearchItem() { return pSearchItem.get(); }
    void                    SetSearchItem(std::unique_ptr<SvxSearchItem> pItem);

    /** Return the virtual device that can be used for printer independent
        layout.
        @return
            The returned pointer is NULL when the device could not be
            created when this modules was instantiated.
    */
    OutputDevice* GetVirtualRefDevice() { return mpVirtualRefDevice;}

    SD_DLLPUBLIC SvNumberFormatter*     GetNumberFormatter();

    // virtual methods for the option dialog
    virtual std::optional<SfxItemSet> CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual std::unique_ptr<SfxTabPage> CreateTabPage( sal_uInt16 nId, weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet ) override;
    virtual std::optional<SfxStyleFamilies> CreateStyleFamilies() override;

    SdExtPropertySetInfoCache gImplImpressPropertySetInfoCache;
    SdExtPropertySetInfoCache gImplDrawPropertySetInfoCache;
    SdTypesCache gImplTypesCache;

    svtools::ColorConfig& GetColorConfig();

private:

    SdOptions*              pImpressOptions;
    SdOptions*              pDrawOptions;
    std::unique_ptr<SvxSearchItem>      pSearchItem;
    std::unique_ptr<SvNumberFormatter>  pNumberFormatter;
    rtl::Reference<SotStorage>          xOptionStorage;
    bool                    bWaterCan;
    std::unique_ptr<SfxErrorHandler> mpErrorHdl;
    /** This device is used for printer independent layout.  It is virtual
        in the sense that it does not represent a printer.  The pointer may
        be NULL when the virtual device could not be created.
    */
    VclPtr< VirtualDevice >  mpVirtualRefDevice;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    SfxFrame* ExecuteNewDocument( SfxRequest const & rReq );

    static SfxFrame* CreateEmptyDocument( const css::uno::Reference< css::frame::XFrame >& i_rFrame );
    static SfxFrame* CreateFromTemplate(const OUString& rTemplatePath,
                                        const css::uno::Reference<css::frame::XFrame>& i_rFrame,
                                        const bool bReplaceable);

    bool mbEventListenerAdded;

    /** Take an outline from a text document and create a new impress
        document according to the structure of the outline.
        @param rRequest
            This typically is the unmodified request from an execute()
            function from where this function is called.
    */
    static bool OutlineToImpress(SfxRequest const & rRequest);

    /** Add an eventlistener as soon as possible in sd, allows to use
        remote devices to start the slideshow elegantly, and respecting
        portability
        @EventListenerHdl
            The event listener handler
        @VclSimpleEvent *
            a pointer to a VCLSimpleEvent (see vcl/vclevent.hxx )
    */
    DECL_DLLPRIVATE_STATIC_LINK( SdModule, EventListenerHdl, VclSimpleEvent&, void );

    std::unique_ptr<svtools::ColorConfig> mpColorConfig;
};

#define SD_MOD() ( static_cast<SdModule*>(SfxApplication::GetModule(SfxToolsModule::Draw)) )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
