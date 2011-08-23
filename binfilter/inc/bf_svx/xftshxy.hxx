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

#ifndef _SVX_XFTSHXY_HXX
#define _SVX_XFTSHXY_HXX

#ifndef _SFXMETRICITEM_HXX //autogen
#include <bf_svtools/metitem.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* FormText-ShadowXValItem
|*
\************************************************************************/

class XFormTextShadowXValItem : public SfxMetricItem
{
public:
                            TYPEINFO();
                            XFormTextShadowXValItem(long nVal = 0);
                            XFormTextShadowXValItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;
};

/*************************************************************************
|*
|* FormText-ShadowYValItem
|*
\************************************************************************/

class XFormTextShadowYValItem : public SfxMetricItem
{
public:
                            TYPEINFO();
                            XFormTextShadowYValItem(long nVal = 0);
                            XFormTextShadowYValItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;
};

}//end of namespace binfilter
#endif

