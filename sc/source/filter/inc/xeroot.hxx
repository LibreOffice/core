/*************************************************************************
 *
 *  $RCSfile: xeroot.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2004-10-18 15:19:05 $
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

#ifndef SC_XEROOT_HXX
#define SC_XEROOT_HXX

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

// Global data ================================================================

class XclExpProgressBar;
class XclExpSst;
class XclExpPalette;
class XclExpFontBuffer;
class XclExpNumFmtBuffer;
class XclExpXFBuffer;
class XclExpTabInfo;
class XclExpLinkManager;
class XclExpPivotTableManager;

/** Stores global buffers and data needed for Excel export filter. */
struct XclExpRootData : public XclRootData
{
    typedef ::std::auto_ptr< XclExpProgressBar >        XclExpProgressPtr;
    typedef ::std::auto_ptr< XclExpSst >                XclExpSstPtr;
    typedef ::std::auto_ptr< XclExpPalette >            XclExpPalettePtr;
    typedef ::std::auto_ptr< XclExpFontBuffer >         XclExpFontBfrPtr;
    typedef ::std::auto_ptr< XclExpNumFmtBuffer >       XclExpNumFmtBfrPtr;
    typedef ::std::auto_ptr< XclExpXFBuffer >           XclExpXFBfrPtr;
    typedef ::std::auto_ptr< XclExpTabInfo >            XclExpTabInfoPtr;
    typedef ::std::auto_ptr< XclExpLinkManager >        XclExpLinkMgrPtr;
    typedef ::std::auto_ptr< XclExpPivotTableManager >  XclExpPTableMgrPtr;

    XclExpProgressPtr   mxProgress;         /// The export progress bar.

    XclExpSstPtr        mxSst;              /// The shared string table.

    XclExpPalettePtr    mxPalette;          /// The color buffer.
    XclExpFontBfrPtr    mxFontBfr;          /// All fonts in the file.
    XclExpNumFmtBfrPtr  mxNumFmtBfr;        /// All number formats in the file.
    XclExpXFBfrPtr      mxXFBfr;            /// All XF records in the file.

    XclExpTabInfoPtr    mxTabInfo;          /// Calc->Excel sheet index conversion.
    XclExpLinkMgrPtr    mxLinkMgr;          /// Manager for internal/external links.
    XclExpPTableMgrPtr  mxPTableMgr;        /// All pivot tables and pivot caches.

    bool                mbRelUrl;           /// true = Store URLs relative.

    explicit            XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            SotStorageRef xRootStrg, SvStream& rBookStrm,
                            ScDocument& rDoc, CharSet eCharSet );
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

    /** Returns the buffer for Calc->Excel sheet index conversion. */
    XclExpTabInfo&      GetTabInfo() const;
    /** Returns the link manager. */
    XclExpLinkManager&  GetLinkManager() const;
    /** Returns the pivot table manager. */
    XclExpPivotTableManager& GetPivotTableManager() const;

    /** Returns the Excel add-in function name for a Calc function name. */
    String              GetXclAddInName( const String& rScName ) const;

    /** Checks if the passed cell address is a valid Excel cell position.
        @descr  See XclRoot::CheckCellAddress for details. */
    bool                CheckCellAddress( const ScAddress& rPos ) const;
    /** Checks and eventually crops the cell range to valid Excel dimensions.
        @descr  See XclRoot::CheckCellRange for details. */
    bool                CheckCellRange( ScRange& rRange ) const;
    /** Checks and eventually crops the cell ranges to valid Excel dimensions.
        @descr  See XclRoot::CheckCellRangeList for details. */
    void                CheckCellRangeList( ScRangeList& rRanges ) const;

private:
    mutable XclExpRootData& mrExpData;      /// Reference to the global export data struct.
};

// ============================================================================

#endif

