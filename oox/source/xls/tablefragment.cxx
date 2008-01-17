/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablefragment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
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

namespace oox {
namespace xls {

// ============================================================================

OoxTableFragment::OoxTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath )
{
}

// oox.xls.OoxContextHelper interface -----------------------------------------

bool OoxTableFragment::onCanCreateContext( sal_Int32 nElement ) const
{
    switch( getCurrentContext() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( table ));
    }
    return false;
}

void OoxTableFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentContext() )
    {
        case XLS_TOKEN( table ):    mxTable = getTables().importTable( rAttribs, getSheetIndex() ); break;
    }
}

bool OoxTableFragment::onCanCreateRecordContext( sal_Int32 nRecId )
{
    switch( getCurrentContext() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_TABLE);
    }
    return false;
}

void OoxTableFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentContext() )
    {
        case OOBIN_ID_TABLE:    mxTable = getTables().importTable( rStrm, getSheetIndex() );    break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

