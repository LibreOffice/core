/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _DFLYOBJ_HXX
#define _DFLYOBJ_HXX
#include <svx/svdovirt.hxx>

class SwFlyFrm;
class SwFrmFmt;
class SdrObjMacroHitRec;

const sal_uInt32 SWGInventor =  sal_uInt32('S')*0x00000001+
                            sal_uInt32('W')*0x00000100+
                            sal_uInt32('G')*0x00010000;

const sal_uInt16 SwFlyDrawObjIdentifier = 0x0001;
const sal_uInt16 SwDrawFirst            = 0x0001;

//---------------------------------------
//SwFlyDrawObj, Die DrawObjekte fuer Flys.

class SwFlyDrawObj : public SdrObject
{
private:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    TYPEINFO();

    SwFlyDrawObj();
    ~SwFlyDrawObj();

    //Damit eine Instanz dieser Klasse beim laden erzeugt werden kann
    //(per Factory).
    virtual sal_uInt32 GetObjInventor()     const;
    virtual sal_uInt16 GetObjIdentifier()   const;
    virtual sal_uInt16 GetObjVersion()      const;
};

//---------------------------------------
//SwVirtFlyDrawObj, die virtuellen Objekte fuer Flys.
//Flys werden immer mit virtuellen Objekten angezeigt. Nur so koennen sie
//ggf. mehrfach angezeigt werden (Kopf-/Fusszeilen).

class SwVirtFlyDrawObj : public SdrVirtObj
{
private:
    SwFlyFrm *pFlyFrm;

protected:
    // AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
    // not used but something own (top left of new SnapRect minus top left
    // of original SnapRect)
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    // for paints triggered form ExecutePrimitive
    void wrap_DoPaintObject() const;

    // for simple access to inner and outer bounds
    basegfx::B2DRange getOuterBound() const;
    basegfx::B2DRange getInnerBound() const;

public:
    TYPEINFO();

    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);
    ~SwVirtFlyDrawObj();

    //Ueberladene Methoden der Basisklasse SdrVirtObj
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const;

    //Wir nehemen die Groessenbehandlung vollstaendig selbst in die Hand.
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLastBoundRect() const;
    virtual       void       RecalcBoundRect();
    virtual       void       RecalcSnapRect();
    virtual const Rectangle& GetSnapRect()  const;
    virtual       void       SetSnapRect(const Rectangle& rRect);
    virtual       void       NbcSetSnapRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual       void       SetLogicRect(const Rectangle& rRect);
    virtual       void       NbcSetLogicRect(const Rectangle& rRect);
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual       void       NbcMove  (const Size& rSiz);
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact);
    virtual       void       Move  (const Size& rSiz);
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact);

    const SwFrmFmt *GetFmt() const;
          SwFrmFmt *GetFmt();

    // Get Methoden fuer die Fly Verpointerung
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // ist eine URL an einer Grafik gesetzt, dann ist das ein Makro-Object
    virtual FASTBOOL HasMacro() const;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const;

    // FullDrag support
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;
};


#endif
