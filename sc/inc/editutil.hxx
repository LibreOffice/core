/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_EDITUTIL_HXX
#define SC_EDITUTIL_HXX

#include "scdllapi.h"
#include "address.hxx"
#include <editeng/editeng.hxx>
#include <svx/pageitem.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>


class OutputDevice;
class ScDocument;
class ScPatternAttr;
class ScEditEngineDefaulter;

class ScEditUtil
{
    ScDocument*     pDoc;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    Point           aScrPos;
    OutputDevice*   pDev;           // MapMode has to be set
    double          nPPTX;
    double          nPPTY;
    Fraction        aZoomX;
    Fraction        aZoomY;

    static const char pCalcDelimiters[];

public:
    static OUString ModifyDelimiters( const OUString& rOld );

    /// Retrieves string with paragraphs delimited by spaces
    static OUString GetSpaceDelimitedString( const EditEngine& rEngine );

    /// Retrieves string with paragraphs delimited by new lines ('\n').
    static OUString GetMultilineString( const EditEngine& rEngine );

    /** Retrieves string with paragraphs delimited by new lines ('\n').

        @param pDoc
               If not NULL, use pDoc->GetEditEngine() to retrieve field content.
               If NULL, a static mutex-guarded ScFieldEditEngine is used that
               is not capable of resolving document specific fields; avoid.
     */
    SC_DLLPUBLIC static OUString GetString( const EditTextObject& rEditText, const ScDocument* pDoc );

    static EditTextObject* CreateURLObjectFromURL(
        ScDocument& rDoc, const OUString& rURL, const OUString& rText );

    static void RemoveCharAttribs( EditTextObject& rEditText, const ScPatternAttr& rAttr );

    static EditTextObject* Clone( const EditTextObject& rSrc, ScDocument& rDestDoc );

public:
                ScEditUtil( ScDocument* pDocument, SCCOL nX, SCROW nY, SCTAB nZ,
                            const Point& rScrPosPixel,
                            OutputDevice* pDevice, double nScaleX, double nScaleY,
                            const Fraction& rX, const Fraction& rY ) :
                    pDoc(pDocument),nCol(nX),nRow(nY),nTab(nZ),
                    aScrPos(rScrPosPixel),pDev(pDevice),
                    nPPTX(nScaleX),nPPTY(nScaleY),aZoomX(rX),aZoomY(rY) {}

    Rectangle   GetEditArea( const ScPatternAttr* pPattern, sal_Bool bForceToTop );
};


class ScEditAttrTester
{
    ScEditEngineDefaulter* pEngine;
    SfxItemSet* pEditAttrs;
    sal_Bool        bNeedsObject;
    sal_Bool        bNeedsCellAttr;

public:
                ScEditAttrTester( ScEditEngineDefaulter* pEng );
                ~ScEditAttrTester();

    sal_Bool                NeedsObject() const     { return bNeedsObject; }
    sal_Bool                NeedsCellAttr() const   { return bNeedsCellAttr; }
    const SfxItemSet&   GetAttribs() const      { return *pEditAttrs; }
};


// construct pool before constructing EditEngine, destroy pool after EditEngine
class ScEnginePoolHelper
{
protected:
    SfxItemPool*    pEnginePool;
    SfxItemSet*     pDefaults;
    sal_Bool            bDeleteEnginePool;
    sal_Bool            bDeleteDefaults;

                    ScEnginePoolHelper( SfxItemPool* pEnginePool,
                        sal_Bool bDeleteEnginePool = false );
                    ScEnginePoolHelper( const ScEnginePoolHelper& rOrg );
    virtual         ~ScEnginePoolHelper();
};


