/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: externallinkfragment.hxx,v $
 * $Revision: 1.4.20.2 $
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

#ifndef OOX_XLS_EXTERNALLINKFRAGMENT_HXX
#define OOX_XLS_EXTERNALLINKFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/externallinkbuffer.hxx"

namespace oox {
namespace xls {

class ExternalLink;

// ============================================================================

class OoxExternalLinkFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxExternalLinkFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ExternalLink& rExtLink );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    ContextWrapper      createSheetDataContext( sal_Int32 nSheetId );

private:
    ExternalLink&       mrExtLink;
    ExternalNameRef     mxExtName;
    ::rtl::OUString     maResultValue;
    sal_Int32           mnResultType;
};

// ============================================================================

class BiffExternalSheetDataContext;

class BiffExternalLinkFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffExternalLinkFragment( const BiffWorkbookFragmentBase& rParent, bool bImportDefNames );
    virtual             ~BiffExternalLinkFragment();

    /** Imports all records related to external links. */
    virtual bool        importFragment();

    /** Tries to import a record related to external links and defined names. */
    void                importRecord();

    /** Finalizes buffers related to external links and defined names. */
    void                finalizeImport();

private:
    void                importExternSheet();
    void                importExternalBook();
    void                importExternalName();
    void                importXct();
    void                importCrn();
    void                importDefinedName();

private:
    typedef ::boost::shared_ptr< BiffWorksheetContextBase > SheetContextRef;

    SheetContextRef     mxContext;
    ExternalLinkRef     mxExtLink;
    bool                mbImportDefNames;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

