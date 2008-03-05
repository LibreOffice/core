/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pivotcachefragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:04:00 $
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

#include "oox/xls/pivotcachefragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/xls/addressconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::sheet::XSpreadsheet;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::xml::sax::SAXException;

namespace oox {
namespace xls {

OoxPivotCacheFragment::OoxPivotCacheFragment( const WorkbookHelper& rHelper,
                                              const OUString& rFragmentPath,
                                              sal_uInt32 nCacheId ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath ),
    mnCacheId( nCacheId ),
    mbValidSource( false )
{
}

ContextWrapper OoxPivotCacheFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( pivotCacheDefinition ));
        case XLS_TOKEN( pivotCacheDefinition ):
            return  (nElement == XLS_TOKEN( cacheSource )) ||
                    (nElement == XLS_TOKEN( cacheFields ));
        case XLS_TOKEN( cacheSource ):
            return  (nElement == XLS_TOKEN( worksheetSource ));
        case XLS_TOKEN( cacheFields ):
            return  (nElement == XLS_TOKEN( cacheField ));
        case XLS_TOKEN( cacheField ):
            return  (nElement == XLS_TOKEN( sharedItems ));
        case XLS_TOKEN( sharedItems ):
            return  (nElement == XLS_TOKEN( s ));
    }
    return false;
}

void OoxPivotCacheFragment::onStartElement( const AttributeList& rAttribs )
{
    switch ( getCurrentElement() )
    {
        case XLS_TOKEN( pivotCacheDefinition ):
            importPivotCacheDefinition( rAttribs );
        break;
        case XLS_TOKEN( cacheSource ):
            importCacheSource( rAttribs );
        break;
        case XLS_TOKEN( worksheetSource ):
            if ( mbValidSource )
                importWorksheetSource( rAttribs );
        break;
        case XLS_TOKEN( cacheFields ):
            if ( mbValidSource )
                maPCacheData.maFields.reserve( rAttribs.getUnsignedInteger(XML_count, 1) );
        break;
        case XLS_TOKEN( cacheField ):
            if ( mbValidSource )
                importCacheField( rAttribs );
        break;
        case XLS_TOKEN( sharedItems ):
            if ( mbValidSource )
                maPCacheData.maFields.back().maItems.reserve( rAttribs.getUnsignedInteger(XML_count, 1) );
        break;
        case XLS_TOKEN( s ):
            if ( mbValidSource )
                maPCacheData.maFields.back().maItems.push_back( rAttribs.getString( XML_v ) );
        break;
    }
}

void OoxPivotCacheFragment::finalizeImport()
{
    if( mbValidSource )
        getPivotTables().setPivotCache( mnCacheId, maPCacheData );
}

void OoxPivotCacheFragment::importPivotCacheDefinition( const AttributeList& /*rAttribs*/ )
{
}

void OoxPivotCacheFragment::importCacheSource( const AttributeList& rAttribs )
{
    switch ( rAttribs.getToken(XML_type) )
    {
        case XML_worksheet:
            maPCacheData.meSourceType = PivotCacheData::WORKSHEET;
            maPCacheData.mpSourceProp.reset( new PivotCacheData::WorksheetSource );
            mbValidSource = true;
        break;
        case XML_external:
            maPCacheData.meSourceType = PivotCacheData::EXTERNAL;
            maPCacheData.mpSourceProp.reset( new PivotCacheData::ExternalSource );
            mbValidSource = true;
        break;
        default:
            // unsupported case source type.
        break;
    }
}

void OoxPivotCacheFragment::importWorksheetSource( const AttributeList& rAttribs )
{
    if ( maPCacheData.meSourceType != PivotCacheData::WORKSHEET )
        return;

    PivotCacheData::WorksheetSource* pSrc = static_cast<PivotCacheData::WorksheetSource*>(
        maPCacheData.mpSourceProp.get() );

    pSrc->maSrcRange  = rAttribs.getString( XML_ref );
    pSrc->maSheetName = rAttribs.getString( XML_sheet );
}

void OoxPivotCacheFragment::importCacheField( const AttributeList& rAttribs )
{
    PivotCacheField aField;
    aField.maName = rAttribs.getString( XML_name );
    maPCacheData.maFields.push_back(aField);
}

} // namespace xls
} // namespace oox
