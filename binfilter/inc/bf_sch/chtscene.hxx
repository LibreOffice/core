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

#ifndef _CHTSCENE_HXX
#define _CHTSCENE_HXX

#include <bf_svx/polysc3d.hxx>
#include <chtmodel.hxx>
namespace binfilter {

/*************************************************************************
|*
|* Basisklasse fuer 3D-Szenen
|* Die Ableitung von SchObjGroup erfolgt damit das Chart frei plaziert
|* werden kann. Die Komponenten von 2d-Charts sind auch SchObjGroups
|*
\************************************************************************/

//DECLARE_LIST(DescrList, E3dLabelObj*)//STRIP008 //DECLARE_LIST(DescrList, E3dLabelObj*);

class ChartScene : public E3dPolyScene
{
  protected:
    ChartModel* pDoc;
    BOOL        bAskForLogicRect;
    Rectangle   Get3DDescrRect(E3dLabelObj *p3DObj,B3dCamera& rCamSet);

  public:

    TYPEINFO();
    ChartScene(ChartModel* pModel);
    virtual ~ChartScene();
    void Initialize();

    virtual Volume3D FitInSnapRect();

    BOOL GetAskForLogicRect() {return bAskForLogicRect;}
    void SetAskForLogicRect(BOOL value) {bAskForLogicRect = value;}

    virtual void WriteData(SvStream& rOut) const;

    void InsertAllTitleText (DescrList         &rList,
                             E3dObject         *pGroup,
                             long              nAxisId);

    // just call parent? overloading deprecated?
    //	The following three methods call their respective equivalents of
    //	E3dObject.  This is necessary because they are overloaded in E3dScene
    //	with methods that do not what we want :-)
//  	virtual	USHORT	GetHdlCount	(void)	const
//  		{	return E3dObject::GetHdlCount();	}
//  	virtual	void	AddToHdlList	(SdrHdlList& rHdlList)	const
//  		{	E3dObject::AddToHdlList (rHdlList);	}
//  	virtual FASTBOOL	HasSpecialDrag	(void)	const
//  		{	return E3dObject::HasSpecialDrag ();	}

    void ReduceDescrList(DescrList& aList);
};


} //namespace binfilter
#endif			// _E3D_SCENE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
