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

#ifndef _SCH_SCHGROUP_HXX
#define _SCH_SCHGROUP_HXX

#include <bf_svx/svdogrp.hxx>

#include "chtmodel.hxx"
namespace binfilter {

/************************************************************************/

class SchObjGroup : public SdrObjGroup
{
 public:
    enum ChartGroupTypeNames {LEGEND, DIAGRAM, NOTHING};

 private:
    SdrObjTransformInfoRec aInfo;
    ChartGroupTypeNames eChartGroupType;  // FG: 11.3.97 Wird einfach auf TRUE gesetzt falls es
                                   //     das Chart selbst ist. Das muss beim Resize anders
                                   //     behandelt werden
    ChartModel *pChartmodel;       // Auch das ist nur fuer das Resize wichtig, um dort
                                   // CreateChart() aufzurufen;

    BOOL bAskForLogicRect;         // Da die Groesse eines Gruppenobjektes durch die
                                   // Mitlgieder berechnet wird, hilft ein Resize der Gruppe nichts.
                                   // Bei BuildChart muss eine Neuberechnung bisweilen verhindert
                                   // werden. (z.B falls der Benutzer die Gruppe selbst resized hat)
    bool mbUseChartInventor;

public:

    TYPEINFO();

    SchObjGroup(ChartModel *pChmodel = NULL);
    virtual ~SchObjGroup();

    virtual UINT32 GetObjInventor() const;
    virtual UINT16 GetObjIdentifier() const;



    // FG: Damit soll ermöglicht werden dass man Objektgruppen im Chart resizen kann
    void	SetObjInfo(SdrObjTransformInfoRec aMyInfo);

    // FG: Um das Verhalten bei einem Resize zu aendern muessen beide Routinen
    //     überladen werden.

    // FG: 9.3.1997 Methode von Joe, die Überladen wird um zu Kennzeichnen
    //              ob die Gruppe jemals verschoben worden ist.
    virtual void Move  (const Size& rSiz);

    void SetGroupType (ChartGroupTypeNames value) {eChartGroupType = value;}
    ChartGroupTypeNames GetGroupType() {return eChartGroupType;}
    void SetGroupIsChart() {eChartGroupType = DIAGRAM;}
    BOOL GetGroupIsChart() {return (eChartGroupType == DIAGRAM);}


    void SetModel (ChartModel *pChModel) {pChartmodel = pChModel; SdrObjGroup::SetModel( (SdrModel*) pChModel ); }

    void SetAskForLogicRect(BOOL value) {bAskForLogicRect = value;}
    BOOL GetAskForLogicRect() {return bAskForLogicRect;}

    void SetUseChartInventor( bool bUseChartInventor );
};

} //namespace binfilter
#endif	// _SCH_SCHGROUP_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
