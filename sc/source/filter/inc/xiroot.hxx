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

#include "xlroot.hxx"
#include <memory>

// Forward declarations of objects in public use ==============================

class XclImpStream;
class XclImpString;

typedef std::shared_ptr< XclImpString > XclImpStringRef;

// Global data ================================================================

class XclImpAddressConverter;
class XclImpFormulaCompiler;
class XclImpSst;
class XclImpPalette;
class XclImpFontBuffer;
class XclImpNumFmtBuffer;
class XclImpXFBuffer;
class XclImpXFRangeBuffer;
class XclImpTabInfo;
class XclImpNameManager;
class XclImpLinkManager;
class XclImpObjectManager;
class XclImpSheetDrawing;
class XclImpCondFormatManager;
class XclImpValidationManager;
class XclImpAutoFilterBuffer;
class XclImpWebQueryBuffer;
class XclImpPivotTableManager;
class XclImpPageSettings;
class XclImpDocViewSettings;
class XclImpTabViewSettings;
class XclImpSheetProtectBuffer;
class XclImpDocProtectBuffer;

class ScRangeListTabs;
class ExcelToSc;
class ScDocumentImport;

/** Stores global buffers and data needed for Excel import filter. */
struct XclImpRootData : public XclRootData
{
    typedef std::shared_ptr< XclImpAddressConverter >    XclImpAddrConvRef;
    typedef std::shared_ptr< XclImpFormulaCompiler >     XclImpFmlaCompRef;

    typedef std::shared_ptr< XclImpSst >                 XclImpSstRef;
    typedef std::shared_ptr< XclImpPalette >             XclImpPaletteRef;
    typedef std::shared_ptr< XclImpFontBuffer >          XclImpFontBfrRef;
    typedef std::shared_ptr< XclImpNumFmtBuffer >        XclImpNumFmtBfrRef;
    typedef std::shared_ptr< XclImpXFBuffer >            XclImpXFBfrRef;
    typedef std::shared_ptr< XclImpXFRangeBuffer >       XclImpXFRangeBfrRef;
    typedef std::shared_ptr< XclImpTabInfo >             XclImpTabInfoRef;
    typedef std::shared_ptr< XclImpNameManager >         XclImpNameMgrRef;
    typedef std::shared_ptr< XclImpLinkManager >         XclImpLinkMgrRef;
    typedef std::shared_ptr< XclImpObjectManager >       XclImpObjectMgrRef;
    typedef std::shared_ptr< XclImpCondFormatManager >   XclImpCondFmtMgrRef;
    typedef std::shared_ptr< XclImpValidationManager >   XclImpValidationMgrRef;
    typedef std::shared_ptr< XclImpWebQueryBuffer >      XclImpWebQueryBfrRef;
    typedef std::shared_ptr< XclImpPivotTableManager >   XclImpPTableMgrRef;
    typedef std::shared_ptr< XclImpPageSettings >        XclImpPageSettRef;
    typedef std::shared_ptr< XclImpDocViewSettings >     XclImpDocViewSettRef;
    typedef std::shared_ptr< XclImpTabViewSettings >     XclImpTabViewSettRef;
    typedef std::shared_ptr< XclImpSheetProtectBuffer >  XclImpTabProtectRef;
    typedef std::shared_ptr< XclImpDocProtectBuffer >    XclImpDocProtectRef;

    XclImpAddrConvRef   mxAddrConv;         /// The address converter.
    XclImpFmlaCompRef   mxFmlaComp;         /// The formula compiler.

    XclImpSstRef        mxSst;              /// The shared string table.
    XclImpPaletteRef    mxPalette;          /// The color buffer.
    XclImpFontBfrRef    mxFontBfr;          /// All fonts in the file.
    XclImpNumFmtBfrRef  mxNumFmtBfr;        /// All number formats in the file.
    XclImpXFBfrRef      mpXFBfr;            /// All XF record data in the file.
    XclImpXFRangeBfrRef mxXFRangeBfr;       /// Buffer of XF index ranges in a sheet.

    XclImpTabInfoRef    mxTabInfo;          /// Sheet creation order list.
    XclImpNameMgrRef    mxNameMgr;          /// Internal defined names.
    XclImpLinkMgrRef    mxLinkMgr;          /// Manager for internal/external links.

    XclImpObjectMgrRef  mxObjMgr;           /// All drawing objects.
    XclImpCondFmtMgrRef mxCondFmtMgr;       /// Conditional formatting.
    XclImpValidationMgrRef mxValidMgr;      /// Data validation
    XclImpWebQueryBfrRef mxWebQueryBfr;     /// All web queries.
    XclImpPTableMgrRef  mxPTableMgr;        /// All pivot tables and pivot caches.

