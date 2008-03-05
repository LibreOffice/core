/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: workbookfragment.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:09:46 $
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

#ifndef OOX_XLS_WORKBOOKFRAGMENT_HXX
#define OOX_XLS_WORKBOOKFRAGMENT_HXX

#include "oox/xls/bifffragmenthandler.hxx"
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

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();

private:
    void                importExternalReference( const AttributeList& rAttribs );
    void                importDefinedName( const AttributeList& rAttribs );
    void                importPivotCache( const AttributeList& rAttribs );

    void                importExternalRef( RecordInputStream& rStrm );

    void                importExternalLinkFragment( ExternalLink& rExtLink );

private:
    DefinedNameRef      mxCurrName;
};

// ============================================================================

class BiffWorkbookFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffWorkbookFragment( const WorkbookHelper& rHelper );

    /** Imports the entire workbook stream, including all contained worksheets. */
    virtual bool        importFragment( BiffInputStream& rStrm );

private:
    /** Imports a complete BIFF4 workspace fragment (with embedded sheets). */
    bool                importWorkspaceFragment( BiffInputStream& rStrm );
    /** Imports the workbook globals fragment from current stream position. */
    bool                importGlobalsFragment(
                            BiffInputStream& rStrm,
                            ISegmentProgressBar& rProgressBar );
    /** Imports a sheet fragment with passed type from current stream position. */
    bool                importSheetFragment(
                            BiffInputStream& rStrm,
                            ISegmentProgressBar& rProgressBar,
                            BiffFragmentType eFragment, sal_Int32 nSheet );

    /** Imports the FILEPASS record and sets a decoder at the passed stream. */
    bool                importFilePass( BiffInputStream& rStrm );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

