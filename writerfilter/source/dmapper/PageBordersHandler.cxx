#include "PageBordersHandler.hxx"

#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

_PgBorder::_PgBorder( ) :
    m_nDistance( 0 ),
    m_ePos( BORDER_RIGHT )
{
}

_PgBorder::~_PgBorder( )
{
}

PageBordersHandler::PageBordersHandler( ) :
    m_nDisplay( 0 ),
    m_nOffset( 0 )
{
}

PageBordersHandler::~PageBordersHandler( )
{
}

void PageBordersHandler::attribute( Id eName, Value& rVal )
{
    int nIntValue = rVal.getInt( );
    switch ( eName )
    {
        case NS_ooxml::LN_CT_PageBorders_display:
        {
            switch ( nIntValue )
            {
                default:
                case NS_ooxml::LN_Value_wordprocessingml_ST_PageBorderDisplay_allPages:
                    m_nDisplay = 0;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_PageBorderDisplay_firstPage:
                    m_nDisplay = 1;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_PageBorderDisplay_notFirstPage:
                    m_nDisplay = 2;
                    break;
            }
        }
        break;
        case NS_ooxml::LN_CT_PageBorders_offsetFrom:
        {
            switch ( nIntValue )
            {
                default:
                case NS_ooxml::LN_Value_wordprocessingml_ST_PageBorderOffset_page:
                    m_nOffset = 1;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_PageBorderOffset_text:
                    m_nOffset = 0;
                    break;
            }
        }
        break;
        default:;
    }
}

void PageBordersHandler::sprm( Sprm& rSprm )
{
    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_PageBorders_top:
        case NS_ooxml::LN_CT_PageBorders_left:
        case NS_ooxml::LN_CT_PageBorders_bottom:
        case NS_ooxml::LN_CT_PageBorders_right:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler( true ) );
                pProperties->resolve(*pBorderHandler);
                BorderPosition ePos = BorderPosition( 0 );
                switch( rSprm.getId( ) )
                {
                    case NS_ooxml::LN_CT_PageBorders_top:
                        ePos = BORDER_TOP;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_left:
                        ePos = BORDER_LEFT;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_bottom:
                        ePos = BORDER_BOTTOM;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_right:
                        ePos = BORDER_RIGHT;
                    break;
                    default:;
                }
                
                _PgBorder aPgBorder;
                aPgBorder.m_rLine = pBorderHandler->getBorderLine( );
                aPgBorder.m_nDistance = pBorderHandler->getLineDistance( );
                aPgBorder.m_ePos = ePos;
                m_aBorders.push_back( aPgBorder );
            }
        }
        break;
        default:;
    }
}

void PageBordersHandler::SetBorders( SectionPropertyMap* pSectContext )
{
    for ( int i = 0, length = m_aBorders.size( ); i < length; i++ )
    {
        _PgBorder aBorder = m_aBorders[i];
        pSectContext->SetBorder( aBorder.m_ePos, aBorder.m_nDistance, aBorder.m_rLine );
    }
}

} }