    XclImpPageSettRef   mxPageSett;         /// Page settings for current sheet.
    XclImpDocViewSettRef mxDocViewSett;     /// View settings for entire document.
    XclImpTabViewSettRef mxTabViewSett;     /// View settings for current sheet.
    XclImpTabProtectRef mxTabProtect;       /// Sheet protection options for current sheet.
    XclImpDocProtectRef mxDocProtect;       /// Document protection options.

    std::shared_ptr<ScDocumentImport> mxDocImport;

    std::unique_ptr<ScRangeListTabs>  mpPrintRanges;
    std::unique_ptr<ScRangeListTabs>  mpPrintTitles;

    bool                mbHasCodePage;      /// true = CODEPAGE record exists.
    bool                mbHasBasic;         /// true = document contains VB project.

    explicit            XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            const tools::SvRef<SotStorage>& xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc );
    virtual             ~XclImpRootData() override;
};

/** Access to global data from other classes. */
class XclImpRoot : public XclRoot
{
public:
    explicit            XclImpRoot( XclImpRootData& rImpRootData );

    /** Returns this root instance - for code readability in derived classes. */
    const XclImpRoot& GetRoot() const { return *this; }
    XclImpRoot& GetRoot() { return *this; }

    /** Sets a code page read from a CODEPAGE record for byte string import. */
    void                SetCodePage( sal_uInt16 nCodePage );

    /** Is called when import filter starts importing a single sheet (all BIFF versions). */
    void                InitializeTable( SCTAB nScTab );
    /** Is called when import filter stops importing a single sheet (all BIFF versions). */
    void                FinalizeTable();

    /** Returns the address converter. */
    XclImpAddressConverter& GetAddressConverter() const;
    /** Returns the formula converter. */
    XclImpFormulaCompiler& GetFormulaCompiler() const;
    /** Returns the old formula converter. */
    ExcelToSc&          GetOldFmlaConverter() const;

    /** Returns the shared string table. */
    XclImpSst&          GetSst() const;
    /** Returns the color buffer. */
    XclImpPalette&      GetPalette() const;
    /** Returns the font buffer. */
    XclImpFontBuffer&   GetFontBuffer() const;
    /** Returns the number format buffer. */
    XclImpNumFmtBuffer& GetNumFmtBuffer() const;
    /** Returns the cell formatting attributes buffer. */
    XclImpXFBuffer&     GetXFBuffer() const;
    /** Returns the buffer of XF index ranges for a sheet. */
    XclImpXFRangeBuffer& GetXFRangeBuffer() const;

    /** Returns the buffer that contains all print areas in the document. */
    ScRangeListTabs&   GetPrintAreaBuffer() const;
    /** Returns the buffer that contains all print titles in the document. */
    ScRangeListTabs&   GetTitleAreaBuffer() const;

    /** Returns the buffer that contains the sheet creation order. */
    XclImpTabInfo&      GetTabInfo() const;
    /** Returns the buffer that contains internal defined names. */
    XclImpNameManager&  GetNameManager() const;
    /** Returns the link manager. */
    XclImpLinkManager&  GetLinkManager() const;

    /** Returns the drawing object manager. */
    XclImpObjectManager& GetObjectManager() const;
    /** Returns the drawing container of the current sheet. */
    XclImpSheetDrawing& GetCurrSheetDrawing() const;
    /** Returns the conditional formatting manager. */
    XclImpCondFormatManager& GetCondFormatManager() const;

    XclImpValidationManager& GetValidationManager() const;
    /** Returns the filter manager. */
    XclImpAutoFilterBuffer& GetFilterManager() const;
    /** Returns the web query buffer. */
    XclImpWebQueryBuffer& GetWebQueryBuffer() const;
    /** Returns the pivot table manager. */
    XclImpPivotTableManager& GetPivotTableManager() const;
    /** Returns the sheet protection options of the current sheet. */
    XclImpSheetProtectBuffer& GetSheetProtectBuffer() const;
    /** Returns the document protection options. */
    XclImpDocProtectBuffer& GetDocProtectBuffer() const;

    /** Returns the page settings of the current sheet. */
    XclImpPageSettings& GetPageSettings() const;
    /** Returns the view settings of the entire document. */
    XclImpDocViewSettings& GetDocViewSettings() const;
    /** Returns the view settings of the current sheet. */
    XclImpTabViewSettings& GetTabViewSettings() const;

    /** Returns the Calc add-in function name for an Excel function name. */
    static OUString      GetScAddInName( const OUString& rXclName );

    /** Returns true, if the document contains a VB project. */
    bool         HasBasic() const { return mrImpData.mbHasBasic; }
    /** Called to indicate that the document contains a VB project. */
    void         SetHasBasic() { mrImpData.mbHasBasic = true; }
    /** Reads the CODENAME record and inserts the codename into the document. */
    void                ReadCodeName( XclImpStream& rStrm, bool bGlobals );

    ScDocumentImport& GetDocImport();

private:
    XclImpRootData& mrImpData;      /// Reference to the global import data struct.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
