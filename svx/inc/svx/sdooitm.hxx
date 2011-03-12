/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SDOOITM_HXX
#define _SDOOITM_HXX

#include <svl/eitem.hxx>
#include "svx/svxdllapi.h"


//------------------------------------------------------------
// class SdrOnOffItem
// here GetValueTextByVal() returns "an" or "aus" instead
// of "TRUE" or "FALSE"
//------------------------------------------------------------
class SVX_DLLPUBLIC SdrOnOffItem: public SfxBoolItem {
public:
    TYPEINFO();
    SdrOnOffItem(): SfxBoolItem() {}
    SdrOnOffItem(sal_uInt16 nId, sal_Bool bOn=sal_False): SfxBoolItem(nId,bOn) {}
    SdrOnOffItem(sal_uInt16 nId, SvStream& rIn):  SfxBoolItem(nId,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    virtual String  GetValueTextByVal(sal_Bool bVal) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
