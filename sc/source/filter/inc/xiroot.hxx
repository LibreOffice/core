/*************************************************************************
 *
 *  $RCSfile: xiroot.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:19:54 $
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

// ============================================================================

#ifndef SC_XIROOT_HXX
#define SC_XIROOT_HXX

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif


// Global data ================================================================

class XclImpSst;
class XclImpPalette;
class XclImpFontBuffer;
class XclImpNumFmtBuffer;
class XclImpXFBuffer;
class XclImpXFIndexBuffer;
class XclImpTabIdBuffer;
class XclImpLinkManager;
class XclImpObjectManager;
class XclImpCondFormatManager;
class XclImpWebQueryBuffer;

/** Stores global buffers and data needed for Excel import filter. */
struct XclImpRootData : public XclRootData
{
    typedef ::std::auto_ptr< XclImpSst >                XclImpSstPtr;
    typedef ::std::auto_ptr< XclImpPalette >            XclImpPalettePtr;
    typedef ::std::auto_ptr< XclImpFontBuffer >         XclImpFontBufferPtr;
    typedef ::std::auto_ptr< XclImpNumFmtBuffer >       XclImpNumFmtBufferPtr;
    typedef ::std::auto_ptr< XclImpXFBuffer >           XclImpXFBufferPtr;
    typedef ::std::auto_ptr< XclImpXFIndexBuffer >      XclImpXFIndexBufferPtr;
    typedef ::std::auto_ptr< XclImpTabIdBuffer >        XclImpTabIdBufferPtr;
    typedef ::std::auto_ptr< XclImpLinkManager >        XclImpLinkManagerPtr;
    typedef ::std::auto_ptr< XclImpObjectManager >      XclImpObjectManagerPtr;
    typedef ::std::auto_ptr< XclImpCondFormatManager >  XclImpCondFormatManagerPtr;
    typedef ::std::auto_ptr< XclImpWebQueryBuffer >     XclImpWebQueryBufferPtr;

    XclImpSstPtr                mpSst;              /// The shared string table.

    XclImpPalettePtr            mpPalette;          /// The color buffer.
    XclImpFontBufferPtr         mpFontBuffer;       /// All fonts in the file.
    XclImpNumFmtBufferPtr       mpNumFmtBuffer;     /// All number formats in the file.
    XclImpXFBufferPtr           mpXFBuffer;         /// All XF record data in the file.
    XclImpXFIndexBufferPtr      mpXFIndexBuffer;    /// Buffer of XF indexes in a sheet.

    XclImpTabIdBufferPtr        mpTabIdBuffer;      /// Sheet creation order list.
    XclImpLinkManagerPtr        mpLinkManager;      /// Manager for internal/external links.

    XclImpObjectManagerPtr      mpObjManager;       /// All drawing objects.
    XclImpCondFormatManagerPtr  mpCondFmtManager;   /// Conditional formattings.
    XclImpWebQueryBufferPtr     mpWebQBuffer;       /// All web queries.

    explicit                    XclImpRootData(
                                    XclBiff eBiff,
                                    ScDocument& rDocument,
                                    const String& rDocUrl,
                                    CharSet eCharSet );
    virtual                     ~XclImpRootData();
};


// ----------------------------------------------------------------------------

class ExcelToSc;

/** Access to global data from other classes. */
class XclImpRoot : public XclRoot
{
private:
    mutable XclImpRootData&     mrImpData;      /// Reference to the global import data struct.

public:
                                XclImpRoot( const XclImpRoot& rRoot );

    XclImpRoot&                 operator=( const XclImpRoot& rRoot );

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclImpRoot&    GetRoot() const { return *this; }

    /** Returns the shared string table. */
    XclImpSst&                  GetSst() const;

    /** Returns the color buffer. */
    XclImpPalette&              GetPalette() const;
    /** Returns the font buffer. */
    XclImpFontBuffer&           GetFontBuffer() const;
    /** Returns the number format buffer. */
    XclImpNumFmtBuffer&         GetNumFmtBuffer() const;
    /** Returns the cell formatting attributes buffer. */
    XclImpXFBuffer&             GetXFBuffer() const;
    /** Returns the buffer of XF indexes for a sheet. */
    XclImpXFIndexBuffer&        GetXFIndexBuffer() const;

    /** Returns the buffer that contains the sheet creation order. */
    XclImpTabIdBuffer&          GetTabIdBuffer() const;
    /** Returns the link manager. */
    XclImpLinkManager&          GetLinkManager() const;

    /** Returns the drawing object manager. */
    XclImpObjectManager&        GetObjectManager() const;
    /** Returns the conditional formattings manager. */
    XclImpCondFormatManager&    GetCondFormatManager() const;
    /** Returns the web query buffer. */
    XclImpWebQueryBuffer&       GetWebQueryBuffer() const;

    /** Returns the formula converter. */
    ExcelToSc&                  GetFmlaConverter() const;

    /** Returns the Calc add-in function name for an Excel function name. */
    String                      GetScAddInName( const String& rXclName ) const;

    /** Checks if the passed cell address is a valid Calc cell position.
        @descr  See XclRoot::CheckCellAddress for details. */
    bool                        CheckCellAddress( const ScAddress& rPos ) const;
    /** Checks and eventually crops the cell range to valid Calc dimensions.
        @descr  See XclRoot::CheckCellRange for details. */
    bool                        CheckCellRange( ScRange& rRange ) const;
    /** Checks and eventually crops the cell ranges to valid Calc dimensions.
        @descr  See XclRoot::CheckCellRangeList for details. */
    void                        CheckCellRangeList( ScRangeList& rRanges ) const;

protected:
    explicit                    XclImpRoot( XclImpRootData& rImpRootData );

    /** Sets the BIFF version and BOF/EOF block type. */
    void                        SetBiff( XclBiff eBiff );
};


// ============================================================================

#endif

