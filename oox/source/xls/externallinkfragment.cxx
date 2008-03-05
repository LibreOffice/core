/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externallinkfragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:00:29 $
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

#include "oox/xls/externallinkfragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/sheetdatacontext.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::oox::core::RecordInfo;
using ::oox::core::Relation;

namespace oox {
namespace xls {

// ============================================================================

OoxExternalLinkFragment::OoxExternalLinkFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ExternalLink& rExtLink ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath ),
    mrExtLink( rExtLink ),
    mnResultType( XML_TOKEN_INVALID )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxExternalLinkFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( externalLink ));
        case XLS_TOKEN( externalLink ):
            return  (nElement == XLS_TOKEN( externalBook )) ||
                    (nElement == XLS_TOKEN( ddeLink )) ||
                    (nElement == XLS_TOKEN( oleLink ));
        case XLS_TOKEN( externalBook ):
            return  (nElement == XLS_TOKEN( sheetNames )) ||
                    (nElement == XLS_TOKEN( definedNames )) ||
                    (nElement == XLS_TOKEN( sheetDataSet ));
        case XLS_TOKEN( sheetNames ):
            return  (nElement == XLS_TOKEN( sheetName ));
        case XLS_TOKEN( definedNames ):
            return  (nElement == XLS_TOKEN( definedName ));
        case XLS_TOKEN( sheetDataSet ):
            if( (nElement == XLS_TOKEN( sheetData )) && (mrExtLink.getLinkType() == LINKTYPE_EXTERNAL) )
                return createSheetDataContext( rAttribs.getInteger( XML_sheetId, -1 ) ).get();
        break;
        case XLS_TOKEN( ddeLink ):
            return  (nElement == XLS_TOKEN( ddeItems ));
        case XLS_TOKEN( ddeItems ):
            return  (nElement == XLS_TOKEN( ddeItem ));
        case XLS_TOKEN( ddeItem ):
            return  (nElement == XLS_TOKEN( values ));
        case XLS_TOKEN( values ):
            return  (nElement == XLS_TOKEN( value ));
        case XLS_TOKEN( value ):
            return  (nElement == XLS_TOKEN( val ));
        case XLS_TOKEN( oleLink ):
            return  (nElement == XLS_TOKEN( oleItems ));
        case XLS_TOKEN( oleItems ):
            return  (nElement == XLS_TOKEN( oleItem ));
    }
    return false;
}

void OoxExternalLinkFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( externalBook ): mrExtLink.importExternalBook( getRelations(), rAttribs );   break;
        case XLS_TOKEN( sheetName ):    mrExtLink.importSheetName( rAttribs );                      break;
        case XLS_TOKEN( definedName ):  mrExtLink.importDefinedName( rAttribs );                    break;
        case XLS_TOKEN( ddeLink ):      mrExtLink.importDdeLink( rAttribs );                        break;
        case XLS_TOKEN( ddeItem ):      mxExtName = mrExtLink.importDdeItem( rAttribs );            break;
        case XLS_TOKEN( values ):       if( mxExtName.get() ) mxExtName->importValues( rAttribs );  break;
        case XLS_TOKEN( value ):        mnResultType = rAttribs.getToken( XML_t, XML_n );           break;
        case XLS_TOKEN( oleLink ):      mrExtLink.importOleLink( getRelations(), rAttribs );        break;
        case XLS_TOKEN( oleItem ):      mxExtName = mrExtLink.importOleItem( rAttribs );            break;
    }
}

void OoxExternalLinkFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( val ):
            maResultValue = rChars;
        break;
        case XLS_TOKEN( value ):
            if( mxExtName.get() ) switch( mnResultType )
            {
                case XML_b:
                    mxExtName->appendResultValue( maResultValue.toDouble() );
                break;
                case XML_e:
                    mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( getUnitConverter().calcBiffErrorCode( maResultValue ) ) );
                break;
                case XML_n:
                    mxExtName->appendResultValue( maResultValue.toDouble() );
                break;
                case XML_str:
                    mxExtName->appendResultValue( maResultValue );
                break;
                default:
                    mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) );
            }
        break;
    }
}

ContextWrapper OoxExternalLinkFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_EXTERNALBOOK);
        case OOBIN_ID_EXTERNALBOOK:
            if( (nRecId == OOBIN_ID_EXTSHEETDATA) && (mrExtLink.getLinkType() == LINKTYPE_EXTERNAL) )
                return createSheetDataContext( rStrm.readInt32() ).get();
            return  (nRecId == OOBIN_ID_EXTSHEETNAMES) ||
                    (nRecId == OOBIN_ID_EXTERNALNAME);
        case OOBIN_ID_EXTERNALNAME:
            return  (nRecId == OOBIN_ID_EXTERNALNAMEFLAGS) ||
                    (nRecId == OOBIN_ID_DDEITEMVALUES);
        case OOBIN_ID_DDEITEMVALUES:
            return  (nRecId == OOBIN_ID_DDEITEM_BOOL) ||
                    (nRecId == OOBIN_ID_DDEITEM_DOUBLE) ||
                    (nRecId == OOBIN_ID_DDEITEM_ERROR) ||
                    (nRecId == OOBIN_ID_DDEITEM_STRING);
    }
    return false;
}

void OoxExternalLinkFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_EXTERNALBOOK:         mrExtLink.importExternalBook( getRelations(), rStrm );              break;
        case OOBIN_ID_EXTSHEETNAMES:        mrExtLink.importExtSheetNames( rStrm );                             break;
        case OOBIN_ID_EXTERNALNAME:         mxExtName = mrExtLink.importExternalName( rStrm );                  break;
        case OOBIN_ID_EXTERNALNAMEFLAGS:    if( mxExtName.get() ) mxExtName->importExternalNameFlags( rStrm );  break;
        case OOBIN_ID_DDEITEMVALUES:        if( mxExtName.get() ) mxExtName->importDdeItemValues( rStrm );      break;
        case OOBIN_ID_DDEITEM_BOOL:         if( mxExtName.get() ) mxExtName->importDdeItemBool( rStrm );        break;
        case OOBIN_ID_DDEITEM_DOUBLE:       if( mxExtName.get() ) mxExtName->importDdeItemDouble( rStrm );      break;
        case OOBIN_ID_DDEITEM_ERROR:        if( mxExtName.get() ) mxExtName->importDdeItemError( rStrm );       break;
        case OOBIN_ID_DDEITEM_STRING:       if( mxExtName.get() ) mxExtName->importDdeItemString( rStrm );      break;
    }
}

::rtl::Reference< OoxExternalSheetDataContext > OoxExternalLinkFragment::createSheetDataContext( sal_Int32 nSheetId )
{
    sal_Int32 nSheet = mrExtLink.getSheetIndex( nSheetId );
    ::rtl::Reference< OoxExternalSheetDataContext > xContext(
        new OoxExternalSheetDataContext( *this, SHEETTYPE_WORKSHEET, nSheet ) );
    if( xContext->isValidSheet() )
        return xContext;
    return 0;
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxExternalLinkFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_DDEITEMVALUES,   OOBIN_ID_DDEITEMVALUES + 1  },
        { OOBIN_ID_EXTERNALBOOK,    OOBIN_ID_EXTERNALBOOK + 228 },
        { OOBIN_ID_EXTERNALNAME,    OOBIN_ID_EXTERNALNAME + 10  },
        { OOBIN_ID_EXTROW,          -1                          },
        { OOBIN_ID_EXTSHEETDATA,    OOBIN_ID_EXTSHEETDATA + 1   },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

// ============================================================================

BiffExternalLinkFragment::BiffExternalLinkFragment( const WorkbookHelper& rHelper, bool bImportDefNames ) :
    BiffWorkbookFragmentBase( rHelper ),
    mbImportDefNames( bImportDefNames )
{
}

BiffExternalLinkFragment::~BiffExternalLinkFragment()
{
}

bool BiffExternalLinkFragment::importFragment( BiffInputStream& rStrm )
{
    // process all record in this sheet fragment
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( isBofRecord( rStrm.getRecId() ) )
            skipFragment( rStrm );  // skip unknown embedded fragments
        else
            importRecord( rStrm );
    }
    return rStrm.isValid() && (rStrm.getRecId() == BIFF_ID_EOF);
}

void BiffExternalLinkFragment::importRecord( BiffInputStream& rStrm )
{
    sal_uInt16 nRecId = rStrm.getRecId();
    switch( getBiff() )
    {
        case BIFF2: switch( nRecId )
        {
            case BIFF2_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF2_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
        }
        break;
        case BIFF3: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF3_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF3_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF4: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF3_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF3_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF5: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF5_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF5_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF8: switch( nRecId )
        {
            case BIFF_ID_CRN:           importCrn( rStrm );             break;
            case BIFF_ID_EXTERNALBOOK:  importExternalBook( rStrm );    break;
            case BIFF5_ID_EXTERNALNAME: importExternalName( rStrm );    break;
            case BIFF_ID_EXTERNSHEET:   importExternSheet( rStrm );     break;
            case BIFF5_ID_DEFINEDNAME:  importDefinedName( rStrm );     break;
            case BIFF_ID_XCT:           importXct( rStrm );             break;
        }
        break;
        case BIFF_UNKNOWN: break;
    }
}

void BiffExternalLinkFragment::finalizeImport()
{
    getDefinedNames().finalizeImport();
}

// private --------------------------------------------------------------------

void BiffExternalLinkFragment::importExternSheet( BiffInputStream& rStrm )
{
    mxContext.reset();
    if( getBiff() == BIFF8 )
        getExternalLinks().importExternSheet8( rStrm );
    else
        mxExtLink = getExternalLinks().importExternSheet( rStrm );
}

void BiffExternalLinkFragment::importExternalBook( BiffInputStream& rStrm )
{
    mxContext.reset();
    mxExtLink = getExternalLinks().importExternalBook( rStrm );
}

void BiffExternalLinkFragment::importExternalName( BiffInputStream& rStrm )
{
    if( mxExtLink.get() )
        mxExtLink->importExternalName( rStrm );
}

void BiffExternalLinkFragment::importXct( BiffInputStream& rStrm )
{
    mxContext.reset();
    if( mxExtLink.get() && (mxExtLink->getLinkType() == LINKTYPE_EXTERNAL) )
    {
        sal_Int32 nSheet = -1;
        switch( getBiff() )
        {
            case BIFF2:
            break;
            case BIFF3:
            case BIFF4:
            case BIFF5:
                nSheet = mxExtLink->getSheetIndex();
            break;
            case BIFF8:
                rStrm.skip( 2 );
                nSheet = mxExtLink->getSheetIndex( rStrm.readInt16() );
            break;
            case BIFF_UNKNOWN: break;
        }

        // create a sheet data context to import the CRN records and set the cached cell values
        mxContext.reset( new BiffExternalSheetDataContext( *this, SHEETTYPE_WORKSHEET, nSheet ) );
        if( !mxContext->isValidSheet() )
            mxContext.reset();
    }
}

void BiffExternalLinkFragment::importCrn( BiffInputStream& rStrm )
{
    if( mxContext.get() )
        mxContext->importCrn( rStrm );
}

void BiffExternalLinkFragment::importDefinedName( BiffInputStream& rStrm )
{
    if( mbImportDefNames )
        getDefinedNames().importDefinedName( rStrm );
}

// ============================================================================

} // namespace xls
} // namespace oox

