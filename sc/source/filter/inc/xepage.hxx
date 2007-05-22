/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xepage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:56:05 $
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

#ifndef SC_XEPAGE_HXX
#define SC_XEPAGE_HXX

#ifndef SC_XLPAGE_HXX
#include "xlpage.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

// Page settings records ======================================================

// Header/footer --------------------------------------------------------------

/** Represents a HEADER or FOOTER record. */
class XclExpHeaderFooter : public XclExpRecord
{
public:
    explicit            XclExpHeaderFooter( sal_uInt16 nRecId, const String& rHdrString );

private:
    /** Writes the header or footer string. Writes an empty record, if no header/footer present. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    String              maHdrString;        /// Header or footer contents.
};

// General page settings ------------------------------------------------------

/** Represents a SETUP record that contains common page settings. */
class XclExpSetup : public XclExpRecord
{
public:
    explicit            XclExpSetup( const XclPageData& rPageData );

private:
    /** Writes the contents of the SETUP record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const XclPageData&  mrData;             /// Page settings data of current sheet.
};

// Manual page breaks ---------------------------------------------------------

/** Stores an array of manual page breaks for columns or rows. */
class XclExpPageBreaks : public XclExpRecord
{
public:
    explicit            XclExpPageBreaks(
                            sal_uInt16 nRecId,
                            const ScfUInt16Vec& rPageBreaks,
                            sal_uInt16 nMaxPos );

    /** Writes the record, if the list is not empty. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Writes the page break list. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const ScfUInt16Vec& mrPageBreaks;       /// Page settings data of current sheet.
    sal_uInt16          mnMaxPos;           /// Maximum row/column for BIFF8 page breaks.
};

// Background bitmap ----------------------------------------------------------

class Graphic;

/** Provides export of a background bitmap of a sheet (record BITMAP). */
class XclExpBitmap : public XclExpRecordBase
{
public:
    explicit            XclExpBitmap( const Graphic& rGraphic );

    /** Writes the BITMAP record. */
    virtual void        Save( XclExpStream& rStrm );

private:
    const Graphic&      mrGraphic;      /// The VCL graphic.
};

// Page settings ==============================================================

/** Contains all page (print) settings records for a single sheet. */
class XclExpPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit            XclExpPageSettings( const XclExpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData& GetPageData() const { return maData; }

    /** Writes all page settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    XclPageData         maData;         /// Page settings data.
};

// ----------------------------------------------------------------------------

/** Contains all page (print) settings records for a chart object. */
class XclExpChartPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit            XclExpChartPageSettings( const XclExpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData& GetPageData() const { return maData; }

    /** Writes all page settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    XclPageData         maData;         /// Page settings data.
};

// ============================================================================

#endif

