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
#ifndef _SFX_SHELL_HXX
#define _SFX_SHELL_HXX

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <bf_svtools/brdcst.hxx>
#endif

#include <tools/string.hxx>

namespace binfilter {

class SfxItemPool;
class SfxPoolItem;


struct SfxShell_Impl;

typedef SfxPoolItem* SfxPoolItemPtr;
SV_DECL_PTRARR_DEL( SfxItemPtrArray, SfxPoolItemPtr, 4, 4 )//STRIP008 ;
typedef SfxItemPtrArray SfxItemArray_Impl;

//====================================================================

class SfxShell: public SfxBroadcaster
{
    friend class SfxObjectItem;

    SfxShell_Impl*				pImp;
    SfxItemPool*				pPool;
    String						aName;

private:
                                SfxShell( const SfxShell & ); // n.i.
    SfxShell&					operator = ( const SfxShell & ); // n.i.

protected:
                                SfxShell();
public:
                                TYPEINFO();
    virtual                     ~SfxShell();

    void						SetName( const String &rName ) { aName = rName; }
    const String&   			GetName() const { return aName; }

    SfxItemPool&				GetPool() const { return *pPool; }
    void						SetPool( SfxItemPool *pNewPool ) { pPool = pNewPool; }

    const SfxPoolItem*			GetItem( USHORT nSlotId ) const;
    void						PutItem( const SfxPoolItem& rItem );
    void						RemoveItem( USHORT nSlotId );
};

}//end of namespace binfilter
#endif

