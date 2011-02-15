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
#ifndef _IDLITEMS_HXX
#define _IDLITEMS_HXX

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>


class SvxDrawToolItem : public SfxEnumItem
{
public:

        SvxDrawToolItem( const SvxDrawToolItem& rDrawToolItem ) :
                                SfxEnumItem( rDrawToolItem ){}

        SvxDrawToolItem(sal_uInt16 nWhichP) : SfxEnumItem(nWhichP){}


    virtual String              GetValueText() const;


    virtual String              GetValueText(sal_uInt16 nVal) const;
    virtual sal_uInt16              GetValueCount() const
                                    {return((sal_uInt16)SVX_SNAP_DRAW_TEXT);}

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*        Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline SvxDrawToolItem& operator=(const  SvxDrawToolItem&
                                                        rDrawTool)
            {
                SetValue( rDrawTool.GetValue() );
                return *this;

            }
};
#endif


