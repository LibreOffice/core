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
#ifndef INCLUDED_SFX2_REQUEST_HXX
#define INCLUDED_SFX2_REQUEST_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <svl/itemset.hxx>
#include <svl/hint.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchRecorder.hpp>

class SfxPoolItem;
class SfxAllItemSet;
class SfxItemSet;
class SfxItemPool;
class SfxShell;
class SfxSlot;
class SfxViewFrame;
struct SfxRequest_Impl;
enum class SfxCallMode : sal_uInt16;



class SFX2_DLLPUBLIC SfxRequest: public SfxHint
{
friend struct SfxRequest_Impl;

    sal_uInt16          nSlot;
    SfxAllItemSet*      pArgs;
    SfxRequest_Impl*    pImp;


public:
    SAL_DLLPRIVATE void Record_Impl( SfxShell &rSh, const SfxSlot &rSlot,
                                     com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder,
                                     SfxViewFrame* );
private:
    SAL_DLLPRIVATE void Done_Impl( const SfxItemSet *pSet );



public:
                        SfxRequest( SfxViewFrame*, sal_uInt16 nSlotId );
                        SfxRequest( sal_uInt16 nSlot, SfxCallMode nCallMode, SfxItemPool &rPool );
                        SfxRequest( const SfxSlot* pSlot, const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rArgs,
                                            SfxCallMode nCallMode, SfxItemPool &rPool );
                        SfxRequest( sal_uInt16 nSlot, SfxCallMode nCallMode, const SfxAllItemSet& rSfxArgs );
                        SfxRequest( const SfxRequest& rOrig );
                        virtual ~SfxRequest();

    sal_uInt16              GetSlot() const { return nSlot; }
    void                SetSlot(sal_uInt16 nNewSlot) { nSlot = nNewSlot; }

    sal_uInt16              GetModifier() const;
    void                SetModifier( sal_uInt16 nModi );
    SAL_DLLPRIVATE void SetInternalArgs_Impl( const SfxAllItemSet& rArgs );
    SAL_DLLPRIVATE const SfxItemSet* GetInternalArgs_Impl() const;
    const SfxItemSet*   GetArgs() const { return pArgs; }
    void                SetArgs( const SfxAllItemSet& rArgs );
    void                AppendItem(const SfxPoolItem &);
    void                RemoveItem( sal_uInt16 nSlotId );

    static const SfxPoolItem* GetItem( const SfxItemSet*, sal_uInt16 nSlotId,
                                       bool bDeep = false,
                                       TypeId aType = 0 );
    const SfxPoolItem*  GetArg( sal_uInt16 nSlotId, bool bDeep = false, TypeId aType = 0 ) const;
    void                ReleaseArgs();
    void                SetReturnValue(const SfxPoolItem &);
    const SfxPoolItem*  GetReturnValue() const;

    static com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > GetMacroRecorder( SfxViewFrame* pFrame=NULL );
    static bool         HasMacroRecorder( SfxViewFrame* pFrame=NULL );
    SfxCallMode         GetCallMode() const;
    void                AllowRecording( bool );
    bool                AllowsRecording() const;
    bool                IsAPI() const;
    bool                IsSynchronCall() const;
    void                SetSynchronCall( bool bSynchron );

    bool                IsDone() const;
    void                Done( bool bRemove = false );

    void                Ignore();
    void                Cancel();
    bool                IsCancelled() const;
    void                Done(const SfxItemSet &, bool bKeep = true );

    void                ForgetAllArgs();

private:
    const SfxRequest&   operator=(const SfxRequest &); // n.i.!!
};



#define SFX_REQUEST_ARG(rReq, pItem, ItemType, nSlotId, bDeep) \
        const ItemType *pItem = static_cast<const ItemType*>( \
                rReq.GetArg( nSlotId, bDeep, TYPE(ItemType) ) )
#define SFX_ITEMSET_ARG(pArgs, pItem, ItemType, nSlotId, bDeep) \
    const ItemType *pItem = static_cast<const ItemType*>( \
        SfxRequest::GetItem( pArgs, nSlotId, bDeep, TYPE(ItemType) ) )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
