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
#ifndef INCLUDED_SFX2_CTRLITEM_HXX
#define INCLUDED_SFX2_CTRLITEM_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <svl/poolitem.hxx>
#include <tools/debug.hxx>

class SfxBindings;

class SFX2_DLLPUBLIC SfxControllerItem
{
private:
    sal_uInt16          nId;
    SfxControllerItem*  pNext; // to notify next ControllerItem
    SfxBindings*        pBindings;
    MapUnit             eFallbackCoreMetric;

public:
    SfxBindings &       GetBindings() {
                            DBG_ASSERT(pBindings, "no Bindings");
                            return *pBindings;
                        }
    const SfxBindings & GetBindings() const {
                            DBG_ASSERT(pBindings, "no Bindings");
                            return *pBindings;
                        }

                        SfxControllerItem(); // for arrays
                        SfxControllerItem( sal_uInt16 nId, SfxBindings & );
    virtual             ~SfxControllerItem();
    virtual void        dispose();

    void                Bind( sal_uInt16 nNewId, SfxBindings *);    // Register in SfxBindings
    void                UnBind();
    void                ReBind();
    bool                IsBound() const;
    void                ClearCache();

    SfxControllerItem*  GetItemLink();
    SfxControllerItem*  ChangeItemLink( SfxControllerItem* pNewLink );

    void                SetId( sal_uInt16 nItemId );
    sal_uInt16          GetId() const { return nId; }

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );

    virtual void        GetControlState( sal_uInt16 nSID, boost::property_tree::ptree& );

    MapUnit             GetCoreMetric() const;
    // override what GetCoreMetric will return if it cannot determine the current CoreMetric
    void                SetFallbackCodeMetric(MapUnit eFallback)
    {
        eFallbackCoreMetric = eFallback;
    }

    static SfxItemState GetItemState( const SfxPoolItem* pState );

    SAL_DLLPRIVATE void BindInternal_Impl( sal_uInt16 nNewId, SfxBindings* );
};


class SFX2_DLLPUBLIC SfxStatusForwarder final : public SfxControllerItem
{
    SfxControllerItem*      pMaster;

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState ) override;

public:
                            SfxStatusForwarder( sal_uInt16 nSlotId,
                                   SfxControllerItem&rMaster );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
