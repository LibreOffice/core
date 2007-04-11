/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: request.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:27:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXREQUEST_HXX
#define _SFXREQUEST_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDER_HPP_
#include <com/sun/star/frame/XDispatchRecorder.hpp>
#endif

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

    USHORT              nSlot;
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
                        SfxRequest( SfxViewFrame*, USHORT nSlotId );
                        SfxRequest( USHORT nSlot, USHORT nCallMode, SfxItemPool &rPool );
                        SfxRequest( const SfxSlot* pSlot, const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rArgs,
                                            USHORT nCallMode, SfxItemPool &rPool );
                        SfxRequest( USHORT nSlot, USHORT nCallMode, const SfxAllItemSet& rSfxArgs );
                        SfxRequest( const SfxRequest& rOrig );
                        ~SfxRequest();

    USHORT              GetSlot() const { return nSlot; }
    void                SetSlot(USHORT nNewSlot) { nSlot = nNewSlot; }

    USHORT              GetModifier() const;
    void                SetModifier( USHORT nModi );
    SAL_DLLPRIVATE void SetInternalArgs_Impl( const SfxAllItemSet& rArgs );
    SAL_DLLPRIVATE const SfxItemSet* GetInternalArgs_Impl() const;
    const SfxItemSet*   GetArgs() const { return pArgs; }
    void                SetArgs( const SfxAllItemSet& rArgs );
    void                AppendItem(const SfxPoolItem &);
    void                RemoveItem( USHORT nSlotId );

    static const SfxPoolItem* GetItem( const SfxItemSet*, USHORT nSlotId,
                                       bool bDeep = false,
                                       TypeId aType = 0 );
    const SfxPoolItem*  GetArg( USHORT nSlotId, FASTBOOL bDeep = FALSE, TypeId aType = 0 ) const;
    void                ReleaseArgs();
    void                SetReturnValue(const SfxPoolItem &);
    const SfxPoolItem*  GetReturnValue() const;

    static SfxMacro*    GetRecordingMacro();
    static com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > GetMacroRecorder( SfxViewFrame* pFrame=NULL );
    static BOOL         HasMacroRecorder( SfxViewFrame* pFrame=NULL );
    USHORT              GetCallMode() const;
    FASTBOOL            IsRecording() const;
    void                AllowRecording( BOOL );
    BOOL                AllowsRecording() const;
    BOOL                IsAPI() const;
    BOOL                IsSynchronCall() const;
    void                SetSynchronCall( BOOL bSynchron );
    void                SetTarget( const String &rTarget );

    BOOL                IsDone() const;
    void                Done( BOOL bRemove = FALSE );

    void                Ignore();
    void                Cancel();
    BOOL                IsCancelled() const;
    void                Done(const SfxItemSet &, FASTBOOL bKeep = TRUE );

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
