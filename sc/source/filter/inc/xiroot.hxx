/*************************************************************************
 *
 *  $RCSfile: xiroot.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:12:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

// Global data ================================================================

class XclImpSst;
class XclImpPalette;
class XclImpFontBuffer;
class XclImpNumFmtBuffer;
class XclImpXFBuffer;
class XclImpXFRangeBuffer;
class XclImpPageSettings;
class _ScRangeListTabs;
class XclImpTabInfo;
class XclImpNameManager;
class XclImpLinkManager;
class XclImpObjectManager;
class XclImpCondFormatManager;
class XclImpAutoFilterBuffer;
class XclImpWebQueryBuffer;
class XclImpPivotTableManager;

/** Stores global buffers and data needed for Excel import filter. */
struct XclImpRootData : public XclRootData
{
    typedef ::std::auto_ptr< XclImpSst >                XclImpSstPtr;
    typedef ::std::auto_ptr< XclImpPalette >            XclImpPalettePtr;
    typedef ::std::auto_ptr< XclImpFontBuffer >         XclImpFontBfrPtr;
    typedef ::std::auto_ptr< XclImpNumFmtBuffer >       XclImpNumFmtBfrPtr;
    typedef ::std::auto_ptr< XclImpXFBuffer >           XclImpXFBfrPtr;
    typedef ::std::auto_ptr< XclImpXFRangeBuffer >      XclImpXFRangeBfrPtr;
    typedef ::std::auto_ptr< XclImpPageSettings >       XclImpPageSettPtr;
    typedef ::std::auto_ptr< XclImpTabInfo >            XclImpTabInfoPtr;
    typedef ::std::auto_ptr< XclImpNameManager >        XclImpNameMgrPtr;
    typedef ::std::auto_ptr< XclImpLinkManager >        XclImpLinkMgrPtr;
    typedef ::std::auto_ptr< XclImpObjectManager >      XclImpObjectMgrPtr;
    typedef ::std::auto_ptr< XclImpCondFormatManager >  XclImpCondFmtMgrPtr;
    typedef ::std::auto_ptr< XclImpWebQueryBuffer >     XclImpWebQueryBfrPtr;
    typedef ::std::auto_ptr< XclImpPivotTableManager >  XclImpPTableMgrPtr;

    XclImpSstPtr        mxSst;              /// The shared string table.

    XclImpPalettePtr    mxPalette;          /// The color buffer.
    XclImpFontBfrPtr    mxFontBfr;          /// All fonts in the file.
    XclImpNumFmtBfrPtr  mxNumFmtBfr;        /// All number formats in the file.
    XclImpXFBfrPtr      mpXFBfr;            /// All XF record data in the file.
    XclImpXFRangeBfrPtr mxXFRangeBfr;       /// Buffer of XF index ranges in a sheet.

    XclImpPageSettPtr   mxPageSettings;     /// Page settings for current sheet.

    XclImpTabInfoPtr    mxTabInfo;          /// Sheet creation order list.
    XclImpNameMgrPtr    mxNameMgr;          /// Internal defined names.
    XclImpLinkMgrPtr    mxLinkMgr;          /// Manager for internal/external links.

    XclImpObjectMgrPtr  mxObjMgr;           /// All drawing objects.
    XclImpCondFmtMgrPtr mxCondFmtMgr;       /// Conditional formattings.
    XclImpWebQueryBfrPtr mxWebQueryBfr;     /// All web queries.
    XclImpPTableMgrPtr  mxPTableMgr;        /// All pivot tables and pivot caches.

    explicit            XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            SotStorageRef xRootStrg, SvStream& rBookStrm,
                            ScDocument& rDoc, CharSet eCharSet );
    virtual             ~XclImpRootData();
};

// ----------------------------------------------------------------------------

class ExcelToSc;

/** Access to global data from other classes. */
class XclImpRoot : public XclRoot
{
public:
    explicit            XclImpRoot( XclImpRootData& rImpRootData );

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclImpRoot& GetRoot() const { return *this; }

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

    /** Returns the page settings of the current sheet. */
    XclImpPageSettings& GetPageSettings() const;
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

    /** Returns the formula converter. */
    ExcelToSc&          GetFmlaConverter() const;

    /** Returns the Calc add-in function name for an Excel function name. */
    String              GetScAddInName( const String& rXclName ) const;

    /** Checks if the passed cell address is a valid Calc cell position.
        @descr  See XclRoot::CheckCellAddress for details. */
    bool                CheckCellAddress( const ScAddress& rPos ) const;
    /** Checks and eventually crops the cell range to valid Calc dimensions.
        @descr  See XclRoot::CheckCellRange for details. */
    bool                CheckCellRange( ScRange& rRange ) const;
    /** Checks and eventually crops the cell ranges to valid Calc dimensions.
        @descr  See XclRoot::CheckCellRangeList for details. */
    void                CheckCellRangeList( ScRangeList& rRanges ) const;

private:
    mutable XclImpRootData& mrImpData;      /// Reference to the global import data struct.
};

// ============================================================================

#endif

