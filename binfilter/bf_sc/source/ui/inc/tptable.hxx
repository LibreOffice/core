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

#ifndef SC_TPTABLE_HXX
#define SC_TPTABLE_HXX

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>
namespace binfilter {

//===================================================================

class ScTablePage : public SfxTabPage
{
public:
    static	SfxTabPage*	Create			( Window*		 	pParent,
                                          const SfxItemSet&	rCoreSet );
    static	USHORT*		GetRanges		();
    virtual	BOOL		FillItemSet		( SfxItemSet& rCoreSet );
    virtual	void		Reset			( const SfxItemSet& rCoreSet );
    virtual int			DeactivatePage	( SfxItemSet* pSet = NULL );
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt );

private:
           ScTablePage( Window*			pParent,
                         const SfxItemSet&	rCoreSet );
            ~ScTablePage();

    void            ShowImage();

private:
    FixedLine       aFlPageDir;
    RadioButton		aBtnTopDown;
    RadioButton		aBtnLeftRight;
    FixedImage      aBmpPageDir;
    Image           aImgLeftRight;
    Image           aImgTopDown;
    Image           aImgLeftRightHC;
    Image           aImgTopDownHC;
    CheckBox		aBtnPageNo;
    NumericField	aEdPageNo;

    FixedLine       aFlPrint;
    CheckBox		aBtnHeaders;
    CheckBox		aBtnGrid;
    CheckBox		aBtnNotes;
    CheckBox		aBtnObjects;
    CheckBox		aBtnCharts;
    CheckBox		aBtnDrawings;
    CheckBox		aBtnFormulas;
    CheckBox		aBtnNullVals;

    FixedLine       aFlScale;
    RadioButton		aBtnScaleAll;
    RadioButton		aBtnScalePageNum;
    MetricField     aEdScaleAll;
    NumericField	aEdScalePageNum;

#ifdef _TPTABLE_CXX
private:
    //------------------------------------
    // Handler:
    DECL_LINK( ScaleHdl,   RadioButton* );
    DECL_LINK( PageDirHdl, RadioButton* );
    DECL_LINK( PageNoHdl,  CheckBox* );
#endif
};



} //namespace binfilter
#endif // SC_TPTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
