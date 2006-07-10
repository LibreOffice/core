/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeroot.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:55:52 $
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

#ifndef SC_XEROOT_HXX
#define SC_XEROOT_HXX

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

// Forward declarations of objects in public use ==============================

class XclExpStream;
class XclExpRecordBase;
class XclExpString;

typedef ScfRef< XclExpRecordBase >  XclExpRecordRef;
typedef ScfRef< XclExpString >      XclExpStringRef;

// Global data ================================================================

class XclExpTabInfo;
class XclExpAddressConverter;
class XclExpFormulaCompiler;
class XclExpProgressBar;
class XclExpSst;
class XclExpPalette;
class XclExpFontBuffer;
class XclExpNumFmtBuffer;
class XclExpXFBuffer;
class XclExpLinkManager;
class XclExpNameManager;
class XclExpFilterManager;
class XclExpPivotTableManager;

/** Stores global buffers and data needed for Excel export filter. */
struct XclExpRootData : public XclRootData
{
    typedef ScfRef< XclExpTabInfo >             XclExpTabInfoRef;
    typedef ScfRef< XclExpAddressConverter >    XclExpAddrConvRef;
    typedef ScfRef< XclExpFormulaCompiler >     XclExpFmlaCompRef;
    typedef ScfRef< XclExpProgressBar >         XclExpProgressRef;

    typedef ScfRef< XclExpSst >                 XclExpSstRef;
    typedef ScfRef< XclExpPalette >             XclExpPaletteRef;
    typedef ScfRef< XclExpFontBuffer >          XclExpFontBfrRef;
    typedef ScfRef< XclExpNumFmtBuffer >        XclExpNumFmtBfrRef;
    typedef ScfRef< XclExpXFBuffer >            XclExpXFBfrRef;
    typedef ScfRef< XclExpNameManager >         XclExpNameMgrRef;
    typedef ScfRef< XclExpLinkManager >         XclExpLinkMgrRef;
    typedef ScfRef< XclExpFilterManager >       XclExpFilterMgrRef;
    typedef ScfRef< XclExpPivotTableManager >   XclExpPTableMgrRef;

    XclExpTabInfoRef    mxTabInfo;          /// Calc->Excel sheet index conversion.
    XclExpAddrConvRef   mxAddrConv;         /// The address converter.
    XclExpFmlaCompRef   mxFmlaComp;         /// The formula compiler.
    XclExpProgressRef   mxProgress;         /// The export progress bar.

    XclExpSstRef        mxSst;              /// The shared string table.
    XclExpPaletteRef    mxPalette;          /// The color buffer.
    XclExpFontBfrRef    mxFontBfr;          /// All fonts in the file.
    XclExpNumFmtBfrRef  mxNumFmtBfr;        /// All number formats in the file.
    XclExpXFBfrRef      mxXFBfr;            /// All XF records in the file.
    XclExpNameMgrRef    mxNameMgr;          /// Internal defined names.
    XclExpLinkMgrRef    mxGlobLinkMgr;      /// Global link manager for defined names.
    XclExpLinkMgrRef    mxLocLinkMgr;       /// Local link manager for a sheet.
    XclExpFilterMgrRef  mxFilterMgr;        /// Manager for filtered areas in all sheets.
    XclExpPTableMgrRef  mxPTableMgr;        /// All pivot tables and pivot caches.

    bool                mbRelUrl;           /// true = Store URLs relative.

    explicit            XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            SotStorageRef xRootStrg, ScDocument& rDoc, CharSet eCharSet );
    virtual             ~XclExpRootData();
};

// ----------------------------------------------------------------------------

/** Access to global data from other classes. */
class XclExpRoot : public XclRoot
{
public:
    explicit            XclExpRoot( XclExpRootData& rExpRootData );

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclExpRoot& GetRoot() const { return *this; }
    /** Returns true, if URLs should be stored relative to the document location. */
    inline bool         IsRelUrl() const { return mrExpData.mbRelUrl; }

    /** Returns the buffer for Calc->Excel sheet index conversion. */
    XclExpTabInfo&      GetTabInfo() const;
    /** Returns the address converter. */
    XclExpAddressConverter& GetAddressConverter() const;
    /** Returns the formula compiler to produce formula token arrays. */
    XclExpFormulaCompiler& GetFormulaCompiler() const;
    /** Returns the export progress bar. */
    XclExpProgressBar&  GetProgressBar() const;

    /** Returns the shared string table. */
    XclExpSst&          GetSst() const;
    /** Returns the color buffer. */
    XclExpPalette&      GetPalette() const;
    /** Returns the font buffer. */
    XclExpFontBuffer&   GetFontBuffer() const;
    /** Returns the number format buffer. */
    XclExpNumFmtBuffer& GetNumFmtBuffer() const;
    /** Returns the cell formatting attributes buffer. */
    XclExpXFBuffer&     GetXFBuffer() const;
    /** Returns the global link manager for defined names. */
    XclExpLinkManager&  GetGlobalLinkManager() const;
    /** Returns the local link manager for the current sheet. */
    XclExpLinkManager&  GetLocalLinkManager() const;
    /** Returns the buffer that contains internal defined names. */
    XclExpNameManager&  GetNameManager() const;
    /** Returns the filter manager. */
    XclExpFilterManager& GetFilterManager() const;
    /** Returns the pivot table manager. */
    XclExpPivotTableManager& GetPivotTableManager() const;

    /** Is called when export filter starts to create the Excel document (all BIFF versions). */
    void                InitializeConvert();
    /** Is called when export filter starts to create the workbook global data (>=BIFF5). */
    void                InitializeGlobals();
    /** Is called when export filter starts to create data for a single sheet (all BIFF versions). */
    void                InitializeTable( SCTAB nScTab );
    /** Is called before export filter starts to write the records to the stream. */
    void                InitializeSave();
    /** Returns the reference to a record (or record list) representing a root object.
        @param nRecId  Identifier that specifies which record is returned. */
    XclExpRecordRef     CreateRecord( sal_uInt16 nRecId ) const;

private:
    /** Returns the local or global link manager, depending on current context. */
    XclExpRootData::XclExpLinkMgrRef GetLocalLinkMgrRef() const;

private:
    mutable XclExpRootData& mrExpData;      /// Reference to the global export data struct.
};

// ============================================================================

#endif

