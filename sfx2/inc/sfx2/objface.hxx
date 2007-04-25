/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 15:22:46 $
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
#ifndef _SFXOBJFACE_HXX
#define _SFXOBJFACE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#include <sfx2/msg.hxx>

struct SfxFormalArgument;
struct SfxTypeLibImpl;
struct SfxInterface_Impl;
class  SfxConfigManager;
class  SfxConfigItem;
class  SfxIFConfig_Impl;
class  SfxObjectUIArr_Impl ;
class  SfxModule;
class  SvStream;

class SFX2_DLLPUBLIC SfxInterface
{
friend class SfxIFConfig_Impl;
friend class SfxSlotPool;

    const char*             pName;          // Sfx-internal name of interface
    const SfxInterface*     pGenoType;      // base interface
    SfxSlot*                pSlots;         // SlotMap
    USHORT                  nCount;         // number of slots in SlotMap
    SfxInterfaceId          nClassId;       // Id of interface
    ResId                   aNameResId;     // ResId of external interface name
    SfxInterface_Impl*      pImpData;

    SfxSlot*                operator[]( USHORT nPos ) const;

public:
                            SfxInterface( const char *pClass,
                                          const ResId& rResId,
                                          SfxInterfaceId nClassId,
                                          const SfxInterface* pGeno,
                                          SfxSlot &rMessages, USHORT nMsgCount );
    virtual                 ~SfxInterface();

    void                    SetSlotMap( SfxSlot& rMessages, USHORT nMsgCount );
    inline USHORT           Count() const;

    const SfxSlot*          GetRealSlot( const SfxSlot * ) const;
    const SfxSlot*          GetRealSlot( USHORT nSlotId ) const;
    virtual const SfxSlot*  GetSlot( USHORT nSlotId ) const;
    const SfxSlot*          GetSlot( const String& rCommand ) const;

    const char*             GetClassName() const { return pName; }
    int                     HasName() const { return 0 != aNameResId.GetId(); }
    String                  GetName() const
                            { return String(aNameResId); }
    ResMgr*                 GetResMgr() const
                            { return aNameResId.GetResMgr(); }

    const SfxInterface*     GetGenoType() const { return pGenoType; }
    const SfxInterface*     GetRealInterfaceForSlot( const SfxSlot* ) const;

    void                    RegisterObjectBar( USHORT, const ResId&, const String* pST=0 );
    void                    RegisterObjectBar( USHORT, const ResId&, sal_uInt32 nFeature, const String* pST=0 );
    void                    RegisterChildWindow( USHORT, BOOL bContext, const String* pST=0 );
    void                    RegisterChildWindow( USHORT, BOOL bContext, sal_uInt32 nFeature, const String* pST=0 );
    void                    RegisterStatusBar( const ResId& );
    const ResId&            GetObjectBarResId( USHORT nNo ) const;
    USHORT                  GetObjectBarPos( USHORT nNo ) const;
    sal_uInt32              GetObjectBarFeature( USHORT nNo ) const;
    USHORT                  GetObjectBarCount() const;
    void                    SetObjectBarPos( USHORT nPos, USHORT nId );
    const String*           GetObjectBarName( USHORT nNo ) const;
    BOOL                    IsObjectBarVisible( USHORT nNo) const;
    sal_uInt32              GetChildWindowFeature( USHORT nNo ) const;
    sal_uInt32              GetChildWindowId( USHORT nNo ) const;
    USHORT                  GetChildWindowCount() const;
    void                    RegisterPopupMenu( const ResId& );
    const ResId&            GetPopupMenuResId() const;
    const ResId&            GetStatusBarResId() const;

    void                    Register( SfxModule* );

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE int      ContainsSlot_Impl( const SfxSlot *pSlot ) const
                            { return pSlot >= pSlots && pSlot < pSlots + Count(); }
    SAL_DLLPRIVATE ResMgr*  GetResManager_Impl() const
                            { return aNameResId.GetResMgr(); }
//#endif
};

//--------------------------------------------------------------------

// returns the number of functions in this cluster

inline USHORT SfxInterface::Count() const
{
    return nCount;
}

//--------------------------------------------------------------------

// returns a function by position in the array

inline SfxSlot* SfxInterface::operator[]( USHORT nPos ) const
{
    return nPos < nCount? pSlots+nPos: 0;
}

//#if 0 // _SOLAR__PRIVATE
class SfxIFConfig_Impl
{
friend class SfxInterface;
    USHORT                  nCount;
    SfxObjectUIArr_Impl*    pObjectBars;

public:
                    SfxIFConfig_Impl();
                    ~SfxIFConfig_Impl();
    BOOL            Store(SvStream&);
    void            RegisterObjectBar( USHORT, const ResId&, sal_uInt32 nFeature, const String* pST=0 );
    USHORT          GetType();
};
//#endif

#endif
