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

#pragma once

#include <memory>
#include "scdllapi.h"
#include "types.hxx"
#include <editeng/editeng.hxx>
#include <svx/pageitem.hxx>
#include <tools/datetime.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <vcl/outdev.hxx>

class ScDocument;
class ScPatternAttr;
class ScEditEngineDefaulter;

class ScEditUtil
{
    ScDocument*     pDoc;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    Point           aCellPos;
    VclPtr<OutputDevice> pDev; // MapMode has to be set
    double          nPPTX;
    double          nPPTY;
    Fraction        aZoomX;
    Fraction        aZoomY;
    bool            bInPrintTwips;

public:
    static OUString ModifyDelimiters( const OUString& rOld );

    /// Retrieves string with paragraphs delimited by spaces
    static OUString GetSpaceDelimitedString( const EditEngine& rEngine );

    /// Retrieves string with paragraphs delimited by new lines ('\n').
    static OUString GetMultilineString( const EditEngine& rEngine );
    static OUString GetMultilineString( const EditTextObject& rEdit );

    /** Retrieves string with paragraphs delimited by new lines ('\n').

        @param pDoc
               If not NULL, use pDoc->GetEditEngine() to retrieve field content.
               If NULL, a static mutex-guarded ScFieldEditEngine is used that
               is not capable of resolving document specific fields; avoid.
     */
    SC_DLLPUBLIC static OUString GetString( const EditTextObject& rEditText, const ScDocument* pDoc );

    static std::unique_ptr<EditTextObject> CreateURLObjectFromURL(
        ScDocument& rDoc, const OUString& rURL, const OUString& rText );

    static void RemoveCharAttribs( EditTextObject& rEditText, const ScPatternAttr& rAttr );

    static std::unique_ptr<EditTextObject> Clone( const EditTextObject& rSrc, ScDocument& rDestDoc );

    static OUString GetCellFieldValue(
        const SvxFieldData& rFieldData, const ScDocument* pDoc, std::optional<Color>* ppTextColor );

public:
                ScEditUtil( ScDocument* pDocument, SCCOL nX, SCROW nY, SCTAB nZ,
                            const Point& rCellPos,
                            OutputDevice* pDevice, double nScaleX, double nScaleY,
                            const Fraction& rX, const Fraction& rY, bool bPrintTwips = false );

    tools::Rectangle   GetEditArea( const ScPatternAttr* pPattern, bool bForceToTop );
    tools::Long GetIndent(const ScPatternAttr* pPattern) const;
    void GetMargins(const ScPatternAttr* pPattern, tools::Long& nLeftMargin, tools::Long& nTopMargin,
                    tools::Long& nRightMargin, tools::Long& BottomMargin) const;

};

class ScEditAttrTester
{
    std::unique_ptr<SfxItemSet> pEditAttrs;
    bool        bNeedsObject;
    bool        bNeedsCellAttr;

public:
                ScEditAttrTester( ScEditEngineDefaulter* pEng );
                ~ScEditAttrTester();

    bool                NeedsObject() const     { return bNeedsObject; }
    bool                NeedsCellAttr() const   { return bNeedsCellAttr; }
    const SfxItemSet&   GetAttribs() const      { return *pEditAttrs; }
};

// construct pool before constructing EditEngine, destroy pool after EditEngine
class ScEnginePoolHelper
{
protected:
    rtl::Reference<SfxItemPool> pEnginePool;
    SfxItemSet*     pDefaults;
    bool            bDeleteEnginePool;
    bool            bDeleteDefaults;

                    ScEnginePoolHelper( SfxItemPool* pEnginePool, bool bDeleteEnginePool );
                    ScEnginePoolHelper( const ScEnginePoolHelper& rOrg );
    virtual         ~ScEnginePoolHelper();
};

