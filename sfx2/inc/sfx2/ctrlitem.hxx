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
#ifndef _SFXCTRLITEM_HXX
#define _SFXCTRLITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <svl/poolitem.hxx>

class SfxPoolItem;
class SfxBindings;
class SvStream;

class SFX2_DLLPUBLIC SfxControllerItem
{
private:
    sal_uInt16              nId;
    SfxControllerItem*  pNext; // zu benachrichtigendes weiteres ControllerItem
    SfxBindings*        pBindings;

protected:
//#if defined( DBG_UTIL ) && defined( _SOLAR__PRIVATE )
#if defined( DBG_UTIL )
    SAL_DLLPRIVATE void CheckConfigure_Impl( sal_uIntPtr nType );
#endif

public:
    SfxBindings &       GetBindings() {
                            DBG_ASSERT(pBindings, "keine Bindings");
                            return *pBindings;
                        }
    const SfxBindings & GetBindings() const {
                            DBG_ASSERT(pBindings, "keine Bindings");
                            return *pBindings;
                        }

                        SfxControllerItem(); // fuer arrays
                        SfxControllerItem( sal_uInt16 nId, SfxBindings & );
    virtual             ~SfxControllerItem();

    void                Bind( sal_uInt16 nNewId, SfxBindings * = 0);    // in SfxBindings registrieren
    void                UnBind();
    void                ReBind();
    sal_Bool                IsBound() const;
    void                UpdateSlot();
    void                ClearCache();
    void                SetBindings(SfxBindings &rBindings) { pBindings = &rBindings; }

    SfxControllerItem*  GetItemLink();
    SfxControllerItem*  ChangeItemLink( SfxControllerItem* pNewLink );

    void                SetId( sal_uInt16 nItemId );
    sal_uInt16              GetId() const { return nId; }

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual void        DeleteFloatingWindow();

    SfxMapUnit          GetCoreMetric() const;

    static SfxItemState GetItemState( const SfxPoolItem* pState );

    SAL_DLLPRIVATE sal_Bool IsBindable_Impl() const
                        { return pBindings != NULL; }
    SAL_DLLPRIVATE void BindInternal_Impl( sal_uInt16 nNewId, SfxBindings* );
};

//====================================================================

class SFX2_DLLPUBLIC SfxStatusForwarder: public SfxControllerItem
{
    SfxControllerItem*      pMaster;

protected:
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );

public:
                            SfxStatusForwarder( sal_uInt16 nSlotId,
                                   SfxControllerItem&rMaster );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
