/*************************************************************************
 *
 *  $RCSfile: xepage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:40:29 $
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

/** Represents a HEADER or FOOTER record. */
class XclExpHeaderFooter : public XclExpRecord
{
public:
    explicit                    XclExpHeaderFooter( sal_uInt16 nRecId, const String& rHdrString );

private:
    /** Writes the header or footer string. Writes an empty record, if no header/footer present. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    String                      maHdrString;        /// Header or footer contents.
};


// ----------------------------------------------------------------------------

/** Represents a SETUP record that contains common page settings. */
class XclExpSetup : public XclExpRecord
{
public:
    explicit                    XclExpSetup( const XclPageData& rPageData );

private:
    /** Writes the contents of the SETUP record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    const XclPageData&          mrData;             /// Page settings data of current sheet.
};


// Manual page breaks ---------------------------------------------------------

/** Stores an array of manual page breaks for columns or rows. */
class XclExpPageBreaks : public XclExpRecord
{
public:
    explicit                    XclExpPageBreaks(
                                    sal_uInt16 nRecId,
                                    const ScfUInt16Vec& rPageBreaks,
                                    sal_uInt16 nMaxPos );

    /** Writes the record, if the list is not empty. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Writes the page break list. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    const ScfUInt16Vec&         mrPageBreaks;       /// Page settings data of current sheet.
    sal_uInt16                  mnMaxPos;           /// Maximum row/column for BIFF8 page breaks.
};


// Background bitmap ----------------------------------------------------------

class Graphic;

/** Provides export of a background bitmap of a sheet (record BITMAP). */
class XclExpBitmap : public XclExpRecordBase
{
public:
    explicit                    XclExpBitmap( const Graphic& rGraphic );

    /** Writes the BITMAP record. */
    virtual void                Save( XclExpStream& rStrm );

private:
    const Graphic&              mrGraphic;      /// The VCL graphic.
};


// Page settings ==============================================================

/** Contains all page (print) settings records for a single sheet. */
class XclExpPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit                    XclExpPageSettings( const XclExpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData&   GetPageData() const { return maData; }

    /** Writes all page settings records to the stream. */
    virtual void                Save( XclExpStream& rStrm );

private:
    XclPageData                 maData;         /// Page settings data.
};


// ============================================================================

#endif

