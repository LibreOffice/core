/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawattr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:56:03 $
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
#ifndef _IDLITEMS_HXX
#define _IDLITEMS_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif


class SvxDrawToolItem : public SfxEnumItem
{
public:

        SvxDrawToolItem( const SvxDrawToolItem& rDrawToolItem ) :
                                SfxEnumItem( rDrawToolItem ){}

        SvxDrawToolItem(USHORT nWhichP) : SfxEnumItem(nWhichP){}


    virtual String              GetValueText() const;


    virtual String              GetValueText(USHORT nVal) const;
    virtual USHORT              GetValueCount() const
                                    {return((USHORT)SVX_SNAP_DRAW_TEXT);}

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*        Create( SvStream& rStream, USHORT nVer ) const;

    inline SvxDrawToolItem& operator=(const  SvxDrawToolItem&
                                                        rDrawTool)
            {
                SetValue( rDrawTool.GetValue() );
                return *this;

            }
};
#endif


