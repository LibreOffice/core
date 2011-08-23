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

#ifndef SC_EDITUTIL_HXX
#define SC_EDITUTIL_HXX


#include <bf_svx/editeng.hxx>

#include <bf_svx/pageitem.hxx>

#include <tools/date.hxx>

#include <tools/time.hxx>

#include <tools/gen.hxx>

#include <tools/fract.hxx>
class OutputDevice;
namespace binfilter {


class ScDocument;
class ScPatternAttr;

class ScEditUtil
{
    ScDocument*		pDoc;
    USHORT			nCol;
    USHORT			nRow;
    USHORT			nTab;
    Point			aScrPos;
    OutputDevice*	pDev;			// MapMode muss eingestellt sein
    double			nPPTX;
    double			nPPTY;
    Fraction		aZoomX;
    Fraction		aZoomY;

    static const char __FAR_DATA pCalcDelimiters[];

public:
    static String ModifyDelimiters( const String& rOld );
    static String GetSpaceDelimitedString( const EditEngine& rEngine );

public:
                ScEditUtil( ScDocument* pDocument, USHORT nX, USHORT nY, USHORT nZ,
                            const Point& rScrPosPixel,
                            OutputDevice* pDevice, double nScaleX, double nScaleY,
                            const Fraction& rX, const Fraction& rY ) :
                    pDoc(pDocument),nCol(nX),nRow(nY),nTab(nZ),
                    aScrPos(rScrPosPixel),pDev(pDevice),
                    nPPTX(nScaleX),nPPTY(nScaleY),aZoomX(rX),aZoomY(rY) {}

};


class ScEditAttrTester
{
    EditEngine*	pEngine;
    SfxItemSet*	pEditAttrs;
    BOOL		bNeedsObject;
    BOOL		bNeedsCellAttr;

public:
                ScEditAttrTester( EditEngine* pEng );
                ~ScEditAttrTester();

    BOOL				NeedsObject() const		{ return bNeedsObject; }
    BOOL				NeedsCellAttr() const	{ return bNeedsCellAttr; }
    const SfxItemSet&	GetAttribs() const		{ return *pEditAttrs; }
};


// construct pool before constructing EditEngine, destroy pool after EditEngine
class ScEnginePoolHelper
{
protected:
    SfxItemPool*	pEnginePool;
    SfxItemSet*		pDefaults;
    BOOL			bDeleteEnginePool;
    BOOL			bDeleteDefaults;

                    ScEnginePoolHelper( SfxItemPool* pEnginePool,
                        BOOL bDeleteEnginePool = FALSE );
                    ScEnginePoolHelper( const ScEnginePoolHelper& rOrg );
    virtual			~ScEnginePoolHelper();
};


class ScEditEngineDefaulter : public ScEnginePoolHelper, public EditEngine
{
public:
                    /// bDeleteEnginePool: Engine becomes the owner of the pool
                    /// and deletes it on destruction
                    ScEditEngineDefaulter( SfxItemPool* pEnginePool,
                        BOOL bDeleteEnginePool = FALSE );
                    /// If rOrg.bDeleteEnginePool: pool gets cloned and will be
                    /// deleted on destruction. Defaults are not set.
                    ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg );
    virtual			~ScEditEngineDefaulter();

                    /// Creates a copy of SfxItemSet if bRememberCopy set
    void			SetDefaults( const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );

                    /// Becomes the owner of the SfxItemSet if bTakeOwnership set
    void			SetDefaults( SfxItemSet* pDefaults, BOOL bTakeOwnership = TRUE );

                    /// Set the item in the default ItemSet which is created
                    /// if it doesn't exist yet.
                    /// The default ItemSet is then applied to each paragraph.

                    /// Overwritten method to be able to apply defaults already set
    void			SetText( const EditTextObject& rTextObject );
                    /// Current defaults are not applied, new defaults are applied
    void			SetTextNewDefaults( const EditTextObject& rTextObject,
                        const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );
                    /// Current defaults are not applied, new defaults are applied
    void			SetTextNewDefaults( const EditTextObject& rTextObject,
                        SfxItemSet* pDefaults, BOOL bTakeOwnership = TRUE );

                    /// Overwritten method to be able to apply defaults already set
    void			SetText( const String& rText );
                    /// Current defaults are not applied, new defaults are applied
    void			SetTextNewDefaults( const String& rText,
                        const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );
                    /// Current defaults are not applied, new defaults are applied
    void			SetTextNewDefaults( const String& rText,
                        SfxItemSet* pDefaults, BOOL bTakeOwnership = TRUE );

                    /// Paragraph attributes that are not defaults are copied to
                    /// character attributes and all paragraph attributes reset

                    /// Re-apply existing defaults if set, same as in SetText,
                    /// but without EnableUndo/SetUpdateMode.
    void			RepeatDefaults();
};


// 1/100 mm


struct ScHeaderFieldData
{
    String		aTitle;				// Titel oder Dateiname wenn kein Titel
    String		aLongDocName;		// Pfad und Dateiname
    String		aShortDocName;		// nur Dateiname
    String		aTabName;
    Date		aDate;
    Time		aTime;
    long		nPageNo;
    long		nTotalPages;
    SvxNumType	eNumType;

    ScHeaderFieldData();
};


// fuer Feldbefehle in der Tabelle
class ScFieldEditEngine : public ScEditEngineDefaulter
{
private:
    BOOL	bExecuteURL;

public:
    // pEnginePool = ScDocument.GetEnginePool()
    // pTextObjectPool = ScDocument.GetEditPool()
    ScFieldEditEngine( SfxItemPool* pEnginePool,
                        SfxItemPool* pTextObjectPool = NULL,
                        BOOL bDeleteEnginePool = FALSE );

    void	SetExecuteURL(BOOL bSet)	{ bExecuteURL = bSet; }

    virtual String	CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );
};


// for headers/footers with fields
class ScHeaderEditEngine : public ScEditEngineDefaulter
{
private:
    ScHeaderFieldData	aData;

public:
    ScHeaderEditEngine( SfxItemPool* pEnginePool, BOOL bDeleteEnginePool = FALSE );
    virtual String CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );

    void SetTitle(const String& rNew)				{ aData.aTitle = rNew; }
    void SetLongDocName(const String& rNew)			{ aData.aLongDocName = rNew; }
    void SetShortDocName(const String& rNew)		{ aData.aShortDocName = rNew; }
    void SetTabName(const String& rNew)				{ aData.aTabName = rNew; }
    void SetDate(const Date& rNew)					{ aData.aDate = rNew; }
    void SetTime(const Time& rNew)					{ aData.aTime = rNew; }
    void SetPageNo(long nNew)						{ aData.nPageNo = nNew; }
    void SetTotalPages(long nNew)					{ aData.nTotalPages = nNew; }
    void SetNumType(SvxNumType eNew)				{ aData.eNumType = eNew; }
    void SetData(const ScHeaderFieldData& rNew)		{ aData = rNew; }
};

//	SvxFieldData-Ableitungen sind nach Svx verschoben


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
