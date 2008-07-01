/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pivotcachefragment.cxx,v $
 * $Revision: 1.5 $
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
                maPCacheData.maFields.back().maItems.push_back( rAttribs.getString( XML_v, OUString() ) );
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
    switch ( rAttribs.getToken(XML_type, XML_TOKEN_INVALID) )
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

    pSrc->maSrcRange  = rAttribs.getString( XML_ref, OUString() );
    pSrc->maSheetName = rAttribs.getString( XML_sheet, OUString() );
}

void OoxPivotCacheFragment::importCacheField( const AttributeList& rAttribs )
{
    PivotCacheField aField;
    aField.maName = rAttribs.getString( XML_name, OUString() );
    maPCacheData.maFields.push_back(aField);
}

} // namespace xls
} // namespace oox