class SC_DLLPUBLIC ScEditEngineDefaulter : public ScEnginePoolHelper, public EditEngine
{
public:
    using EditEngine::SetText;
                    /// bDeleteEnginePool: Engine becomes the owner of the pool
                    /// and deletes it on destruction
                    ScEditEngineDefaulter( SfxItemPool* pEnginePool,
                        bool bDeleteEnginePool = false );
                    /// If rOrg.bDeleteEnginePool: pool gets cloned and will be
                    /// deleted on destruction. Defaults are not set.
                    ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg );
    virtual         ~ScEditEngineDefaulter() override;

                    /// Creates a copy of SfxItemSet if bRememberCopy set
    void            SetDefaults( const SfxItemSet& rDefaults, bool bRememberCopy = true );

                    /// Becomes the owner of the SfxItemSet
    void            SetDefaults( std::unique_ptr<SfxItemSet> pDefaults );

                    /// Set the item in the default ItemSet which is created
                    /// if it doesn't exist yet.
                    /// The default ItemSet is then applied to each paragraph.
    void            SetDefaultItem( const SfxPoolItem& rItem );

                    /// Returns the stored defaults, used to find non-default character attributes
    const SfxItemSet& GetDefaults();

                    /// SetText and apply defaults already set
    void            SetTextCurrentDefaults( const EditTextObject& rTextObject );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const EditTextObject& rTextObject,
                        const SfxItemSet& rDefaults, bool bRememberCopy = true );

                    /// SetText and apply defaults already set
    void            SetTextCurrentDefaults( const OUString& rText );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const OUString& rText,
                        const SfxItemSet& rDefaults );

                    /// Paragraph attributes that are not defaults are copied to
                    /// character attributes and all paragraph attributes reset
    void            RemoveParaAttribs();

                    /// Re-apply existing defaults if set, same as in SetText,
                    /// but without EnableUndo/SetUpdateMode.
    void            RepeatDefaults();
};

// for field commands (or just fields?) in a table
class SC_DLLPUBLIC ScFieldEditEngine : public ScEditEngineDefaulter
{
private:
    ScDocument* mpDoc;
    bool bExecuteURL;

public:
    ScFieldEditEngine(
        ScDocument* pDoc, SfxItemPool* pEnginePool, SfxItemPool* pTextObjectPool = nullptr,
        bool bDeleteEnginePool = false);

    void SetExecuteURL(bool bSet)    { bExecuteURL = bSet; }

    virtual bool    FieldClicked( const SvxFieldItem& rField ) override;
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor ) override;
};

// 1/100 mm
class SC_DLLPUBLIC ScTabEditEngine final : public ScFieldEditEngine
{
private:
    void    Init(const ScPatternAttr& rPattern);
public:
    ScTabEditEngine( ScDocument* pDoc );            // Default
    ScTabEditEngine(const ScPatternAttr& rPattern,
                    SfxItemPool *pEngineItemPool, ScDocument *pDoc,
                    SfxItemPool* pTextObjectPool = nullptr );
};

struct ScHeaderFieldData
{
    OUString    aTitle;             // title or file name (if no title)
    OUString    aLongDocName;       // path and file name
    OUString    aShortDocName;      // pure file name
    OUString    aTabName;
    DateTime    aDateTime;
    tools::Long        nPageNo;
    tools::Long        nTotalPages;
    SvxNumType  eNumType;

    ScHeaderFieldData();
};

// for headers/footers with fields
class SC_DLLPUBLIC ScHeaderEditEngine final : public ScEditEngineDefaulter
{
private:
    ScHeaderFieldData   aData;

public:
    ScHeaderEditEngine( SfxItemPool* pEnginePool );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor ) override;

    void SetNumType(SvxNumType eNew)                { aData.eNumType = eNew; }
    void SetData(const ScHeaderFieldData& rNew)     { aData = rNew; }
};

// for Note text objects.
class ScNoteEditEngine final : public ScEditEngineDefaulter
{

public:
    ScNoteEditEngine( SfxItemPool* pEnginePool, SfxItemPool* pTextObjectPool );

};

//  SvxFieldData derivations were moved to Svx (comment can be deleted?)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
