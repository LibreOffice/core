#include "hyperlinkbuffer.hxx"
#include "worksheetbuffer.hxx"
#include "document.hxx"
#include "convuno.hxx"
#include "cell.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <oox/core/filterbase.hxx>
#include <oox/token/properties.hxx>

#include <com/sun/star/text/XText.hpp>

namespace oox {
namespace xls {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::container;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

HyperlinkBuffer::HyperlinkBuffer( const WorkbookHelper& rHelper ) : WorkbookHelper( rHelper )
{
}

OUString HyperlinkBuffer::getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const
{
    OUStringBuffer aUrlBuffer;
    if( !rHyperlink.maTarget.isEmpty() )
        aUrlBuffer.append( getBaseFilter().getAbsoluteUrl( rHyperlink.maTarget ) );
    if( !rHyperlink.maLocation.isEmpty() )
        aUrlBuffer.append( sal_Unicode( '#' ) ).append( rHyperlink.maLocation );
    OUString aUrl = aUrlBuffer.makeStringAndClear();
    // if necessary convert '#SheetName!A1' to '#SheetName.A1'
    getWorksheets().convertSheetNameRef( aUrl );
    return aUrl;
}

void HyperlinkBuffer::finalizeImport()
{
    Reference< XIndexAccess > xSheets( getDocument()->getSheets(), UNO_QUERY_THROW );
    for ( sal_Int16 nTab = 0, nElem = xSheets->getCount(); nTab < nElem; ++nTab )
    {

        Reference< XSpreadsheet > xCurrSheet( getSheetFromDoc( nTab ) );
        HyperlinkModelList& rHyperlinks = mHyperlinkModelLists[ nTab ];
        for( HyperlinkModelList::const_iterator aIt = rHyperlinks.begin(), aEnd = rHyperlinks.end(); aIt != aEnd; ++aIt )
        {
            OUString aUrl = getHyperlinkUrl( *aIt );
            // try to insert URL into each cell of the range
            if( !aUrl.isEmpty() )
            {
                for( CellAddress aAddress( nTab, aIt->maRange.StartColumn, aIt->maRange.StartRow ); aAddress.Row <= aIt->maRange.EndRow; ++aAddress.Row )
                {
                    for( aAddress.Column = aIt->maRange.StartColumn; aAddress.Column <= aIt->maRange.EndColumn; ++aAddress.Column )
                    {
                        insertHyperlink( xCurrSheet, aAddress, aUrl );
                    }
                }
            }
        }
    }
}

void HyperlinkBuffer::insertHyperlink( const Reference< XSpreadsheet > rCurrSheet, const CellAddress& rAddress, const OUString& rUrl ) const
{
    Reference< XCell > xCell;
    try
    {
        xCell = rCurrSheet->getCellByPosition( rAddress.Column, rAddress.Row );
    }
    catch( Exception& )
    {
    }
    if( xCell.is() )
    {
        switch ( xCell->getType() )
        {
            case CellContentType_FORMULA:
            {
                ScDocument& rDoc = getScDocument();
                ScAddress aCellPos;
                ScUnoConversion::FillScAddress( aCellPos, rAddress );
                ScFormulaCell* pFormCell = static_cast< ScFormulaCell* >( rDoc.GetCell( aCellPos ) );
                pFormCell->SetHyperlinkURL( rUrl );
                break;
            }
            // #i54261# restrict creation of URL field to text cells
            case CellContentType_TEXT:
            {
                Reference< XText > xText( xCell, UNO_QUERY );
                if( xText.is() )
                {
                    // create a URL field object and set its properties
                    Reference< XTextContent > xUrlField( getBaseFilter().getModelFactory()->createInstance( CREATE_OUSTRING( "com.sun.star.text.TextField.URL" ) ), UNO_QUERY );
                    OSL_ENSURE( xUrlField.is(), "HyperlinkBuffer::insertHyperlink - cannot create text field" );
                    if( xUrlField.is() )
                    {
                        // properties of the URL field
                        PropertySet aPropSet( xUrlField );
                        aPropSet.setProperty( PROP_URL, rUrl );
                        aPropSet.setProperty( PROP_Representation, xText->getString() );
                        try
                        {
                            // insert the field into the cell
                            xText->setString( OUString() );
                            Reference< XTextRange > xRange( xText->createTextCursor(), UNO_QUERY_THROW );
                            xText->insertTextContent( xRange, xUrlField, sal_False );
                        }
                        catch( const Exception& )
                        {
                            OSL_FAIL( "HyperlinkBuffer::insertHyperlink - cannot insert text field" );
                        }
                    }
                }
                break;
            }
            default:;
        }
    }
}

void HyperlinkBuffer::createHyperlinkMapEntry( sal_Int32 nTab, const HyperlinkModel& rModel )
{
    mHyperlinkModelLists[ nTab ].push_back( rModel );
}

}
}
