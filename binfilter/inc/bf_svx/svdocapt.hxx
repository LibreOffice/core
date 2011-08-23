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

#ifndef _SVDCAPT_HXX
#define _SVDCAPT_HXX

#include <bf_svx/svdorect.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrCaptionSetItem;
class ImpCaptParams;

#define SDRSETITEM_CAPTION_ATTR		SDRSETITEM_ATTR_COUNT

//************************************************************
//   Hilfsklasse SdrCaptObjGeoData
//************************************************************

class SdrCaptObjGeoData : public SdrRectObjGeoData
{
public:
    Polygon						aTailPoly;
};

//************************************************************
//   SdrCaptionObj
//************************************************************

class SdrCaptionObj : public SdrRectObj
{
    friend class				SdrTextObj; // fuer ImpRecalcTail() bei AutoGrow

protected:
    Polygon						aTailPoly;  // das ganze Polygon des Schwanzes
    sal_Bool					mbSpecialTextBoxShadow; // for calc special shadow, default FALSE

private:
    void ImpGetCaptParams(ImpCaptParams& rPara) const;
    void ImpCalcTail3(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    void ImpCalcTail (const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    void ImpRecalcTail();

public:
    TYPEINFO();
    SdrCaptionObj();
    virtual ~SdrCaptionObj();

    virtual UINT16 GetObjIdentifier() const;
    virtual void RecalcBoundRect();

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = TRUE; }


    virtual void SetModel(SdrModel* pNewModel);

    // ItemSet access
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);




    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);

    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual const Point& GetAnchorPos() const;

    virtual void RecalcSnapRect();
    virtual const Rectangle& GetSnapRect() const;
    virtual void NbcSetSnapRect(const Rectangle& rRect);


protected:

public:

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    const Point& GetTailPos() const;
    void SetTailPos(const Point& rPos);
    void NbcSetTailPos(const Point& rPos);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOCAPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
