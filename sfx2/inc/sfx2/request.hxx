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
#ifndef _SFXREQUEST_HXX
#define _SFXREQUEST_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <svl/itemset.hxx>
#include <svl/hint.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchRecorder.hpp>

class SfxMacro;
class SfxPoolItem;
class SfxAllItemSet;
class SfxItemSet;
class SfxItemPool;
class SfxShell;
class SfxSlot;
class SfxMacroStatement;
class SfxArguments;
class String;
class SfxViewFrame;
struct SfxRequest_Impl;

//==================================================================

class SFX2_DLLPUBLIC SfxRequest: public SfxHint
{
friend struct SfxRequest_Impl;

    sal_uInt16              nSlot;
    SfxAllItemSet*      pArgs;
    SfxRequest_Impl*    pImp;

    //---------------------------------------------------------------------
//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE void Record_Impl( SfxShell &rSh, const SfxSlot &rSlot,
                                     com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder,
                                     SfxViewFrame* );
private:
    SAL_DLLPRIVATE void Done_Impl( const SfxItemSet *pSet );

//#endif
    //---------------------------------------------------------------------

public:
                        SfxRequest( SfxViewFrame*, sal_uInt16 nSlotId );
                        SfxRequest( sal_uInt16 nSlot, sal_uInt16 nCallMode, SfxItemPool &rPool );
                        SfxRequest( const SfxSlot* pSlot, const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rArgs,
                                            sal_uInt16 nCallMode, SfxItemPool &rPool );
                        SfxRequest( sal_uInt16 nSlot, sal_uInt16 nCallMode, const SfxAllItemSet& rSfxArgs );
                        SfxRequest( const SfxRequest& rOrig );
                        ~SfxRequest();

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

    static SfxMacro*    GetRecordingMacro();
    static com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > GetMacroRecorder( SfxViewFrame* pFrame=NULL );
    static sal_Bool         HasMacroRecorder( SfxViewFrame* pFrame=NULL );
    sal_uInt16              GetCallMode() const;
    bool            IsRecording() const;
    void                AllowRecording( sal_Bool );
    sal_Bool                AllowsRecording() const;
    sal_Bool                IsAPI() const;
    sal_Bool                IsSynchronCall() const;
    void                SetSynchronCall( sal_Bool bSynchron );
    void                SetTarget( const String &rTarget );

    sal_Bool                IsDone() const;
    void                Done( sal_Bool bRemove = sal_False );

    void                Ignore();
    void                Cancel();
    sal_Bool                IsCancelled() const;
    void                Done(const SfxItemSet &, bool bKeep = true );

    void                ForgetAllArgs();

private:
    const SfxRequest&   operator=(const SfxRequest &); // n.i.!!
};

//------------------------------------------------------------------------

#define SFX_REQUEST_ARG(rReq, pItem, ItemType, nSlotId, bDeep) \
        const ItemType *pItem = (const ItemType*) \
                rReq.GetArg( nSlotId, bDeep, TYPE(ItemType) )
#define SFX_ITEMSET_ARG(pArgs, pItem, ItemType, nSlotId, bDeep) \
    const ItemType *pItem = (const ItemType*) \
        SfxRequest::GetItem( pArgs, nSlotId, bDeep, TYPE(ItemType) )

#endif
