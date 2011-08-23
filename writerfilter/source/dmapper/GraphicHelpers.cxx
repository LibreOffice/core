#include "ConversionHelper.hxx"
#include "GraphicHelpers.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

#include <iostream>
using namespace std;

namespace writerfilter {
namespace dmapper {

using namespace com::sun::star;

PositionHandler::PositionHandler( ) :
    Properties( )
{
    m_nOrient = text::VertOrientation::NONE;
    m_nRelation = text::RelOrientation::FRAME;
    m_nPosition = 0;
}

PositionHandler::~PositionHandler( )
{
}

void PositionHandler::attribute( Id aName, Value& rVal )
{
    sal_Int32 nIntValue = rVal.getInt( );
    switch ( aName )
    {
        case NS_ooxml::LN_CT_PosV_relativeFrom:
            {
                // TODO There are some other unhandled values
                static Id pVertRelValues[] =
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_margin,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_paragraph,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_line
                };

                static sal_Int16 pVertRelations[] = 
                {
                    text::RelOrientation::PAGE_PRINT_AREA,
                    text::RelOrientation::PAGE_FRAME,
                    text::RelOrientation::FRAME,
                    text::RelOrientation::TEXT_LINE
                };
                
                for ( int i = 0; i < 4; i++ )
                {
                    if ( pVertRelValues[i] == sal_uInt32( nIntValue ) )
                        m_nRelation = pVertRelations[i];
                }
            }
            break;
        case NS_ooxml::LN_CT_PosH_relativeFrom:
            {
                // TODO There are some other unhandled values
                static Id pHoriRelValues[] =
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_margin,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_column,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_character
                };

                static sal_Int16 pHoriRelations[] = 
                {
                    text::RelOrientation::PAGE_PRINT_AREA,
                    text::RelOrientation::PAGE_FRAME,
                    text::RelOrientation::FRAME,
                    text::RelOrientation::CHAR,
                };
                
                for ( int i = 0; i < 4; i++ )
                {
                    if ( pHoriRelValues[i] == sal_uInt32( nIntValue ) )
                        m_nRelation = pHoriRelations[i];
                }
            }
            break;
        default:;
    }
}

void PositionHandler::sprm( Sprm& rSprm )
{
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    
    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_PosV_align:
            {
                static Id pVertValues[] = 
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignV_top,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignV_bottom,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignV_center,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignV_inside,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignV_outside
                };

                static sal_Int16 pVertOrients[] =
                {
                    text::VertOrientation::TOP,
                    text::VertOrientation::BOTTOM,
                    text::VertOrientation::CENTER,
                    text::VertOrientation::NONE,
                    text::VertOrientation::NONE
                };

                for ( int i = 0; i < 5; i++ )
                {
                    if ( pVertValues[i] == sal_uInt32( nIntValue ) )
                        m_nOrient = pVertOrients[i];
                }
            }
            break;
        case NS_ooxml::LN_CT_PosH_align:
            {
                static Id pHoriValues[] = 
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignH_left,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignH_right,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignH_center,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignH_inside,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_AlignH_outside
                };

                static sal_Int16 pHoriOrients[] =
                {
                    text::HoriOrientation::LEFT,
                    text::HoriOrientation::RIGHT,
                    text::HoriOrientation::CENTER,
                    text::HoriOrientation::INSIDE,
                    text::HoriOrientation::OUTSIDE
                };

                for ( int i = 0; i < 5; i++ )
                {
                    if ( pHoriValues[i] == sal_uInt32( nIntValue ) )
                        m_nOrient = pHoriOrients[i];
                }
            }
            break;
        case NS_ooxml::LN_CT_PosH_posOffset:
        case NS_ooxml::LN_CT_PosV_posOffset:
            m_nPosition = ConversionHelper::convertEMUToMM100( nIntValue );
        default:;
    }
}

WrapHandler::WrapHandler( ) :
    Properties( ),
    m_nType( 0 ),
    m_nSide( 0 )
{
}

WrapHandler::~WrapHandler( )
{
}

void WrapHandler::attribute( Id aName, Value& rVal )
{
    switch ( aName )
    {
        case NS_ooxml::LN_CT_Wrap_type:
            m_nType = sal_Int32( rVal.getInt( ) );
            break;
        case NS_ooxml::LN_CT_Wrap_side:
            m_nSide = sal_Int32( rVal.getInt( ) );
            break; 
        default:;
    }
}

void WrapHandler::sprm( Sprm& )
{
}

sal_Int32 WrapHandler::getWrapMode( )
{
    sal_Int32 nMode = com::sun::star::text::WrapTextMode_NONE;

    switch ( m_nType )
    {
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_square:
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_tight:
            {
                switch ( m_nSide )
                {
                    case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapSide_left:
                        nMode = com::sun::star::text::WrapTextMode_LEFT;
                        break;
                    case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapSide_right:
                        nMode = com::sun::star::text::WrapTextMode_RIGHT;
                        break;
                    default:
                        nMode = com::sun::star::text::WrapTextMode_PARALLEL;
                }
            }
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_through:
            nMode = com::sun::star::text::WrapTextMode_THROUGHT;
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_topAndBottom:
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_none:
        default:
            nMode = com::sun::star::text::WrapTextMode_NONE;
    }

    return nMode;
}

} }
