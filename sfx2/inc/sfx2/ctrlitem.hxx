/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ctrlitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:17:39 $
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
#ifndef _SFXCTRLITEM_HXX
#define _SFXCTRLITEM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

class SfxPoolItem;
class SfxBindings;
class SvStream;

class SFX2_DLLPUBLIC SfxControllerItem
{
private:
    USHORT              nId;
    SfxControllerItem*  pNext; // zu benachrichtigendes weiteres ControllerItem
    SfxBindings*        pBindings;

protected:
//#if defined( DBG_UTIL ) && defined( _SOLAR__PRIVATE )
#if defined( DBG_UTIL )
    SAL_DLLPRIVATE void CheckConfigure_Impl( ULONG nType );
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
                        SfxControllerItem( USHORT nId, SfxBindings & );
    virtual             ~SfxControllerItem();

    void                Bind( USHORT nNewId, SfxBindings * = 0);    // in SfxBindings registrieren
    void                UnBind();
    void                ReBind();
    BOOL                IsBound() const;
    void                UpdateSlot();
    void                ClearCache();
    void                SetBindings(SfxBindings &rBindings) { pBindings = &rBindings; }

    SfxControllerItem*  GetItemLink();
    SfxControllerItem*  ChangeItemLink( SfxControllerItem* pNewLink );

    void                SetId( USHORT nItemId );
    USHORT              GetId() const { return nId; }

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual void        DeleteFloatingWindow();

    SfxMapUnit          GetCoreMetric() const;

    static SfxItemState GetItemState( const SfxPoolItem* pState );

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE BOOL IsBindable_Impl() const
                        { return pBindings != NULL; }
    SAL_DLLPRIVATE void BindInternal_Impl( USHORT nNewId, SfxBindings* );
//#endif
};

//====================================================================

class SFX2_DLLPUBLIC SfxStatusForwarder: public SfxControllerItem
{
    SfxControllerItem*      pMaster;

protected:
    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );

public:
                            SfxStatusForwarder( USHORT nSlotId,
                                   SfxControllerItem&rMaster );
};

#endif

