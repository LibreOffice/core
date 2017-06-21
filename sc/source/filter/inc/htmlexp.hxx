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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_HTMLEXP_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_HTMLEXP_HXX

#include "global.hxx"
#include <rtl/textenc.h>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vector>
#include <memory>

#include "expbase.hxx"

class ScDocument;
class SfxItemSet;
class SdrPage;
class Graphic;
class SdrObject;
class OutputDevice;
class ScDrawLayer;
class EditTextObject;
enum class SvtScriptType;
namespace editeng { class SvxBorderLine; }

struct ScHTMLStyle
{   // Defaults from stylesheet
    Color               aBackgroundColor;
    OUString            aFontFamilyName;
    sal_uInt32          nFontHeight;        // Item-Value
    sal_uInt16          nFontSizeNumber;    // HTML value 1-7
    SvtScriptType       nDefaultScriptType; // Font values are valid for the default script type
    bool                bInitialized;

    ScHTMLStyle() :
        nFontHeight(0),
        nFontSizeNumber(2),
        nDefaultScriptType(),
        bInitialized(false)
    {}

    const ScHTMLStyle& operator=( const ScHTMLStyle& rScHTMLStyle )
    {
        aBackgroundColor   = rScHTMLStyle.aBackgroundColor;
        aFontFamilyName    = rScHTMLStyle.aFontFamilyName;
        nFontHeight        = rScHTMLStyle.nFontHeight;
        nFontSizeNumber    = rScHTMLStyle.nFontSizeNumber;
        nDefaultScriptType = rScHTMLStyle.nDefaultScriptType;
        bInitialized       = rScHTMLStyle.bInitialized;
        return *this;
    }
};

struct ScHTMLGraphEntry
{
    ScRange             aRange;         // mapped range
    Size                aSize;          // size in pixels
    Size                aSpace;         // spacing in pixels
    SdrObject*          pObject;
    bool                bInCell;        // if output is in cell
    bool                bWritten;

    ScHTMLGraphEntry( SdrObject* pObj, const ScRange& rRange,
                      const Size& rSize,  bool bIn, const Size& rSpace ) :
        aRange( rRange ),
        aSize( rSize ),
        aSpace( rSpace ),
        pObject( pObj ),
        bInCell( bIn ),
        bWritten( false )
    {}
};

#define SC_HTML_FONTSIZES 7
const short nIndentMax = 23;

class ScHTMLExport : public ScExportBase
{
    // default HtmlFontSz[1-7]
    static const sal_uInt16 nDefaultFontSize[SC_HTML_FONTSIZES];
    // HtmlFontSz[1-7] in s*3.ini [user]
    static sal_uInt16       nFontSize[SC_HTML_FONTSIZES];
    static const char*  pFontSizeCss[SC_HTML_FONTSIZES];
    static const sal_uInt16 nCellSpacing;
    static const sal_Char sIndentSource[];

    typedef std::unique_ptr<std::map<OUString, OUString> > FileNameMapPtr;
    typedef std::vector<ScHTMLGraphEntry> GraphEntryList;

    GraphEntryList   aGraphList;
    ScHTMLStyle      aHTMLStyle;
    OUString         aBaseURL;
    OUString         aStreamPath;
    OUString         aCId;           // Content-Id for Mail-Export
    VclPtr<OutputDevice> pAppWin;        // for Pixel-work
    FileNameMapPtr   pFileNameMap;        // for CopyLocalFileToINet
    OUString         aNonConvertibleChars;   // collect nonconvertible characters
    rtl_TextEncoding eDestEnc;
    SCTAB            nUsedTables;
    short            nIndent;
    sal_Char         sIndent[nIndentMax+1];
    bool             bAll;           // whole document
    bool             bTabHasGraphics;
    bool             bTabAlignedLeft;
    bool             bCalcAsShown;
    bool             bCopyLocalFileToINet;
    bool             bTableDataHeight;
    bool             mbSkipImages;
    /// If HTML header and footer should be written as well, or just the content itself.
    bool             mbSkipHeaderFooter;

    const SfxItemSet& PageDefaults( SCTAB nTab );

    void WriteBody();
    void WriteHeader();
    void WriteOverview();
    void WriteTables();
    void WriteCell( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void WriteGraphEntry( ScHTMLGraphEntry* );
    void WriteImage( OUString& rLinkName,
                     const Graphic&, const OString& rImgOptions,
                     XOutFlags nXOutFlags = XOutFlags::NONE );
            // nXOutFlags for XOutBitmap::WriteGraphic

    // write to stream if and only if URL fields in edit cell
    bool WriteFieldText( const EditTextObject* pData );

    // copy a local file to internet if needed
    void CopyLocalFileToINet( OUString& rFileNm, const OUString& rTargetNm );
    bool HasCId()
    {
        return !aCId.isEmpty();
    }
    void MakeCIdURL( OUString& rURL );

    void PrepareGraphics( ScDrawLayer*, SCTAB nTab,
                          SCCOL nStartCol, SCROW nStartRow,
                          SCCOL nEndCol, SCROW nEndRow );

    void FillGraphList( const SdrPage*, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow );

    static OString BorderToStyle(const char* pBorderName,
                          const editeng::SvxBorderLine* pLine,
                          bool& bInsertSemicolon);

    static sal_uInt16  GetFontSizeNumber( sal_uInt16 nHeight );
    static const char* GetFontSizeCss( sal_uInt16 nHeight );
    sal_uInt16  ToPixel( sal_uInt16 nTwips );
    Size        MMToPixel( const Size& r100thMMSize );
    void        IncIndent( short nVal );

    const sal_Char* GetIndentStr()
    {
        return sIndent;
    }

public:
                        ScHTMLExport( SvStream&, const OUString&, ScDocument*, const ScRange&,
                                      bool bAll, const OUString& aStreamPath, const OUString& rFilterOptions );
    virtual             ~ScHTMLExport() override;
    void                Write();
    const OUString&     GetNonConvertibleChars() const
    {
        return aNonConvertibleChars;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
