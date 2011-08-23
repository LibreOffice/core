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

#ifndef _E3D_LABEL3D_HXX
#define _E3D_LABEL3D_HXX

#include <bf_svx/pntobj3d.hxx>
namespace binfilter {

class Viewport3D;
class E3dScene;
class E3dLabelObj;

/************************************************************************/

class E3dLabelObj;
DECLARE_LIST(E3dLabelList, E3dLabelObj*)//STRIP008 DECLARE_LIST(E3dLabelList, E3dLabelObj*);

/*************************************************************************
|*
|* Objekt zur Verknuepfung eines 2D-Labels mit einer 3D-Position.
|* Bei einer Transformation wird die neue Position des 2D-Objekts
|* mit SetAnchorPos gesetzt. Wenn das Objekt zu dieser Position
|* ausgerichtet werden soll (z.B. rechtsbuendig), kann das mittels
|* SetRelativePos erreicht werden.
|*
\************************************************************************/

class E3dLabelObj : public E3dPointObj
{
 // damit nicht jeder daran rumpfuschen kann...
 friend class E3dScene;
    SdrObject* GetMy2DLabelObj() { return p2DLabelObj; }

 protected:
    SdrObject* p2DLabelObj;

 public:
    TYPEINFO();
    // Eigentumsuebergang bei pLabelObj!
    E3dLabelObj(const Vector3D& aPos,
                SdrObject*      pLabelObj) :
        E3dPointObj(aPos),
        p2DLabelObj(pLabelObj)
    {
    }

    E3dLabelObj() :
        E3dPointObj(),
        p2DLabelObj(NULL)
    {
    }

    virtual ~E3dLabelObj();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pNewModel);

    virtual UINT16 GetObjIdentifier() const;


    const SdrObject* Get2DLabelObj() const { return p2DLabelObj; }

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".

};

}//end of namespace binfilter
#endif			// _E3D_LABEL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