class SC_DLLPUBLIC ScEditEngineDefaulter : public ScEnginePoolHelper, public EditEngine
{
private:
    using EditEngine::SetText;

public:
                    /// bDeleteEnginePool: Engine becomes the owner of the pool
                    /// and deletes it on destruction
                    ScEditEngineDefaulter( SfxItemPool* pEnginePool,
                        sal_Bool bDeleteEnginePool = false );
                    /// If rOrg.bDeleteEnginePool: pool gets cloned and will be
                    /// deleted on destruction. Defaults are not set.
                    ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg );
    virtual         ~ScEditEngineDefaulter();

                    /// Creates a copy of SfxItemSet if bRememberCopy set
    void            SetDefaults( const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );

                    /// Becomes the owner of the SfxItemSet if bTakeOwnership set
    void            SetDefaults( SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

                    /// Set the item in the default ItemSet which is created
                    /// if it doesn't exist yet.
                    /// The default ItemSet is then applied to each paragraph.
    void            SetDefaultItem( const SfxPoolItem& rItem );

                    /// Returns the stored defaults, used to find non-default character attributes
    const SfxItemSet& GetDefaults();

                    /// Overwritten method to be able to apply defaults already set
    void            SetText( const EditTextObject& rTextObject );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const EditTextObject& rTextObject,
                        const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const EditTextObject& rTextObject,
                        SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

                    /// Overwritten method to be able to apply defaults already set
    void            SetText( const OUString& rText );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const String& rText,
                        const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const String& rText,
                        SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

                    /// Paragraph attributes that are not defaults are copied to
                    /// character attributes and all paragraph attributes reset
    void            RemoveParaAttribs();

                    /// Re-apply existing defaults if set, same as in SetText,
                    /// but without EnableUndo/SetUpdateMode.
    void            RepeatDefaults();
};


// 1/100 mm
class SC_DLLPUBLIC ScTabEditEngine : public ScEditEngineDefaulter
{
private:
    void    Init(const ScPatternAttr& rPattern);
public:
    ScTabEditEngine( ScDocument* pDoc );            // Default
    ScTabEditEngine( const ScPatternAttr& rPattern,
                    SfxItemPool* pEnginePool,
                    SfxItemPool* pTextObjectPool = NULL );
};


struct ScHeaderFieldData
{
    String      aTitle;             // title or file name (if no title)
    String      aLongDocName;       // path and file name
    String      aShortDocName;      // pure file name
    String      aTabName;
    Date        aDate;
    Time        aTime;
    long        nPageNo;
    long        nTotalPages;
    SvxNumType  eNumType;

    ScHeaderFieldData();
};


// for field commands (or just fields?) in a table
class SC_DLLPUBLIC ScFieldEditEngine : public ScEditEngineDefaulter
{
private:
    ScDocument* mpDoc;
    bool bExecuteURL;

public:
    ScFieldEditEngine(
        ScDocument* pDoc, SfxItemPool* pEnginePool, SfxItemPool* pTextObjectPool = NULL,
        bool bDeleteEnginePool = false);

    void SetExecuteURL(bool bSet)    { bExecuteURL = bSet; }

    virtual void    FieldClicked( const SvxFieldItem& rField, sal_Int32, sal_uInt16 );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );
};


// for headers/footers with fields
class SC_DLLPUBLIC ScHeaderEditEngine : public ScEditEngineDefaulter
{
private:
    ScHeaderFieldData   aData;

public:
    ScHeaderEditEngine( SfxItemPool* pEnginePool, sal_Bool bDeleteEnginePool = false );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );

    void SetNumType(SvxNumType eNew)                { aData.eNumType = eNew; }
    void SetData(const ScHeaderFieldData& rNew)     { aData = rNew; }
};

// for Note text objects.
class ScNoteEditEngine : public ScEditEngineDefaulter
{

public:
    ScNoteEditEngine( SfxItemPool* pEnginePool,
                SfxItemPool* pTextObjectPool = NULL,
                sal_Bool bDeleteEnginePool = false );

};

//  SvxFieldData derivations were moved to Svx (comment can be deleted?)


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
