/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clipfmtitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:21:34 $
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
#ifndef _SVX_CLIPFMTITEM_HXX
#define _SVX_CLIPFMTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxClipboardFmtItem ----------------------------------------------
struct SvxClipboardFmtItem_Impl;

class SVX_DLLPUBLIC SvxClipboardFmtItem : public SfxPoolItem
{
    SvxClipboardFmtItem_Impl* pImpl;
protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

public:
    TYPEINFO();
    SvxClipboardFmtItem( USHORT nId = 0 );
    SvxClipboardFmtItem( const SvxClipboardFmtItem& );
    virtual ~SvxClipboardFmtItem();

    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const;
    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId );

    void AddClipbrdFormat( ULONG nId, USHORT nPos = USHRT_MAX );
    void AddClipbrdFormat( ULONG nId, const String& rName,
                            USHORT nPos = USHRT_MAX );
    USHORT Count() const;

    ULONG GetClipbrdFormatId( USHORT nPos ) const;
    const String& GetClipbrdFormatName( USHORT nPos ) const;
};


#endif

