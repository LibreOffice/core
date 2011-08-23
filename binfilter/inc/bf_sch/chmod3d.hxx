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


#ifndef SCH_CHARTMODEL_3D_OBJECTS
#define SCH_CHARTMODEL_3D_OBJECTS


//Ableitungen der 3D-Objekte, um #52277# zu beheben.
//Ueberladen der NbcSetAttributes um Attr abzufangen und in das ChartModel zu leiten

#include <bf_svx/extrud3d.hxx>
#include <bf_svx/polygn3d.hxx>
#include <bf_svx/deflt3d.hxx>
#include <bf_svx/svdorect.hxx>
#include <bf_svx/lathe3d.hxx>
#define CHART_SHAPE3D_IGNORE  -2 //intern! (GetChartShapeStyle()!)
#define CHART_SHAPE3D_ANY	  -1 //undefinierter Typ (GetChartShapeStyle()!)
#define CHART_SHAPE3D_SQUARE   0
#define CHART_SHAPE3D_CYLINDER 1
#define CHART_SHAPE3D_CONE     2
#define CHART_SHAPE3D_PYRAMID  3 //reserved
#define CHART_SHAPE3D_HANOI	   4
namespace binfilter {


class SchE3dLatheObj : public E3dLatheObj
{
public:
    SchE3dLatheObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPP)
        :E3dLatheObj(rDefault,rPP){};
    SchE3dLatheObj()
        :E3dLatheObj(){};

//-/	virtual void NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
    virtual void SetItem(const SfxPoolItem& rItem);
    virtual void SetItemSet(const SfxItemSet& rSet);
};


class SchE3dExtrudeObj : public E3dExtrudeObj
{
public:

    SchE3dExtrudeObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPP, double fDepth)
        :E3dExtrudeObj(rDefault,rPP,fDepth){};
    SchE3dExtrudeObj(E3dDefaultAttributes& rDefault, const XPolyPolygon& rXPP, double fDepth)
        :E3dExtrudeObj(rDefault,rXPP,fDepth){};
    SchE3dExtrudeObj()
        :E3dExtrudeObj(){};

//-/	virtual void NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
    virtual void SetItem(const SfxPoolItem& rItem);
    virtual void SetItemSet(const SfxItemSet& rSet);
};

class SchE3dPolygonObj : public E3dPolygonObj
{
public:
    SchE3dPolygonObj(E3dDefaultAttributes& rDefault, const PolyPolygon3D& rPoly3D,
        BOOL bLinOnly=FALSE)
        :E3dPolygonObj(rDefault,rPoly3D,bLinOnly){};

    SchE3dPolygonObj(E3dDefaultAttributes& rDefault, const PolyPolygon3D& rPoly3D,
        const PolyPolygon3D& rVector3D, BOOL bLinOnly=FALSE)
        :E3dPolygonObj(rDefault,rPoly3D,rVector3D,bLinOnly){};

    SchE3dPolygonObj(E3dDefaultAttributes& rDefault, const Vector3D& rP1,
        const Vector3D& rP2, BOOL bLinOnly=TRUE)
        : E3dPolygonObj(rDefault,rP1,rP2,bLinOnly){};

    SchE3dPolygonObj() : E3dPolygonObj(){};

//-/	virtual void NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
    virtual void SetItem(const SfxPoolItem& rItem);
    virtual void SetItemSet(const SfxItemSet& rSet);
};


class SchE3dObject : public E3dObject
{

 public:
     SchE3dObject()
         :E3dObject(){};

//-/	 virtual void NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
    virtual void SetItemSet(const SfxItemSet& rSet);
};
class SchRectObj : public SdrRectObj
{
public:
    SchRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect)
        :SdrRectObj(eNewTextKind,rRect){};
    SchRectObj(const Rectangle& rRect): SdrRectObj(rRect){};

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
//-/	virtual void NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
    virtual void SetItemSet(const SfxItemSet& rSet);
};

} //namespace binfilter
#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
