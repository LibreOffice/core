/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_XLS_EXCELHANDLERS_HXX
#define OOX_XLS_EXCELHANDLERS_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================
// ============================================================================

/** Context handler derived from the WorkbookHelper helper class.

    Used to import contexts in global workbook fragments.
 */
class WorkbookContextBase : public ::oox::core::ContextHandler2, public WorkbookHelper
{
public:
    template< typename ParentType >
    explicit            WorkbookContextBase( ParentType& rParent );
};

// ----------------------------------------------------------------------------

template< typename ParentType >
WorkbookContextBase::WorkbookContextBase( ParentType& rParent ) :
    ::oox::core::ContextHandler2( rParent ),
    WorkbookHelper( rParent )
{
}

// ============================================================================

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class WorksheetContextBase : public ::oox::core::ContextHandler2, public WorksheetHelperRoot
{
public:
    template< typename ParentType >
    explicit            WorksheetContextBase(
                            ParentType& rParent,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

    template< typename ParentType >
    explicit            WorksheetContextBase( ParentType& rParent );
};

// ----------------------------------------------------------------------------

template< typename ParentType >
WorksheetContextBase::WorksheetContextBase( ParentType& rParent,
        const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    ::oox::core::ContextHandler2( rParent ),
    WorksheetHelperRoot( rParent, rxProgressBar, eSheetType, nSheet )
{
}

template< typename ParentType >
WorksheetContextBase::WorksheetContextBase( ParentType& rParent ) :
    ::oox::core::ContextHandler2( rParent ),
    WorksheetHelperRoot( rParent )
{
}

// ============================================================================

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class WorkbookFragmentBase : public ::oox::core::FragmentHandler2, public WorkbookHelper
{
public:
    explicit            WorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );
};

// ============================================================================

/** Fragment handler derived from the WorksheetHelper helper class.

    Used to import sheet fragments.
 */
class WorksheetFragmentBase : public ::oox::core::FragmentHandler2, public WorksheetHelperRoot
{
public:
    explicit            WorksheetFragmentBase(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

    explicit            WorksheetFragmentBase(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );
};

// ============================================================================
// ============================================================================

/** Base class for all BIFF context handlers.

    Derived handlers have to implement the importRecord() function that has to
    import the record the passed BIFF input stream currently points to.
 */
class BiffContextHandler
{
public:
    virtual             ~BiffContextHandler();

    /** Derived classes have to implement importing the current record. */
    virtual void        importRecord( BiffInputStream& rStrm ) = 0;
};

// ----------------------------------------------------------------------------

/** Context handler derived from the WorkbookHelper helper class.

    Used to import contexts in global workbook fragments.
 */
class BiffWorkbookContextBase : public BiffContextHandler, public WorkbookHelper
{
protected:
    explicit            BiffWorkbookContextBase( const WorkbookHelper& rHelper );
};

// ----------------------------------------------------------------------------

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class BiffWorksheetContextBase : public BiffContextHandler, public WorksheetHelperRoot
{
protected:
    explicit            BiffWorksheetContextBase(
                            const WorkbookHelper& rHelper,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

    explicit            BiffWorksheetContextBase( const WorksheetHelper& rHelper );
};

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

// ----------------------------------------------------------------------------

class BiffFragmentHandler
{
public:
    /** Opens the stream with the passed full name. */
    explicit            BiffFragmentHandler(
                            const ::oox::core::FilterBase& rFilter,
                            const ::rtl::OUString& rStrmName );

    virtual             ~BiffFragmentHandler();

    /** Imports the fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment() = 0;

protected:
    /** Returns the BIFF input stream of this fragment. */
    inline BiffInputStream& getInputStream() { return *mxBiffStrm; }

    /** Starts a new fragment in a workbbok stream and returns the fragment type.

        The passed stream must point before a BOF record. The function will
        try to start the next record and read the contents of the BOF record,
        if extant.

        @return  Fragment type according to the imported BOF record.
     */
    BiffFragmentType    startFragment( BiffType eBiff );

    /** Starts a new fragment at a specific position in the workbbok stream and
        returns the fragment type.

        The passed record handle must specify the stream position of the BOF
        record of the fragment substream. The function will try to start the
        next record and read the contents of the BOF record, if extant.

        @return  Fragment type according to the imported BOF record.
     */
    BiffFragmentType    startFragment( BiffType eBiff, sal_Int64 nRecHandle );

    /** Skips the current fragment up to its trailing EOF record.

        Skips all records until next EOF record. When this function returns,
        stream points to the EOF record, and the next call of startNextRecord()
        at the stream will start the record following the EOF record.

        Embedded fragments enclosed in BOF/EOF records (e.g. embedded chart
        objects) are skipped correctly.

        @return  True = stream points to the EOF record of the current fragment.
     */
    bool                skipFragment();

private:
    typedef ::boost::shared_ptr< BinaryXInputStream >   XInputStreamRef;
    typedef ::boost::shared_ptr< BiffInputStream >      BiffInputStreamRef;

    XInputStreamRef     mxXInStrm;
    BiffInputStreamRef  mxBiffStrm;
};

// ----------------------------------------------------------------------------

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class BiffWorkbookFragmentBase : public BiffFragmentHandler, public WorkbookHelper
{
protected:
    explicit            BiffWorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rStrmName,
                            bool bCloneDecoder = false );
};

// ----------------------------------------------------------------------------

/** Fragment handler derived from the WorksheetHelper helper class.

    Used to import sheet fragments.
 */
class BiffWorksheetFragmentBase : public BiffFragmentHandler, public WorksheetHelperRoot
{
protected:
    explicit            BiffWorksheetFragmentBase(
                            const BiffWorkbookFragmentBase& rParent,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );
};

// ----------------------------------------------------------------------------

/** Special fragment handler for worksheets that have to be skipped.
 */
class BiffSkipWorksheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffSkipWorksheetFragment(
                            const BiffWorkbookFragmentBase& rParent,
                            const ISegmentProgressBarRef& rxProgressBar,
                            sal_Int16 nSheet );

    virtual bool        importFragment();
};

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */