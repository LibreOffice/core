/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <unordered_map>

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <vcl/builderpage.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/setitem.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace com::sun::star::frame { class XFrame; }

enum class DeactivateRC {
    KeepPage   = 0x00,      // Error handling; page does not change
    // 2. Fill an itemset for update
    // parent examples, this pointer can be NULL all the time!
    LeavePage  = 0x01,
    // Set, refresh and update other Page
    RefreshSet = 0x02
};
namespace o3tl {
template<> struct typed_flags<DeactivateRC> : is_typed_flags<DeactivateRC, 0x03> {};
}

class SFX2_DLLPUBLIC SfxTabPage : public BuilderPage
{
    friend class SfxTabDialog;
    friend class SfxTabDialogController;

private:
    const SfxItemSet* mpSet;
    OUString maUserString;
    bool mbHasExchangeSupport;
    bool mbCancel;
    std::unordered_map<OUString, css::uno::Any> maAdditionalProperties;

    bool mbStandard;
    SfxOkDialogController* mpSfxDialogController;
    css::uno::Reference<css::frame::XFrame> mxFrame;

protected:
    SfxTabPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OUString& rID, const SfxItemSet *rAttrSet);

    sal_uInt16          GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const
    {
        return mpSet->GetPool()->GetWhichIDFromSlotID(nSlot, bDeep);
    }
    template<class T>
    TypedWhichId<T> GetWhich( TypedWhichId<T> nSlot, bool bDeep = true ) const
    {
        return TypedWhichId<T>(GetWhich(sal_uInt16(nSlot), bDeep));
    }

    const SfxPoolItem*  GetOldItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    template<class T> const T* GetOldItem( const SfxItemSet& rSet, TypedWhichId<T> nSlot, bool bDeep = true )
    {
        return static_cast<const T*>(GetOldItem(rSet, sal_uInt16(nSlot), bDeep));
    }

    SfxOkDialogController* GetDialogController() const;
public:
    virtual             ~SfxTabPage() override;

    void set_visible(bool bVisible)
    {
        m_xContainer->set_visible(bVisible);
    }

    const SfxItemSet& GetItemSet() const
    {
        return *mpSet;
    }

    virtual bool        FillItemSet( SfxItemSet* );
    virtual void        Reset( const SfxItemSet* );
    // Allows to postpone some initialization to the first activation
    virtual bool        DeferResetToFirstActivation();

    bool HasExchangeSupport() const
    {
        return mbHasExchangeSupport;
    }

    void SetExchangeSupport()
    {
        mbHasExchangeSupport = true;
    }

    virtual void            ActivatePage( const SfxItemSet& );
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet );
    void SetUserData(const OUString& rString)
    {
        maUserString = rString;
    }
    const OUString& GetUserData() const
    {
        return maUserString;
    }
    virtual void            FillUserData();
    virtual bool            IsReadOnly() const;
    // Whether the user has canceled the dialog. Allows to restore settings, etc.
    bool IsCancelMode() { return mbCancel; }
    void SetCancelMode(bool bCancel) { mbCancel = bCancel; }
    virtual void PageCreated (const SfxAllItemSet& aSet);
    virtual void ChangesApplied();
    static const SfxPoolItem* GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    template<class T> static const T* GetItem( const SfxItemSet& rSet, TypedWhichId<T> nSlot, bool bDeep = true )
    {
        return static_cast<const T*>(GetItem(rSet, sal_uInt16(nSlot), bDeep));
    }

    virtual OUString GetAllStrings();
    void SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
    css::uno::Reference< css::frame::XFrame > GetFrame() const;

    const SfxItemSet* GetDialogExampleSet() const;

    OUString        GetHelpId() const;
    OUString        GetConfigId() const { return GetHelpId(); }
    bool            IsVisible() const { return m_xContainer->get_visible(); }

    weld::Window*   GetFrameWeld() const;

    std::unordered_map<OUString, css::uno::Any>& getAdditionalProperties()
    {
        return maAdditionalProperties;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
