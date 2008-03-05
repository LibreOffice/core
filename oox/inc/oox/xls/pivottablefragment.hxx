/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pivottablefragment.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:06:23 $
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

#ifndef OOX_XLS_PIVOTTABLEFRAGMENT_HXX
#define OOX_XLS_PIVOTTABLEFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/pivottablebuffer.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxPivotTableFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxPivotTableFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual void        finalizeImport();

private:
    void                importLocation( const AttributeList& rAttribs );
    void                importPivotTableDefinition( const AttributeList& rAttribs );
    void                importPivotFields( const AttributeList& rAttribs );
    void                importPivotField( const AttributeList& rAttribs );

private:
    ::rtl::OUString maName;
    PivotTableData  maData;

    bool        mbValidRange;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

