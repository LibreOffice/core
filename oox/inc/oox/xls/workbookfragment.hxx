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

#ifndef OOX_XLS_WORKBOOKFRAGMENT_HXX
#define OOX_XLS_WORKBOOKFRAGMENT_HXX

#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

class ExternalLink;

// ============================================================================

class OoxWorkbookFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxWorkbookFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();

private:
    void                importExternalReference( const AttributeList& rAttribs );
    void                importDefinedName( const AttributeList& rAttribs );
    void                importPivotCache( const AttributeList& rAttribs );

    void                importExternalRef( RecordInputStream& rStrm );
    void                importPivotCache( RecordInputStream& rStrm );

    void                importExternalLinkFragment( ExternalLink& rExtLink );
    void                importPivotCacheDefFragment( const ::rtl::OUString& rRelId, sal_Int32 nCacheId );

private:
    DefinedNameRef      mxCurrName;
};

// ============================================================================

class BiffWorkbookFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffWorkbookFragment( const WorkbookHelper& rHelper, const ::rtl::OUString& rStrmName );

    /** Imports the entire workbook stream, including all contained worksheets. */
    virtual bool        importFragment();

private:
    /** Imports a complete BIFF4 workspace fragment (with embedded sheets). */
    bool                importWorkspaceFragment();
    /** Imports the workbook globals fragment from current stream position. */
    bool                importGlobalsFragment( ISegmentProgressBar& rProgressBar );
    /** Imports a sheet fragment with passed type from current stream position. */
    bool                importSheetFragment(
                            ISegmentProgressBar& rProgressBar,
                            BiffFragmentType eFragment, sal_Int16 nCalcSheet );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
