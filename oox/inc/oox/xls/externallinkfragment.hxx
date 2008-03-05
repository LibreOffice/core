/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externallinkfragment.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:04:06 $
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

#ifndef OOX_XLS_EXTERNALLINKFRAGMENT_HXX
#define OOX_XLS_EXTERNALLINKFRAGMENT_HXX

#include "oox/xls/bifffragmenthandler.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/externallinkbuffer.hxx"

namespace oox {
namespace xls {

class ExternalLink;
class OoxExternalSheetDataContext;

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
    ::rtl::Reference< OoxExternalSheetDataContext >
                        createSheetDataContext( sal_Int32 nSheetId );

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
    explicit            BiffExternalLinkFragment( const WorkbookHelper& rHelper, bool bImportDefNames );
    virtual             ~BiffExternalLinkFragment();

    /** Imports all records related to external links. */
    virtual bool        importFragment( BiffInputStream& rStrm );

    /** Tries to import a record related to external links and defined names. */
    void                importRecord( BiffInputStream& rStrm );

    /** Finalizes buffers related to external links and defined names. */
    void                finalizeImport();

private:
    void                importExternSheet( BiffInputStream& rStrm );
    void                importExternalBook( BiffInputStream& rStrm );
    void                importExternalName( BiffInputStream& rStrm );
    void                importXct( BiffInputStream& rStrm );
    void                importCrn( BiffInputStream& rStrm );
    void                importDefinedName( BiffInputStream& rStrm );

private:
    typedef ::boost::shared_ptr< BiffExternalSheetDataContext > SheetContextRef;

    SheetContextRef     mxContext;
    ExternalLinkRef     mxExtLink;
    bool                mbImportDefNames;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

