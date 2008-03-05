/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablefragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:07:04 $
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

#include "oox/xls/tablefragment.hxx"

using ::rtl::OUString;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxTableFragment::OoxTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( table ));
    }
    return false;
}

void OoxTableFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( table ):    mxTable = getTables().importTable( rAttribs, getSheetIndex() ); break;
    }
}

ContextWrapper OoxTableFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_TABLE);
    }
    return false;
}

void OoxTableFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_TABLE:    mxTable = getTables().importTable( rStrm, getSheetIndex() );    break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_TABLE,   OOBIN_ID_TABLE + 1  },
        { -1,               -1                  }
    };
    return spRecInfos;
}

// ============================================================================

} // namespace xls
} // namespace oox

