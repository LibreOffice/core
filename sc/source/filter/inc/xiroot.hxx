/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiroot.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:00:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XIROOT_HXX
#define SC_XIROOT_HXX

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

// Forward declarations of objects in public use ==============================

class XclImpStream;
class XclImpString;

typedef ScfRef< XclImpString > XclImpStringRef;

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
class XclImpCondFormatManager;
class XclImpAutoFilterBuffer;
class XclImpWebQueryBuffer;
class XclImpPivotTableManager;
class XclImpPageSettings;
class XclImpDocViewSettings;
class XclImpTabViewSettings;

class _ScRangeListTabs;
class ExcelToSc;

/** Stores global buffers and data needed for Excel import filter. */
struct XclImpRootData : public XclRootData
{
    typedef ScfRef< XclImpAddressConverter >    XclImpAddrConvRef;
    typedef ScfRef< XclImpFormulaCompiler >     XclImpFmlaCompRef;

    typedef ScfRef< XclImpSst >                 XclImpSstRef;
    typedef ScfRef< XclImpPalette >             XclImpPaletteRef;
    typedef ScfRef< XclImpFontBuffer >          XclImpFontBfrRef;
    typedef ScfRef< XclImpNumFmtBuffer >        XclImpNumFmtBfrRef;
    typedef ScfRef< XclImpXFBuffer >            XclImpXFBfrRef;
    typedef ScfRef< XclImpXFRangeBuffer >       XclImpXFRangeBfrRef;
    typedef ScfRef< XclImpTabInfo >             XclImpTabInfoRef;
    typedef ScfRef< XclImpNameManager >         XclImpNameMgrRef;
    typedef ScfRef< XclImpLinkManager >         XclImpLinkMgrRef;
    typedef ScfRef< XclImpObjectManager >       XclImpObjectMgrRef;
    typedef ScfRef< XclImpCondFormatManager >   XclImpCondFmtMgrRef;
    typedef ScfRef< XclImpWebQueryBuffer >      XclImpWebQueryBfrRef;
    typedef ScfRef< XclImpPivotTableManager >   XclImpPTableMgrRef;
    typedef ScfRef< XclImpPageSettings >        XclImpPageSettRef;
    typedef ScfRef< XclImpDocViewSettings >     XclImpDocViewSettRef;
    typedef ScfRef< XclImpTabViewSettings >     XclImpTabViewSettRef;

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
    XclImpWebQueryBfrRef mxWebQueryBfr;     /// All web queries.
    XclImpPTableMgrRef  mxPTableMgr;        /// All pivot tables and pivot caches.

    XclImpPageSettRef   mxPageSett;         /// Page settings for current sheet.
    XclImpDocViewSettRef mxDocViewSett;     /// View settings for entire document.
    XclImpTabViewSettRef mxTabViewSett;     /// View settings for current sheet.

    explicit            XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            SotStorageRef xRootStrg, ScDocument& rDoc, CharSet eCharSet );
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
    /** Returns the conditional formattings manager. */
    XclImpCondFormatManager& GetCondFormatManager() const;
    /** Returns the filter manager. */
    XclImpAutoFilterBuffer& GetFilterManager() const;
    /** Returns the web query buffer. */
    XclImpWebQueryBuffer& GetWebQueryBuffer() const;
    /** Returns the pivot table manager. */
    XclImpPivotTableManager& GetPivotTableManager() const;

    /** Returns the page settings of the current sheet. */
    XclImpPageSettings& GetPageSettings() const;
    /** Returns the view settings of the entire document. */
    XclImpDocViewSettings& GetDocViewSettings() const;
    /** Returns the view settings of the current sheet. */
    XclImpTabViewSettings& GetTabViewSettings() const;

    /** Returns the Calc add-in function name for an Excel function name. */
    String              GetScAddInName( const String& rXclName ) const;

private:
    mutable XclImpRootData& mrImpData;      /// Reference to the global import data struct.
};

// ============================================================================

#endif

