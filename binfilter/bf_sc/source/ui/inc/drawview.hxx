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

#ifndef SC_DRAWVIEW_HXX
#define SC_DRAWVIEW_HXX

#include <bf_svx/fmview.hxx>

#include "global.hxx"
namespace binfilter {

class ScDocument;
class ScViewData;
class SdrViewUserMarker;

class ScDrawView: public FmFormView
{
    ScViewData*				pViewData;
    OutputDevice*			pDev;					//! noetig ?
    ScDocument*				pDoc;
    USHORT					nTab;
    Fraction				aScaleX;				// Faktor fuer Drawing-MapMode
    Fraction				aScaleY;
    SdrViewUserMarker*		pDropMarker;
    SdrObject*				pDropMarkObj;
    BOOL					bInConstruct;
    BOOL					bDisableHdl;

    void			Construct();
    void			UpdateBrowser();

protected:
    virtual void	ModelHasChanged();

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

public:
                    ScDrawView( OutputDevice* pOut, ScViewData* pData );
    virtual			~ScDrawView();

    virtual void	MarkListHasChanged();
    virtual void	SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

    void			DrawMarks( OutputDevice* pOut ) const;


    BOOL			IsDisableHdl() const 	{ return bDisableHdl; }


    void			InvalidateAttribs();
    void			InvalidateDrawTextAttrs();


    void			GetScale( Fraction& rFractX, Fraction& rFractY ) const;
    void			RecalcScale();
    void			UpdateWorkArea();
    USHORT			GetTab() const		{ return nTab; }




    void			VCAddWin( Window* pWin );
    void			VCRemoveWin( Window* pWin );


    USHORT			GetPopupMenuId();
    void			UpdateUserViewOptions();


    String			GetSelectedChartName() const;


    SdrEndTextEditKind	ScEndTextEdit();	// ruft SetDrawTextUndo(0)
};




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
