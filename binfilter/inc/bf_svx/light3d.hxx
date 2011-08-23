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

#ifndef _E3D_LIGHT3D_HXX
#define _E3D_LIGHT3D_HXX

#ifndef _E3D_PNTOBJ3D_HXX
#include <bf_svx/pntobj3d.hxx>
#endif
namespace binfilter {

/************************************************************************/

class E3dLight;

#ifndef _E3D_LIGHT3D_LIST
DECLARE_LIST(E3dLightList, E3dLight*)//STRIP008 DECLARE_LIST(E3dLightList, E3dLight*);
#endif

/*************************************************************************
|*
|* Basisklasse fuer alle Lichtquellen, gleichzeitig als ambiente
|* (ungerichtete) Lichtquelle zu verwenden. D.h., die Position dient
|* nur der Verwaltung, hat aber keinen Einfluss auf die Beleuchtung.
|*
\************************************************************************/

class E3dLight : public E3dPointObj
{
 private:
    Color		aColor;
    double		fIntensity;
    double		fRed, fGreen, fBlue;

    FASTBOOL	bOn				: 1;
    FASTBOOL	bVisible		: 1;

 protected:
    FASTBOOL ImpCalcLighting(Color& rNewColor, const Color& rPntColor,
                             double fR, double fG, double fB) const;
    // Lichtquellen koennen Unterobjekte nur selbst anlegen
    virtual void Insert3DObj(E3dObject* p3DObj);

    virtual void CreateLightObj();

 public:
    TYPEINFO();
    E3dLight(const Vector3D& rPos, const Color& rColor,
             double fLightIntensity = 1.0);
    E3dLight();

    virtual UINT16 GetObjIdentifier() const;

    virtual FASTBOOL CalcLighting(Color& rNewColor,
                                  const Vector3D& rPnt,
                                  const Vector3D& rPntNormal,
                                  const Color& rPntColor);

    virtual void SetPosition(const Vector3D& rNewPos);

    double GetIntensity() const { return fIntensity; }
    virtual void SetIntensity(double fNew);

    const Color& GetColor() const { return aColor; }
    virtual void SetColor(const Color& rNewColor);

    FASTBOOL IsOn() const { return bOn; }

    FASTBOOL IsLightObjVisible() const { return bVisible; }

    double GetRed() const	{ return fRed; }
    double GetGreen() const	{ return fGreen; }
    double GetBlue() const	{ return fBlue; }

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);


//-/	virtual void           SetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
//-/	virtual void           NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll);
//-/	virtual void           TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const;
//-/	virtual void Collect3DAttributes(SfxItemSet& rAttr) const;
//-/	virtual void Distribute3DAttributes(const SfxItemSet& rAttr);

    virtual void           SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
    virtual SfxStyleSheet* GetStyleSheet() const;
};

}//end of namespace binfilter
#endif			// _E3D_LIGHT3D_HXX
