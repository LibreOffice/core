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

#ifndef _SVDOPAGE_HXX
#define _SVDOPAGE_HXX

#include <bf_svx/svdobj.hxx>
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPageObj : public SdrObject
{
    SfxItemSet*				mpPageItemSet;
    sal_uInt16				nPageNum;
    BOOL					bPainting;  // Verriegelungsflag
    BOOL					bNotifying; // Verriegelungsflag

protected:
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

public:
    TYPEINFO();
    SdrPageObj(USHORT nNewPageNum=0);
    ~SdrPageObj();

    virtual void SetModel(SdrModel* pNewModel);
    USHORT GetPageNum() const { return nPageNum; }
    void SetPageNum(USHORT nNewPageNum) 
        { NbcSetPageNum(nNewPageNum); SetChanged(); SendRepaintBroadcast(GetBoundRect()); }
    void NbcSetPageNum(USHORT nNewPageNum) { nPageNum=nNewPageNum; }

    virtual UINT16 GetObjIdentifier() const;


    virtual const Rectangle& GetBoundRect() const;
    virtual const Rectangle& GetSnapRect() const;
    virtual const Rectangle& GetLogicRect() const;
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    // ItemSet access
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);
    virtual const SfxItemSet& GetItemSet() const;
    // #86481# simply ignore item setting on page objects
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
