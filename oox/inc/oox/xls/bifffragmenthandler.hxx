/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bifffragmenthandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:59:55 $
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

#ifndef OOX_XLS_BIFFFRAGMENTHANDLER_HXX
#define OOX_XLS_BIFFFRAGMENTHANDLER_HXX

#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** An enumeration for all types of fragments in a BIFF workbook stream. */
enum BiffFragmentType
{
    BIFF_FRAGMENT_GLOBALS,      /// Workbook globals fragment.
    BIFF_FRAGMENT_WORKSHEET,    /// Worksheet fragment.
    BIFF_FRAGMENT_CHARTSHEET,   /// Chart sheet fragment.
    BIFF_FRAGMENT_MACROSHEET,   /// Macro sheet fragment.
    BIFF_FRAGMENT_MODULESHEET,  /// BIFF5 VB module fragment.
    BIFF_FRAGMENT_EMPTYSHEET,   /// Sheet fragment of unsupported type.
    BIFF_FRAGMENT_WORKSPACE,    /// BIFF4 workspace/workbook globals.
    BIFF_FRAGMENT_UNKNOWN       /// Unknown fragment/error.
};

// ============================================================================

class BiffFragmentHandler
{
public:
    inline explicit     BiffFragmentHandler() {}
    virtual             ~BiffFragmentHandler();

    /** Imports the fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment( BiffInputStream& rStrm );

protected:
    /** @return  True = passed record identifier is a BOF record. */
    static bool         isBofRecord( sal_uInt16 nRecId );

    /** Starts a new fragment in a workbbok stream and returns the fragment type.

        The passed stream must point before a BOF record. The function will
        try to start the next record and read the contents of the BOF record,
        if extant.

        @return  Fragment type according to the imported BOF record.
     */
    static BiffFragmentType startFragment( BiffInputStream& rStrm, BiffType eBiff );

    /** Skips the current fragment (processes embedded BOF/EOF blocks correctly).

        Skips all records until next EOF record. When this function returns,
        stream points to the EOF record, and the next call of startNextRecord()
        at the stream will start the record following the EOF record.

        @return  True = stream points to the EOF record of the current fragment.
     */
    static bool         skipFragment( BiffInputStream& rStrm );
};

// ============================================================================

class BiffWorkbookFragmentBase : public BiffFragmentHandler, public WorkbookHelper
{
public:
    explicit            BiffWorkbookFragmentBase( const WorkbookHelper& rHelper );
};

// ============================================================================

class BiffWorksheetFragmentBase : public BiffFragmentHandler, public WorksheetHelperRoot
{
public:
    explicit            BiffWorksheetFragmentBase(
                            const WorkbookHelper& rHelper,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    explicit            BiffWorksheetFragmentBase( const WorksheetHelper& rHelper );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

