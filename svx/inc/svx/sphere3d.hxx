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



#ifndef _E3D_SPHERE3D_HXX
#define _E3D_SPHERE3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Kugelobjekt mit Durchmesser r3DSize; Anzahl der Flaechen wird durch
|* die horizontale und vertikale Segmentanzahl vorgegeben
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dSphereObj : public E3dCompoundObject
{
private:
    basegfx::B3DPoint               aCenter;
    basegfx::B3DVector              aSize;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

public:
    TYPEINFO();
    E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize);
    E3dSphereObj();

    // FG: Dieser Konstruktor wird nur von MakeObject aus der 3d-Objectfactory beim
    //     Laden von Dokumenten mit Kugeln aufgerufen. Dieser Konstruktor ruft kein
    //     CreateSphere auf, er erzeugt also keine Kugel.
    E3dSphereObj(int dummy);

    // HorizontalSegments:
    sal_uInt32 GetHorizontalSegments() const
        { return ((const Svx3DHorizontalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_uInt32 GetVerticalSegments() const
        { return ((const Svx3DVerticalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

    virtual void operator=(const SdrObject&);

    void ReSegment(sal_uInt32 nHorzSegments, sal_uInt32 nVertSegments);
    const basegfx::B3DPoint& Center() const { return aCenter; }
    const basegfx::B3DVector& Size() const { return aSize; }

    // Lokale Parameter setzen mit Geometrieneuerzeugung
    void SetCenter(const basegfx::B3DPoint& rNew);
    void SetSize(const basegfx::B3DVector& rNew);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
};

#endif          // _E3D_SPHERE3D_HXX
