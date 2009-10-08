/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tfrmitem.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _TFRMITEM_HXX
#define _TFRMITEM_HXX

#include <tools/rtti.hxx>

#include <svl/poolitem.hxx>

class SvStream;

enum SfxOpenMode
{
    SfxOpenSelect       = 0,     // selected in view
    SfxOpenOpen         = 1,     // doubleclicked or <enter>
    SfxOpenAddTask      = 2,     // doubleclicked or <enter> with Ctrl-Modifier
    SfxOpenDontKnow     = 3,
    SfxOpenReserved1    = 4,
    SfxOpenReserved2    = 5,
    SfxOpenModeLast     = 5
};

DBG_NAMEEX(SfxTargetFrameItem)

// class SfxTargetFrameItem -------------------------------------------------

class SfxTargetFrameItem : public SfxPoolItem
{
private:
            String          _aFrames[ (USHORT)SfxOpenModeLast+1 ];
public:
            TYPEINFO();

            SfxTargetFrameItem( const SfxTargetFrameItem& rCpy );
            SfxTargetFrameItem( USHORT nWhich );
            SfxTargetFrameItem(
                USHORT nWhich,
                const String& rOpenSelectFrame,
                const String& rOpenOpenFrame,
                const String& rOpenAddTaskFrame );
            ~SfxTargetFrameItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nItemVersion ) const;
    virtual SvStream&       Store( SvStream&, USHORT nItemVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 );
    /*
        Framebezeichner im Sfx:
        _browser : Beamerview
        _document : Desktopview
        _blank : new task
        "" : do nothing
    */
    String                  GetTargetFrame( SfxOpenMode eMode ) const;
};

#endif

