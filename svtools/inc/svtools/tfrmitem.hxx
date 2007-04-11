/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tfrmitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:40:24 $
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
#ifndef _TFRMITEM_HXX
#define _TFRMITEM_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#include <svtools/poolitem.hxx>

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

