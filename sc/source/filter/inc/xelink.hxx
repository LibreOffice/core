/*************************************************************************
 *
 *  $RCSfile: xelink.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:40:19 $
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

#ifndef SC_XELINK_HXX
#define SC_XELINK_HXX

#ifndef SC_MARKDATA_HXX
#include "markdata.hxx"
#endif

#ifndef SC_XLLINK_HXX
#include "xllink.hxx"
#endif
#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

class ScRange;
struct SingleRefData;


/* ============================================================================
Classes for export of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- Internal and external defined names
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

typedef ::std::vector< ::std::pair< sal_uInt16, sal_uInt16 > > XclExpRefLogVec;

/** Stores the correct Excel sheet index for each Calc sheet.
    @descr  The class knows all sheets which will not exported
    (i.e. external link sheets, scenario sheets). */
class XclExpTabIdBuffer
{
public:
    /** Initializes the complete buffer from the passed document. */
    explicit                    XclExpTabIdBuffer( ScDocument& rDoc );

    /** Returns true, if the specified Calc sheet is used to store external cell contents. */
    bool                        IsExternal( sal_uInt16 nScTab ) const;
    /** Returns true, if the specified Calc sheet has to be exported. */
    bool                        IsExportTable( sal_uInt16 nScTab ) const;

    /** Returns the Excel sheet index for a given Calc sheet. */
    sal_uInt16                  GetXclTab( sal_uInt16 nScTab ) const;

    /** Returns the Calc sheet index of the nSortedTab-th entry in the sorted sheet names list. */
    sal_uInt16                  GetRealScTab( sal_uInt16 nSortedTab ) const;
    /** Returns the index of the passed Calc sheet in the sorted sheet names list. */
    sal_uInt16                  GetSortedScTab( sal_uInt16 nScTab ) const;

    /** Returns the number of Calc sheets. */
    inline sal_uInt16           GetScTabCount() const { return mnScCnt; }
    /** Returns the number of Excel sheets to be exported. */
    inline sal_uInt16           GetXclTabCount() const { return mnXclCnt; }
    /** Returns the number of external linked sheets (in Calc). */
    inline sal_uInt16           GetExternTabCount() const { return mnExtCnt; }
    /** Returns the number of codepages (VBA modules). */
    inline sal_uInt16           GetCodenameCount() const { return mnCodeCnt; }
    /** Returns the maximum number of Calc sheets and codepages. */
    inline sal_uInt16           GetMaxScTabCount() const { return ::std::max( mnScCnt, mnCodeCnt ); }

    // *** for change tracking ***

    /** Enables logging of Excel sheet indexes in each 3D-reference. */
    void                        StartRefLog();
    /** Appends sheet index pair (called by formula compiler). */
    void                        AppendTabRef( sal_uInt16 nXclFirst, sal_uInt16 nXclLast );
    /** Disables logging of Excel sheet indexes. */
    const XclExpRefLogVec&      EndRefLog();

private:
    /** Searches for sheets not to be exported. */
    void                        CalcXclIndexes();
    /** Sorts the names of all tables and stores the indexes of the sorted indexes. */
    void                        CalcSortedIndexes( ScDocument& rDoc );

private:
    typedef ::std::vector< ::std::pair< sal_uInt16, sal_uInt8 > > IndexEntryVec;

    IndexEntryVec               maIndexVec;         /// Array of sheet index information.

    sal_uInt16                  mnScCnt;            /// Count of Calc sheets.
    sal_uInt16                  mnXclCnt;           /// Count of Excel sheets to be exported.
    sal_uInt16                  mnExtCnt;           /// Count of external link sheets (in Calc).
    sal_uInt16                  mnCodeCnt;          /// Count of codepages.

    ScfUInt16Vec                maFromSortedVec;    /// Sorted index -> real index.
    ScfUInt16Vec                maToSortedVec;      /// Real index -> sorted index.

    XclExpRefLogVec             maRefLog;           /// A log for each requested Excel sheet index.
    bool                        mbEnableLog;        /// true = log all sheet indexes (for formula compiler).
};


// Export link manager ========================================================

class XclExpLinkManager_Impl;

/** Stores all EXTERNSHEET and SUPBOOK record data.
    @descr  This is the central class for export of all external references.
    File contents in BIFF8:
    - Record SUPBOOK: Contains the name of an external workbook and the names of its sheets.
    This record is followed by EXTERNNAME, XCT and CRN records.
    - Record XCT: Contains the sheet index of the following CRN records.
    - Record CRN: Contains addresses (row and column) and values of external referenced cells.
    - Record NAME: Contains defined names of the own workbook. This record follows the
    EXTERNSHEET record.
    - Record EXTERNNAME: Contains external defined names or DDE links or OLE object links.
    - Record EXTERNSHEET: Contains indexes to URLs of external documents (SUPBOOKs)
    and sheet indexes for each external reference used anywhere in the workbook.
    This record follows a list of SUPBOOK records.
*/
class XclExpLinkManager : public XclExpRecordBase
{
public:
    explicit                    XclExpLinkManager( const XclExpRoot& rRoot );
    virtual                     ~XclExpLinkManager();

    /** Searches for XTI structure with the given Excel sheet range. Adds new XTI if not found.
        @return  The list index of the XTI structure. */
    sal_uInt16                  FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast );

    /** Returns the external document URL of the specified Excel sheet. */
    const XclExpString*         GetUrl( sal_uInt16 nXclTab ) const;
    /** Returns the external sheet name of the specified Excel sheet. */
    const XclExpString*         GetTableName( sal_uInt16 nXclTab ) const;

    /** Stores the cell with the given address in a CRN record list. */
    void                        StoreCellCont( const SingleRefData& rRef );
    /** Stores all cells in the given range in a CRN record list. */
    void                        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 );

    /** Finds or inserts an EXTERNNAME record for an add-in function name.
        @param rnXti  Returns the index of the XTI structure which contains the add-in function name.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    void                        InsertAddIn(
                                    sal_uInt16& rnXti, sal_uInt16& rnExtName,
                                    const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @param rnXti  Returns the index of the XTI structure which contains the DDE link.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    bool                        InsertDde(
                                    sal_uInt16& rnXti, sal_uInt16& rnExtName,
                                    const String& rApplic, const String& rTopic, const String& rItem );

    /** Writes the entire Link table. */
    virtual void                Save( XclExpStream& rStrm );

private:
    typedef ::std::auto_ptr< XclExpLinkManager_Impl > XclExpLinkManager_ImplPtr;
    XclExpLinkManager_ImplPtr   mpImpl;
};


// ============================================================================

#endif

