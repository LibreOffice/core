/*************************************************************************
 *
 *  $RCSfile: xilink.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:41:56 $
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

#ifndef SC_XILINK_HXX
#define SC_XILINK_HXX

#ifndef SC_XLLINK_HXX
#include "xllink.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

class ScDocument;
class ScTokenArray;
class XclImpStream;


/* ============================================================================
Classes for import of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- Internal and external defined names
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

/** This buffer contains the creation order of all sheets inside the Excel workbook.
    @descr  The creation order list is contained in the TABID record.
    Example: If the list contains 3;1;2 this means that the second sheet in the file
    was created first, than the third sheet in the file was created and finally the
    first sheet. */
class XclImpTabIdBuffer
{
public:
    /** Reads the TABID record. */
    void                        ReadTabid( XclImpStream& rStrm );

    /** Returns the current sheet index calculated from creation index.
        @param nCreatedId  The creation index of the sheet (1-based).
        @param nMaxTabId  All values greater than this parameter are not used to find the index.
        @return  The 0-based index of the sheet nCreatedId if it is contained in the list.
        Example: The buffer is 3;5;2;4;1, nCreatedId is 1 and nMaxTabId is 3. The function will
        return 2 which is the 0-based index of sheet 1 in the list 3;2;1. */
    sal_uInt16                  GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId = 0xFFFF ) const;

private:
    ScfUInt16Vec                maTabIdVec;     /// The vector with sheet indexes.
};


// Internal defined names =====================================================

class ScRangeData;

/** Represents a defined name. It may be related to a single sheet or global. */
class XclImpName : protected XclImpRoot
{
public:
    explicit                    XclImpName( XclImpStream& rStrm, sal_uInt16 nScIndex );

    inline const String&        GetXclName() const { return maXclName; }
    inline const String&        GetScName() const { return maScName; }
    inline sal_uInt16           GetScTabNum() const { return mnScTab; }
    inline const ScRangeData*   GetScRangeData() const { return mpScData; }
    inline bool                 IsGlobal() const { return mnScTab == EXC_NOTAB; }

private:
    String                      maXclName;      /// Original name read from the file.
    String                      maScName;       /// Name inserted into the Calc document.
    const ScRangeData*          mpScData;       /// Pointer to Calc defined name (no ownership).
    sal_Unicode                 mcBuiltIn;      /// Excel built-in name index.
    sal_uInt16                  mnScTab;        /// Sheet index of local names.
};


// ----------------------------------------------------------------------------

/** This buffer contains all internal defined names of the document.
    @descr  It manages the position of the names in the document, means if they are
    global or attached to a specific sheet. While inserting the names into the Calc
    document this buffer resolves conflicts caused by equal names from different
    sheets. */
class XclImpNameBuffer : protected XclImpRoot
{
public:
    explicit                    XclImpNameBuffer( const XclImpRoot& rRoot );

    /** Reads a NAME record and creates an entry in this buffer. */
    void                        ReadName( XclImpStream& rStrm );

    /** Tries to find the name used in Calc, based on the original Excel defined name.
        @param nScTab  The sheet index for local names or EXC_NOTAB for global names.
        If no local name is found, tries to find a matching global name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*           FindName( const String& rXclName, sal_uInt16 nScTab = EXC_NOTAB ) const;

private:
    typedef ScfDelList< XclImpName > XclImpNameList;
    XclImpNameList              maNameList;
};


// External names =============================================================

/** Type of an external name. */
enum XclImpExtNameType
{
    xlExtName,                  /// An external defined name.
    xlExtAddIn,                 /// An add-in function name.
    xlExtDDE,                   /// A DDE link range.
    xlExtOLE                    /// An OLE object link.
};


// ----------------------------------------------------------------------------

/** Stores contents of an external name.
    @descr Supported: External defined names, AddIn names, DDE links and OLE objects. */
class XclImpExtName
{
public:
    /** Reads the external name from the stream. */
    explicit                    XclImpExtName( XclImpStream& rStrm, bool bAddIn = false );

    /** Create and apply the cached list of this DDE Link to the document. */
    void                        CreateDdeData( ScDocument& rDoc,
                                    const String& rApplc,
                                    const String& rExtDoc) const;

    inline XclImpExtNameType    GetType() const         { return meType; }
    inline const String&        GetName() const         { return maName; }
    inline sal_uInt32           GetStorageId() const    { return mnStorageId; }

private:
    typedef ::std::auto_ptr< XclImpCachedMatrix > XclImpCachedMatrixPtr;

    XclImpCachedMatrixPtr       mpDdeMatrix;        /// Cached results of the DDE link.
    String                      maName;             /// The name of the external name.
    sal_uInt32                  mnStorageId;        /// Storage ID for OLE object storages.
    XclImpExtNameType           meType;             /// Type of the external name.
};


// Import link manager ========================================================

class XclImpLinkManager_Impl;

/** This is the central class for the import of all internal/external links.
    @descr  This manager stores all data about external documents with their sheets
    and cached cell contents. Additionally it handles external names, such as add-in
    function names, DDE links, and OLE object links.
    File contents in BIFF8:
    - Record SUPBOOK: Contains the name of an external document and the names of its sheets.
    This record is optionally followed by NAME, EXTERNNAME, XCT and CRN records.
    - Record XCT: Contains the number and sheet index of the following CRN records.
    - Record CRN: Contains addresses (row and column) and values of external referenced cells.
    - Record NAME: Contains defined names of the own workbook.
    - Record EXTERNNAME: Contains external defined names, DDE links, or OLE object links.
    - Record EXTERNSHEET: Contains indexes to URLs of external documents (SUPBOOKs)
    and sheet indexes for each external reference used anywhere in the workbook.
    This record follows a list of SUPBOOK records (with their attached records).
*/
class XclImpLinkManager
{
public:
    explicit                    XclImpLinkManager( const XclImpRoot& rRoot );
                                ~XclImpLinkManager();

    /** Reads the EXTERNSHEET record. */
    void                        ReadExternsheet( XclImpStream& rStrm );
    /** Reads a SUPBOOK record. */
    void                        ReadSupbook( XclImpStream& rStrm );
    /** Reads an XCT record and appends it to the current SUPBOOK. */
    void                        ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record and appends it to the current SUPBOOK. */
    void                        ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record and appends it to the current SUPBOOK. */
    void                        ReadExternname( XclImpStream& rStrm );

    /** Returns true, if the specified XTI entry contains an internal reference. */
    bool                        IsSelfRef( sal_uInt16 nXtiIndex ) const;
    /** Returns the Calc sheet index range of the specified XTI entry.
        @return  true = XTI data found, returned sheet index range is valid. */
    bool                        GetScTabRange(
                                    sal_uInt16& rnFirstScTab, sal_uInt16& rnLastScTab,
                                    sal_uInt16 nXtiIndex ) const;
    /** Returns the specified external name or 0 on error. */
    const XclImpExtName*        GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const;
    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                        GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const;

    /** Returns the Calc sheet index of a table in an external document.
        @return  Calc sheet index or EXC_TAB_INVALID on error. */
    sal_uInt16                  GetScTab( const String& rUrl, const String& rTabName ) const;

private:
    typedef ::std::auto_ptr< XclImpLinkManager_Impl > XclImpLinkManager_ImplPtr;
    XclImpLinkManager_ImplPtr   mpImpl;
};


// ============================================================================

#endif

