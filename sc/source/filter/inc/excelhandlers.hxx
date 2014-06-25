/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EXCELHANDLERS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EXCELHANDLERS_HXX

#include <oox/core/fragmenthandler2.hxx>
#include "worksheethelper.hxx"

namespace oox {
namespace xls {

/** Context handler derived from the WorkbookHelper helper class.

    Used to import contexts in global workbook fragments.
 */
class WorkbookContextBase : public ::oox::core::ContextHandler2, public WorkbookHelper
{
public:
    template< typename ParentType >
    inline explicit     WorkbookContextBase( ParentType& rParent ) :
                            ::oox::core::ContextHandler2( rParent ), WorkbookHelper( rParent ) {}
};

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class WorksheetContextBase : public ::oox::core::ContextHandler2, public WorksheetHelper
{
public:
    template< typename ParentType >
    inline explicit     WorksheetContextBase( ParentType& rParent ) :
                            ::oox::core::ContextHandler2( rParent ), WorksheetHelper( rParent ) {}
};

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class WorkbookFragmentBase : public ::oox::core::FragmentHandler2, public WorkbookHelper
{
public:
    explicit            WorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const OUString& rFragmentPath );
};

/** Fragment handler derived from the WorksheetHelper helper class.

    Used to import sheet fragments.
 */
class WorksheetFragmentBase : public ::oox::core::FragmentHandler2, public WorksheetHelper
{
public:
    explicit            WorksheetFragmentBase(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );
};

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

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class BiffWorksheetContextBase : public BiffContextHandler, public WorksheetHelper
{
protected:
    explicit            BiffWorksheetContextBase( const WorksheetHelper& rHelper );
};

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

class BiffFragmentHandler
{
public:
    /** Opens the stream with the passed full name. */
    explicit            BiffFragmentHandler(
                            const ::oox::core::FilterBase& rFilter,
                            const OUString& rStrmName );

    virtual             ~BiffFragmentHandler();

    /** Imports the fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment() = 0;

protected:
    /** Returns the BIFF input stream of this fragment. */
    inline BiffInputStream& getInputStream() { return *mxBiffStrm; }

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

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class BiffWorkbookFragmentBase : public BiffFragmentHandler, public WorkbookHelper
{
protected:
    explicit            BiffWorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const OUString& rStrmName,
                            bool bCloneDecoder = false );
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
