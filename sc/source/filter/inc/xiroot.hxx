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

#ifndef SC_XIROOT_HXX
#define SC_XIROOT_HXX

#include "xlroot.hxx"
#include <boost/shared_ptr.hpp>

// Forward declarations of objects in public use ==============================

class XclImpStream;
class XclImpString;

typedef boost::shared_ptr< XclImpString > XclImpStringRef;

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

class _ScRangeListTabs;
class ExcelToSc;

/** Stores global buffers and data needed for Excel import filter. */
struct XclImpRootData : public XclRootData
{
    typedef boost::shared_ptr< XclImpAddressConverter >    XclImpAddrConvRef;
    typedef boost::shared_ptr< XclImpFormulaCompiler >     XclImpFmlaCompRef;

    typedef boost::shared_ptr< XclImpSst >                 XclImpSstRef;
    typedef boost::shared_ptr< XclImpPalette >             XclImpPaletteRef;
    typedef boost::shared_ptr< XclImpFontBuffer >          XclImpFontBfrRef;
    typedef boost::shared_ptr< XclImpNumFmtBuffer >        XclImpNumFmtBfrRef;
    typedef boost::shared_ptr< XclImpXFBuffer >            XclImpXFBfrRef;
    typedef boost::shared_ptr< XclImpXFRangeBuffer >       XclImpXFRangeBfrRef;
    typedef boost::shared_ptr< XclImpTabInfo >             XclImpTabInfoRef;
    typedef boost::shared_ptr< XclImpNameManager >         XclImpNameMgrRef;
    typedef boost::shared_ptr< XclImpLinkManager >         XclImpLinkMgrRef;
    typedef boost::shared_ptr< XclImpObjectManager >       XclImpObjectMgrRef;
    typedef boost::shared_ptr< XclImpCondFormatManager >   XclImpCondFmtMgrRef;
    typedef boost::shared_ptr< XclImpValidationManager >   XclImpValidationMgrRef;
    typedef boost::shared_ptr< XclImpWebQueryBuffer >      XclImpWebQueryBfrRef;
    typedef boost::shared_ptr< XclImpPivotTableManager >   XclImpPTableMgrRef;
    typedef boost::shared_ptr< XclImpPageSettings >        XclImpPageSettRef;
    typedef boost::shared_ptr< XclImpDocViewSettings >     XclImpDocViewSettRef;
    typedef boost::shared_ptr< XclImpTabViewSettings >     XclImpTabViewSettRef;
    typedef boost::shared_ptr< XclImpSheetProtectBuffer >  XclImpTabProtectRef;
    typedef boost::shared_ptr< XclImpDocProtectBuffer >    XclImpDocProtectRef;

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
    XclImpCondFmtMgrRef mxCondFmtMgr;       /// Conditional formattings.
    XclImpValidationMgrRef mxValidMgr;      /// Data validation
    XclImpWebQueryBfrRef mxWebQueryBfr;     /// All web queries.
    XclImpPTableMgrRef  mxPTableMgr;        /// All pivot tables and pivot caches.

    XclImpPageSettRef   mxPageSett;         /// Page settings for current sheet.
    XclImpDocViewSettRef mxDocViewSett;     /// View settings for entire document.
    XclImpTabViewSettRef mxTabViewSett;     /// View settings for current sheet.
    XclImpTabProtectRef mxTabProtect;       /// Sheet protection options for current sheet.
    XclImpDocProtectRef mxDocProtect;       /// Document protection options.

    bool                mbHasCodePage;      /// true = CODEPAGE record exists.
    bool                mbHasBasic;         /// true = document contains VB project.

    explicit            XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc );
    virtual             ~XclImpRootData();
};

// ----------------------------------------------------------------------------

/** Access to global data from other classes. */
class XclImpRoot : public XclRoot
{
public:
    explicit            XclImpRoot( XclImpRootData& rImpRootData );

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclImpRoot& GetRoot() const { return *this; }

    /** Sets a code page read from a CODEPAGE record for byte string import. */
    void                SetCodePage( sal_uInt16 nCodePage );
    /** Sets text encoding from the default application font (in case of missing CODEPAGE record). */
    void                SetAppFontEncoding( rtl_TextEncoding eAppFontEnc );

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
    _ScRangeListTabs&   GetPrintAreaBuffer() const;
    /** Returns the buffer that contains all print titles in the document. */
    _ScRangeListTabs&   GetTitleAreaBuffer() const;

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
    /** Returns the conditional formattings manager. */
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
    String              GetScAddInName( const String& rXclName ) const;

    /** Returns true, if the document contains a VB project. */
    inline bool         HasBasic() const { return mrImpData.mbHasBasic; }
    /** Called to indicate that the document contains a VB project. */
    inline void         SetHasBasic() { mrImpData.mbHasBasic = true; }
    /** Reads the CODENAME record and inserts the codename into the document. */
    void                ReadCodeName( XclImpStream& rStrm, bool bGlobals );

private:
    mutable XclImpRootData& mrImpData;      /// Reference to the global import data struct.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
