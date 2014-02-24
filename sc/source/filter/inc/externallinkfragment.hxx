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

#ifndef OOX_XLS_EXTERNALLINKFRAGMENT_HXX
#define OOX_XLS_EXTERNALLINKFRAGMENT_HXX

#include "excelhandlers.hxx"
#include "externallinkbuffer.hxx"

namespace oox {
namespace xls {

class ExternalLink;




/** This class implements importing the sheetData element in external sheets.

    The sheetData element embedded in the externalBook element contains cached
    cells from externally linked sheets.
 */
class ExternalSheetDataContext : public WorkbookContextBase
{
public:
    explicit            ExternalSheetDataContext(
                            WorkbookFragmentBase& rFragment,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >& rxSheetCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

private:
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );

    /** Imports the EXTCELL_BLANK from the passed stream. */
    void                importExtCellBlank( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_BOOL from the passed stream. */
    void                importExtCellBool( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_DOUBLE from the passed stream. */
    void                importExtCellDouble( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_ERROR from the passed stream. */
    void                importExtCellError( SequenceInputStream& rStrm );
    /** Imports the EXTCELL_STRING from the passed stream. */
    void                importExtCellString( SequenceInputStream& rStrm );

    /** Sets the passed cell value to the current position in the sheet cache. */
    void                setCellValue( const ::com::sun::star::uno::Any& rValue );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XExternalSheetCache >
                        mxSheetCache;               /// The sheet cache used to store external cell values.
    ::com::sun::star::table::CellAddress maCurrPos; /// Position of current cell.
    sal_Int32           mnCurrType;                 /// Data type of current cell.
};



class ExternalLinkFragment : public WorkbookFragmentBase
{
public:
    explicit            ExternalLinkFragment(
                            const WorkbookHelper& rHelper,
                            const OUString& rFragmentPath,
                            ExternalLink& rExtLink );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const OUString& rChars );
    virtual void        onEndElement();

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    ::oox::core::ContextHandlerRef createSheetDataContext( sal_Int32 nSheetId );

private:
    ExternalLink&       mrExtLink;
    ExternalNameRef     mxExtName;
    OUString     maResultValue;
    sal_Int32           mnResultType;
};



} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
