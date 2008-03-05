/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartsheetfragment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:01:52 $
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

#ifndef OOX_XLS_CHARTSHEETFRAGMENT_HXX
#define OOX_XLS_CHARTSHEETFRAGMENT_HXX

#include "oox/xls/bifffragmenthandler.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxChartsheetFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxChartsheetFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ISegmentProgressBarRef xProgressBar,
                            sal_Int32 nSheet );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        initializeImport();
    virtual void        finalizeImport();

private:
    /** Imports the the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( RecordInputStream& rStrm );
};

// ============================================================================

class BiffChartsheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffChartsheetFragment(
                            const WorkbookHelper& rHelper,
                            ISegmentProgressBarRef xProgressBar,
                            sal_Int32 nSheet );

    /** Imports the entire sheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment( BiffInputStream& rStrm );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

