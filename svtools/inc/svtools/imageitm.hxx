/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imageitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:22:00 $
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

#ifndef _SVT_IMAGEITM_HXX
#define _SVT_IMAGEITM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

class String;

struct SfxImageItem_Impl;
class SVT_DLLPUBLIC SfxImageItem : public SfxInt16Item
{
    SfxImageItem_Impl*      pImp;
public:
                            TYPEINFO();
                            SfxImageItem( USHORT nWhich = 0, UINT16 nImage = 0 );
                            SfxImageItem( USHORT nWhich, const String& rURL );
                            SfxImageItem( const SfxImageItem& );
    virtual                 ~SfxImageItem();

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    void                    SetRotation( long nValue );
    long                    GetRotation() const;
    void                    SetMirrored( BOOL bSet );
    BOOL                    IsMirrored() const;
    String                  GetURL() const;
};

#endif // _SFX_IMAGEITM_HXX
