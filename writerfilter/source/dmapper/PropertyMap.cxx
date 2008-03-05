/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:53:05 $
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
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#include <ooxml/resourceids.hxx>
#ifndef INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX
#include <DomainMapper_Impl.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif

using namespace ::com::sun::star;

namespace writerfilter {
namespace dmapper{

/*-- 21.06.2006 09:30:56---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMap::PropertyMap() :
    m_cFootnoteSymbol( 0 ),
    m_nFootnoteFontId( -1 )
{
}
/*-- 21.06.2006 09:30:56---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMap::~PropertyMap()
{
}
/*-- 20.06.2006 10:23:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > PropertyMap::GetPropertyValues()
{
    if(!m_aValues.getLength() && size())
    {
        m_aValues.realloc( size() );
        ::com::sun::star::beans::PropertyValue* pValues = m_aValues.getArray();
        //style names have to be the first elements within the property sequence
        //otherwise they will overwrite 'hard' attributes
        sal_Int32 nValue = 0;
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        PropertyMap::iterator aParaStyleIter = find(PropertyDefinition( PROP_PARA_STYLE_NAME, false ) );
        if( aParaStyleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aParaStyleIter->first.eId );
            pValues[nValue].Value = aParaStyleIter->second;
            ++nValue;
        }

        PropertyMap::iterator aCharStyleIter = find(PropertyDefinition( PROP_CHAR_STYLE_NAME, false ));
        if( aCharStyleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aCharStyleIter->first.eId );
            pValues[nValue].Value = aCharStyleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aNumRuleIter = find(PropertyDefinition( PROP_NUMBERING_RULES, false ) );
        if( aNumRuleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aNumRuleIter->first.eId );
            pValues[nValue].Value = aNumRuleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aMapIter = begin();
        for( ; nValue < m_aValues.getLength(); ++aMapIter )
        {
            if( aMapIter != aParaStyleIter && aMapIter != aCharStyleIter && aMapIter != aNumRuleIter )
            {
                pValues[nValue].Name = rPropNameSupplier.GetName( aMapIter->first.eId );
                pValues[nValue].Value = aMapIter->second;
                ++nValue;
            }
        }
    }
    return m_aValues;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void PropertyMap::Insert( PropertyIds eId, bool bIsTextProperty, const uno::Any& rAny, bool bOverwrite )
{
//    const ::rtl::OUString& rInsert = PropertyNameSupplier::
//                           GetPropertyNameSupplier().GetName(eId);
    PropertyMap::iterator aElement = find(PropertyDefinition( eId, bIsTextProperty ) );
    if( aElement != end())
    {
        if(!bOverwrite)
            return;
        erase( aElement );
    }
    _PropertyMap::insert( PropertyMap::value_type
                          (PropertyDefinition( eId, bIsTextProperty),
                           rAny ));
    Invalidate();
}
/*-- 13.12.2006 10:46:42---------------------------------------------------

  -----------------------------------------------------------------------*/
template<class T>
    struct removeExistingElements : public ::std::unary_function<T, void>
{
  PropertyMap& rMap;

  removeExistingElements(PropertyMap& _rMap ) : rMap(_rMap) {}
  void operator() (T x)
  {
    PropertyMap::iterator aElement = rMap.find(x.first);
    if( aElement != rMap.end())
        rMap.erase( aElement );
  }
};
/*-- 13.12.2006 10:46:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void PropertyMap::insert( const PropertyMapPtr pMap, bool bOverwrite )
{
    if( pMap.get() )
    {
        if( bOverwrite )
            ::std::for_each( pMap->begin(), pMap->end(), removeExistingElements<PropertyMap::value_type>(*this) );
        _PropertyMap::insert(pMap->begin(), pMap->end());
    }
}
/*-- 06.06.2007 15:49:09---------------------------------------------------

  -----------------------------------------------------------------------*/
const uno::Reference< text::XFootnote>&  PropertyMap::GetFootnote() const
{
    return m_xFootnote;
}
/*-- 24.07.2006 08:29:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionPropertyMap::SectionPropertyMap(bool bIsFirstSection) :
    m_bIsFirstSection( bIsFirstSection )
    ,m_bTitlePage( false )
    ,m_nColumnCount( 0 )
    ,m_nColumnDistance( 1249 )
    ,m_bSeparatorLineIsOn( false )
    ,m_bEvenlySpaced( false )
    ,m_bIsLandscape( false )
    ,m_bPageNoRestart( false )
    ,m_nPageNumber( -1 )
    ,m_nBreakType( -1 )
    ,m_nPaperBin( -1 )
    ,m_nFirstPaperBin( -1 )
    ,m_nLeftMargin( 3175 ) //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    ,m_nRightMargin( 3175 )//page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    ,m_nTopMargin( 2540 )
    ,m_nBottomMargin( 2540 )
    ,m_nHeaderTop( 1270 ) //720 twip
    ,m_nHeaderBottom( 1270 )//720 twip
    ,m_nDzaGutter( 0 )
    ,m_bGutterRTL( false )
    ,m_bSFBiDi( false )
    ,m_nGridLinePitch( 1 )
    ,m_nDxtCharSpace( 0 )
    ,m_nLnnMod( 0 )
    ,m_nLnc( 0 )
    ,m_ndxaLnn( 0 )
    ,m_nLnnMin( 0 )
{
    static sal_Int32 nNumber = 0;
    nSectionNumber = nNumber++;
    memset(&m_pBorderLines, 0x00, sizeof(m_pBorderLines));
    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
        m_nBorderDistances[ nBorder ] = -1;
    //todo: set defaults in ApplyPropertiesToPageStyles
    //initialize defaults
    //page height, todo: rounded to default values, default: 0x3dc0 (15808) twip  27883 1/100 mm
    Insert( PROP_HEIGHT, false, uno::makeAny( (sal_Int32) 27883 ) );
    //page width, todo: rounded to default values, default 0x2fd0 (12240) twip -> 21590 1/100 mm
    Insert( PROP_WIDTH, false, uno::makeAny( (sal_Int32)21590 ) );
    //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_LEFT_MARGIN, false, uno::makeAny( (sal_Int32) 3175 ) );
    //page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_RIGHT_MARGIN, false, uno::makeAny( (sal_Int32) 3175 ) );
    //page top margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_TOP_MARGIN, false, uno::makeAny( (sal_Int32)2540 ) );
    //page bottom margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, false, uno::makeAny( (sal_Int32) 2540 ) );
    uno::Any aFalse( ::uno::makeAny( false ) );
    Insert( PROP_GRID_DISPLAY, false, aFalse);
    Insert( PROP_GRID_PRINT, false, aFalse);



    if( m_bIsFirstSection )
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        m_sFirstPageStyleName = rPropNameSupplier.GetName( PROP_FIRST_PAGE );
        m_sFollowPageStyleName = rPropNameSupplier.GetName( PROP_STANDARD );
    }
}
/*-- 24.07.2006 08:29:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionPropertyMap::~SectionPropertyMap()
{
    for( sal_Int16 ePos = BORDER_LEFT; ePos <= BORDER_BOTTOM; ++ePos)
        delete m_pBorderLines[ePos];
}
/*-- 24.07.2006 08:31:07---------------------------------------------------

  -----------------------------------------------------------------------*/
const ::rtl::OUString&  SectionPropertyMap::GetPageStyleName( bool bFirst )
{
    return bFirst ? m_sFirstPageStyleName : m_sFollowPageStyleName;
}
/*-- 24.07.2006 08:31:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SectionPropertyMap::SetPageStyleName( bool bFirst, const ::rtl::OUString& rName)
{
    if( bFirst )
        m_sFirstPageStyleName = rName;
    else
        m_sFollowPageStyleName = rName;
}
/*-- 24.07.2006 09:41:20---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString lcl_FindUnusedPageStyleName(const uno::Sequence< ::rtl::OUString >& rPageStyleNames)
{
    static const sal_Char cDefaultStyle[] = "Converted";
    //find the hightest number x in each style with the name "cDefaultStyle+x" and
    //return an incremented name
    sal_Int32 nMaxIndex = 0;
    const sal_Int32 nDefaultLength = sizeof(cDefaultStyle)/sizeof(sal_Char) - 1;
    const ::rtl::OUString sDefaultStyle( cDefaultStyle, nDefaultLength, RTL_TEXTENCODING_ASCII_US );

    const ::rtl::OUString* pStyleNames = rPageStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < rPageStyleNames.getLength(); ++nStyle)
    {
        if( pStyleNames[nStyle].getLength() > nDefaultLength &&
                !rtl_ustr_compare_WithLength( sDefaultStyle, nDefaultLength, pStyleNames[nStyle], nDefaultLength))
        {
            sal_Int32 nIndex = pStyleNames[nStyle].copy( nDefaultLength ).toInt32();
            if( nIndex > nMaxIndex)
                nMaxIndex = nIndex;
        }
    }
    ::rtl::OUString sRet( sDefaultStyle );
    sRet += rtl::OUString::valueOf( nMaxIndex + 1);
    return sRet;
}

/*-- 28.07.2006 13:00:43---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySet > SectionPropertyMap::GetPageStyle(
        const uno::Reference< container::XNameContainer >& xPageStyles,
        const uno::Reference < lang::XMultiServiceFactory >& xTextFactory,
        bool bFirst )
{
    uno::Reference< beans::XPropertySet > xRet;
    try
    {
        if( bFirst )
        {
            if( !m_sFirstPageStyleName.getLength() )
            {
                uno::Sequence< ::rtl::OUString > aPageStyleNames = xPageStyles->getElementNames();
                m_sFirstPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                m_aFirstPageStyle = uno::Reference< beans::XPropertySet > (
                        xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.PageStyle") )),
                        uno::UNO_QUERY);
                xPageStyles->insertByName( m_sFirstPageStyleName, uno::makeAny(m_aFirstPageStyle) );
            }
            else if( !m_aFirstPageStyle.is() )
            {
                xPageStyles->getByName(m_sFirstPageStyleName) >>= m_aFirstPageStyle;
            }
            xRet = m_aFirstPageStyle;
        }
        else
        {
            if( !m_sFollowPageStyleName.getLength() )
            {
                uno::Sequence< ::rtl::OUString > aPageStyleNames = xPageStyles->getElementNames();
                m_sFollowPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                m_aFollowPageStyle = uno::Reference< beans::XPropertySet > (
                        xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.PageStyle") )),
                        uno::UNO_QUERY);
                xPageStyles->insertByName( m_sFollowPageStyleName, uno::makeAny(m_aFollowPageStyle) );
            }
            else if(!m_aFollowPageStyle.is() )
            {
                xPageStyles->getByName(m_sFollowPageStyleName) >>= m_aFollowPageStyle;
            }
            xRet = m_aFollowPageStyle;
        }

    }
    catch( const uno::Exception& )
    {
    }

    return xRet;
}
/*-- 28.07.2006 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const table::BorderLine& rBorderLine )
{
    delete m_pBorderLines[ePos];
    m_pBorderLines[ePos] = new table::BorderLine( rBorderLine );
    m_nBorderDistances[ePos] = nLineDistance;
}
/*-- 28.07.2006 10:56:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::ApplyBorderToPageStyles(
            const uno::Reference< container::XNameContainer >& xPageStyles,
            const uno::Reference < lang::XMultiServiceFactory >& xTextFactory,
        sal_Int32 nValue )
{
            /*
            page border applies to:
            nIntValue & 0x07 ->
            0 all pages in this section
            1 first page in this section
            2 all pages in this section but first
            3 whole document (all sections)
            nIntValue & 0x18 -> page border depth 0 - in front 1- in back
            nIntValue & 0xe0 ->
            page border offset from:
            0 offset from text
            1 offset from edge of page
            */
    uno::Reference< beans::XPropertySet >  xFirst;
    uno::Reference< beans::XPropertySet >  xSecond;
    sal_Int32 nOffsetFrom = (nValue & 0x00E0) >> 5;
    //sal_Int32 bPageDepth = (nValue & 0x0018) >> 3; //unused infromation: 0 - in front 1 - in back
    //todo: negative spacing (from ww8par6.cxx)
    if( nOffsetFrom == 1 )
    {
//        USHORT nDist;
//        if (aBox.GetLeft())
//        {
//            nDist = aBox.GetDistance(BOX_LINE_LEFT);
//    lcl_MakeSafeNegativeSpacing( ) sets the distance to 0 if  > SHRT_MAX
//
//            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<USHORT>(aLR.GetLeft() - nDist)), BOX_LINE_LEFT);
//            aSizeArray[WW8_LEFT] =
//                aSizeArray[WW8_LEFT] - nDist + aBox.GetDistance(BOX_LINE_LEFT);
//        }
        //the same for right, top, bottom

    }
    switch( nValue & 0x07)
    {
        case 0: /*all styles*/
            xFirst = GetPageStyle( xPageStyles, xTextFactory, false );
            xSecond = GetPageStyle( xPageStyles, xTextFactory, true );
        break;
        case 1: /*first page*/
            xFirst = GetPageStyle( xPageStyles, xTextFactory, true );
        break;
        case 2: /*left and right*/
            xFirst  = GetPageStyle( xPageStyles, xTextFactory, false );
        break;
        case 3: //whole document?
            //todo: how to apply a border to the whole document - find all sections or access all page styles?
        default:
            return;
    }
    //has to be sorted like enum BorderPosition: l-r-t-b
    static const PropertyIds aBorderIds[4] =
    {
        PROP_LEFT_BORDER,
        PROP_RIGHT_BORDER,
        PROP_TOP_BORDER,
        PROP_BOTTOM_BORDER
    };
    static const PropertyIds aBorderDistanceIds[4] =
    {
        PROP_LEFT_BORDER_DISTANCE,
        PROP_RIGHT_BORDER_DISTANCE,
        PROP_TOP_BORDER_DISTANCE,
        PROP_BOTTOM_BORDER_DISTANCE,
    };
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder)
    {
        if( m_pBorderLines[nBorder] )
        {
            const ::rtl::OUString sBorderName = rPropNameSupplier.GetName( aBorderIds[nBorder] );
            xFirst->setPropertyValue( sBorderName, uno::makeAny( *m_pBorderLines[nBorder] ));
            if(xSecond.is())
                xSecond->setPropertyValue( sBorderName, uno::makeAny( *m_pBorderLines[nBorder] ));
        }
        if( m_nBorderDistances[nBorder] >= 0 )
        {
            const ::rtl::OUString sBorderDistanceName = rPropNameSupplier.GetName( aBorderDistanceIds[nBorder] );
            xFirst->setPropertyValue( sBorderDistanceName, uno::makeAny( m_nBorderDistances[nBorder] ));
            if(xSecond.is())
                xSecond->setPropertyValue( sBorderDistanceName, uno::makeAny( m_nBorderDistances[nBorder] ));
        }
    }

//                rContext->Insert( aBorderIds[nId - 0x702B], uno::makeAny( aBorderLine ));
//                rContext->Insert( aBorderDistanceIds[nId - 0x702B], uno::makeAny( nLineDistance) );

//    uno::Reference< beans::XPropertySet > xStyle = GetPageStyle( ePageType );

}
/*-- 14.12.2006 12:50:06---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextColumns > SectionPropertyMap::ApplyColumnProperties(
                            uno::Reference< beans::XPropertySet > xColumnContainer )
{
    uno::Reference< text::XTextColumns > xColumns;
    try
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        const ::rtl::OUString sTextColumns = rPropNameSupplier.GetName( PROP_TEXT_COLUMNS );
        xColumnContainer->getPropertyValue(sTextColumns) >>= xColumns;
        uno::Reference< beans::XPropertySet > xColumnPropSet( xColumns, uno::UNO_QUERY_THROW );
        if( !m_bEvenlySpaced &&
                (sal_Int32(m_aColWidth.size()) == (m_nColumnCount + 1 )) &&
                (sal_Int32(m_aColDistance.size()) == m_nColumnCount))
        {
            //the column width in word is an absolute value, in OOo it's relative
            //the distances are both absolute
            sal_Int32 nColSum = 0;
            for( sal_Int32 nCol = 0; nCol <= m_nColumnCount; ++nCol)
            {
                nColSum += m_aColWidth[nCol];
                if(nCol)
                    nColSum += m_aColDistance[nCol -1];
            }

            sal_Int32 nRefValue = xColumns->getReferenceValue();
            double fRel = double( nRefValue ) / double( nColSum );
            uno::Sequence< text::TextColumn > aColumns( m_nColumnCount + 1 );
            text::TextColumn* pColumn = aColumns.getArray();

            nColSum = 0;
            for( sal_Int32 nCol = 0; nCol <= m_nColumnCount; ++nCol)
            {
                //nColSum : nRefValue == (nAbsColWidth + colDist /2) : nRelColWidth;
                pColumn[nCol].LeftMargin = nCol ? m_aColDistance[nCol - 1 ] / 2 : 0;
                pColumn[nCol].RightMargin = nCol == m_nColumnCount ? 0 : m_aColDistance[nCol] / 2;
                pColumn[nCol].Width = sal_Int32((double( m_aColWidth[nCol] + pColumn[nCol].RightMargin + pColumn[nCol].LeftMargin ) + 0.5 ) * fRel );
                nColSum += pColumn[nCol].Width;
            }
            if( nColSum != nRefValue )
                pColumn[m_nColumnCount].Width -= ( nColSum - nRefValue );
            xColumns->setColumns( aColumns );
        }
        else
        {
            xColumns->setColumnCount( m_nColumnCount + 1 );
            xColumnPropSet->setPropertyValue( rPropNameSupplier.GetName( PROP_AUTOMATIC_DISTANCE ), uno::makeAny( m_nColumnDistance ));
        }

        if(m_bSeparatorLineIsOn)
            xColumnPropSet->setPropertyValue(
                rPropNameSupplier.GetName( PROP_SEPARATOR_LINE_IS_ON ),
                uno::makeAny( m_bSeparatorLineIsOn ));
        xColumnContainer->setPropertyValue( sTextColumns, uno::makeAny( xColumns ) );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "Exception in SectionPropertyMap::ApplyColumnProperties");
    }
    return xColumns;
}

/*-- 20.12.2006 09:44:16---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SectionPropertyMap::HasHeader(bool bFirstPage) const
{
    bool bRet = false;
    if( (bFirstPage && m_aFirstPageStyle.is()) ||( !bFirstPage && m_aFollowPageStyle.is()) )
    {
        if( bFirstPage )
            m_aFirstPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_HEADER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_HEADER_IS_ON) ) >>= bRet;
    }
    return bRet;
}
/*-- 20.12.2006 09:44:16---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SectionPropertyMap::HasFooter(bool bFirstPage) const
{
    bool bRet = false;
    if( (bFirstPage && m_aFirstPageStyle.is()) ||( !bFirstPage && m_aFollowPageStyle.is()) )
    {
        if( bFirstPage )
            m_aFirstPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_FOOTER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_FOOTER_IS_ON) ) >>= bRet;
    }
    return bRet;
}
/*-- 20.12.2006 09:41:56---------------------------------------------------

  -----------------------------------------------------------------------*/
#define MIN_HEAD_FOOT_HEIGHT 100 //minimum header/footer height

void SectionPropertyMap::PrepareHeaderFooterProperties( bool bFirstPage )
{
    sal_Int32 nTopMargin = m_nTopMargin;
    if(HasHeader(bFirstPage))
    {
        m_nTopMargin = m_nHeaderTop;
        if( nTopMargin > 0 && nTopMargin > m_nHeaderTop )
            m_nHeaderTop = nTopMargin - m_nHeaderTop;
        else
            m_nHeaderTop = 0;

        //minimum header height 1mm
        if( m_nHeaderTop < MIN_HEAD_FOOT_HEIGHT )
            m_nHeaderTop = MIN_HEAD_FOOT_HEIGHT;
    }


    if( nTopMargin >= 0 ) //fixed height header -> see WW8Par6.hxx
    {
        operator[]( PropertyDefinition( PROP_HEADER_IS_DYNAMIC_HEIGHT, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_HEADER_DYNAMIC_SPACING, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_HEADER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderTop - MIN_HEAD_FOOT_HEIGHT );// ULSpace.Top()
        operator[]( PropertyDefinition( PROP_HEADER_HEIGHT, false )) =  uno::makeAny( m_nHeaderTop );

    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        operator[]( PropertyDefinition( PROP_HEADER_HEIGHT, false )) =  uno::makeAny( m_nHeaderTop );
        operator[]( PropertyDefinition( PROP_HEADER_BODY_DISTANCE, false )) = uno::makeAny( nTopMargin - m_nHeaderTop );
        operator[]( PropertyDefinition( PROP_HEADER_IS_DYNAMIC_HEIGHT, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_HEADER_DYNAMIC_SPACING, false)) = uno::makeAny( false );
    }

    sal_Int32 nBottomMargin = m_nBottomMargin;
    if( HasFooter( bFirstPage ) )
    {
        m_nBottomMargin = m_nHeaderBottom;
        if( nBottomMargin > 0 && nBottomMargin > m_nHeaderBottom )
            m_nHeaderBottom = nBottomMargin - m_nHeaderBottom;
        else
            m_nHeaderBottom = 0;
        if( m_nHeaderBottom < MIN_HEAD_FOOT_HEIGHT )
            m_nHeaderBottom = MIN_HEAD_FOOT_HEIGHT;
    }

    if( nBottomMargin >= 0 ) //fixed height footer -> see WW8Par6.hxx
    {
        operator[]( PropertyDefinition( PROP_FOOTER_IS_DYNAMIC_HEIGHT, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_FOOTER_DYNAMIC_SPACING, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_FOOTER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderBottom - MIN_HEAD_FOOT_HEIGHT);
        operator[]( PropertyDefinition( PROP_FOOTER_HEIGHT, false )) =  uno::makeAny( m_nHeaderBottom );
    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        operator[]( PropertyDefinition( PROP_FOOTER_IS_DYNAMIC_HEIGHT, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_FOOTER_DYNAMIC_SPACING, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_FOOTER_HEIGHT, false )) =  uno::makeAny( nBottomMargin - m_nHeaderBottom );
        operator[]( PropertyDefinition( PROP_FOOTER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderBottom );
    }

    //now set the top/bottom margin for the follow page style
    operator[]( PropertyDefinition( PROP_TOP_MARGIN, false )) = uno::makeAny( m_nTopMargin );
    operator[]( PropertyDefinition( PROP_BOTTOM_MARGIN, false )) = uno::makeAny( m_nBottomMargin );
}
/*-- 11.12.2006 08:31:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::CloseSectionGroup( DomainMapper_Impl& rDM_Impl )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    if( m_nLnnMod )
    {
        bool bFirst = rDM_Impl.IsLineNumberingSet();
        rDM_Impl.SetLineNumbering( m_nLnnMod, m_nLnc, m_ndxaLnn );
        if( m_nLnnMin > 0 || (bFirst && m_nLnc == 1))
        {
            //set the starting value at the beginning of the section
            try
            {
                uno::Reference< beans::XPropertySet > xRangeProperties;
                if( m_xStartingRange.is() )
                {
                    xRangeProperties = uno::Reference< beans::XPropertySet >( m_xStartingRange, uno::UNO_QUERY_THROW );
                }
                else
                {
                    //set the start value at the beginning of the document
                    xRangeProperties = uno::Reference< beans::XPropertySet >( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
                }
                xRangeProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_PARA_LINE_NUMBER_START_VALUE ), uno::makeAny( m_nLnnMin + 1 ));
            }
            catch( const uno::Exception& )
            {
                OSL_ENSURE( false, "Exception in SectionPropertyMap::CloseSectionGroup");
            }
        }
    }

    //depending on the break type no page styles should be created
    if(m_nBreakType == 0)
    {
        //todo: insert a section or access the already inserted section
        //-->debug
//        ::rtl::OUString sSelection = m_xStartingRange->getString();
//        sSelection.getLength();
        //-->debug
        uno::Reference< beans::XPropertySet > xSection =
                                    rDM_Impl.appendTextSectionAfter( m_xStartingRange );
        if( m_nColumnCount > 0 && xSection.is())
            ApplyColumnProperties( xSection );
    }
    else
    {
        //get the properties and create appropriate page styles
        uno::Reference< beans::XPropertySet > xFollowPageStyle = GetPageStyle( rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), false );

        if( m_nDzaGutter > 0 )
        {
            //todo: iGutterPos from DocProperties are missing
            // if( m_iGutterPos > 0 ) m_nTopMargin += m_nDzaGutter; else
            if( m_bGutterRTL )
                m_nRightMargin += m_nDzaGutter;
            else
                m_nLeftMargin += m_nDzaGutter;
        }
        operator[]( PropertyDefinition( PROP_LEFT_MARGIN, false )) =  uno::makeAny( m_nLeftMargin  );
        operator[]( PropertyDefinition( PROP_RIGHT_MARGIN, false )) = uno::makeAny( m_nRightMargin );

//        if( iGutterPos && fRTLGutter )
//        m_nTopMargin += nGutter

        /*** if headers/footers are available then the top/bottom margins of the
            header/footer are copied to the top/bottom margin of the page
          */
        PrepareHeaderFooterProperties( false );

        const ::rtl::OUString sTrayIndex = rPropNameSupplier.GetName( PROP_PRINTER_PAPER_TRAY_INDEX );
        if( m_nPaperBin >= 0 )
            xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( m_nPaperBin ) );
        uno::Reference< text::XTextColumns > xColumns;
        if( m_nColumnCount > 0 )
            xColumns = ApplyColumnProperties( xFollowPageStyle );

        //prepare text grid properties
        sal_Int32 nHeight = 1;
        PropertyMap::iterator aElement = find(PropertyDefinition( PROP_HEIGHT, false ));
        if( aElement != end())
            aElement->second >>= nHeight;

        sal_Int32 nWidth = 1;
        aElement = find(PropertyDefinition( PROP_WIDTH, false ));
        if( aElement != end())
            aElement->second >>= nWidth;

        text::WritingMode eWritingMode = text::WritingMode_LR_TB;
        aElement = find(PropertyDefinition( PROP_WRITING_MODE, false ));
        if( aElement != end())
            aElement->second >>= eWritingMode;



        sal_Int32 nTextAreaHeight = eWritingMode == text::WritingMode_LR_TB ?
            nHeight - m_nTopMargin - m_nBottomMargin :
            nWidth - m_nLeftMargin - m_nRightMargin;

        operator[]( PropertyDefinition( PROP_GRID_LINES, false )) =
                uno::makeAny( static_cast<sal_Int16>(nTextAreaHeight/m_nGridLinePitch));

        sal_Int32 nCharWidth = 423; //240 twip/ 12 pt
        //todo: is '0' the right index here?
        const StyleSheetEntry* pEntry = rDM_Impl.GetStyleSheetTable()->FindStyleSheetByISTD(::rtl::OUString::valueOf(static_cast<sal_Int32>(0), 16));
        if( pEntry )
        {
            PropertyMap::iterator aElement_ = pEntry->pProperties->find(PropertyDefinition( PROP_CHAR_HEIGHT_ASIAN, false ));
            if( aElement_ != pEntry->pProperties->end())
            {
                double fHeight = 0;
                if( aElement_->second >>= fHeight )
                    nCharWidth = ConversionHelper::convertTwipToMM100( (long)( fHeight * 20.0 + 0.5 ));
            }
        }

        //dxtCharSpace
        if(m_nDxtCharSpace)
        {
            sal_Int32 nCharSpace = m_nDxtCharSpace;
            //main lives in top 20 bits, and is signed.
            sal_Int32 nMain = (nCharSpace & 0xFFFFF000);
            nMain /= 0x1000;
            nCharWidth += ConversionHelper::convertTwipToMM100( nMain * 20 );

            sal_Int32 nFraction = (nCharSpace & 0x00000FFF);
            nFraction = (nFraction * 20)/0xFFF;
            nCharWidth += ConversionHelper::convertTwipToMM100( nFraction );
        }
        operator[]( PropertyDefinition( PROP_GRID_BASE_HEIGHT, false )) = uno::makeAny( nCharWidth );
        sal_Int32 nRubyHeight = m_nGridLinePitch - nCharWidth;
        if(nRubyHeight < 0 )
            nRubyHeight = 0;
        operator[]( PropertyDefinition( PROP_GRID_RUBY_HEIGHT, false )) = uno::makeAny( nRubyHeight );

        _ApplyProperties( xFollowPageStyle );

        //todo: creating a "First Page" style depends on HasTitlePage und _fFacingPage_
        if( m_bTitlePage )
        {
            PrepareHeaderFooterProperties( true );
            uno::Reference< beans::XPropertySet > xFirstPageStyle = GetPageStyle(
                                rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), true );
            _ApplyProperties( xFirstPageStyle );
            sal_Int32 nPaperBin = m_nFirstPaperBin >= 0 ? m_nFirstPaperBin : m_nPaperBin >= 0 ? m_nPaperBin : 0;
            if( nPaperBin )
                xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( nPaperBin ) );
            if( xColumns.is() )
                xFollowPageStyle->setPropertyValue(
                    rPropNameSupplier.GetName( PROP_TEXT_COLUMNS ), uno::makeAny( xColumns ));
        }
        try
        {
//            if( m_xStartingRange.is() )
            {
                //now apply this break at the first paragraph of this section
                uno::Reference< beans::XPropertySet > xRangeProperties( m_xStartingRange, uno::UNO_QUERY_THROW );
            /* break type
            0 - No break 1 - New Colunn 2 - New page 3 - Even page 4 - odd page */
                if( m_nBreakType == 2 || m_nBreakType == 3)
                {
                    xRangeProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_BREAK_TYPE),
                        uno::makeAny( m_nBreakType == 2 ? style::BreakType_COLUMN_AFTER : style::BreakType_PAGE_AFTER  ) );
                }
                else
                {
                    xRangeProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_DESC_NAME ),
                        uno::makeAny( m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName ));
    //  todo: page breaks with odd/even page numbering are not available - find out current page number to check how to change the number
    //  or add even/odd page break types
                    if(m_bPageNoRestart || m_nPageNumber >= 0)
                    {
                        sal_Int16 nPageNumber = m_nPageNumber >= 0 ? static_cast< sal_Int16 >(m_nPageNumber) : 1;
                        xRangeProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_NUMBER_OFFSET ),
                            uno::makeAny( nPageNumber ));
                    }
                }
            }
        }
        catch( const uno::Exception& rEx)
        {
            OSL_ENSURE( false, "Exception in SectionPropertyMap::CloseSectionGroup");
            (void)rEx;
       }
    }
}
/*-- 11.12.2006 08:31:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::_ApplyProperties( uno::Reference< beans::XPropertySet > xStyle )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    PropertyMap::iterator aMapIter = begin();
    while( aMapIter != end())
    {
        try
        {
            xStyle->setPropertyValue( rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second );
        }
        catch( const uno::Exception& )
        {
            OSL_ENSURE( false, "Exception in <PageStyle>::setPropertyValue");
        }
        ++aMapIter;
    }
}
sal_Int32 lcl_AlignPaperBin( sal_Int32 nSet )
{
    //default tray numbers are above 0xff
    if( nSet > 0xff )
        nSet = nSet >> 8;
    //there are some special numbers which can't be handled easily
    //1, 4, 15, manual tray, upper tray, auto select? see ww8atr.cxx
    //todo: find out appropriate conversion
    return nSet;
}
/*-- 13.12.2006 15:34:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::SetPaperBin( sal_Int32 nSet )
{
    m_nPaperBin = lcl_AlignPaperBin( nSet );
}
/*-- 13.12.2006 15:34:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::SetFirstPaperBin( sal_Int32 nSet )
{
    m_nFirstPaperBin = lcl_AlignPaperBin( nSet );
}
/*-- 14.06.2007 13:57:42---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetPropertyMap::StyleSheetPropertyMap() :
//    mnCT_Spacing_after( 0 ),
    mnCT_Spacing_line( 0 ),
    mnCT_Spacing_lineRule( 0 ),
    mbCT_TrPrBase_tblHeader( false ),
    mnCT_TrPrBase_jc( 0 ),
    mnCT_TcPrBase_vAlign( 0 ),
    mnCT_TblWidth_w( 0 ),
    mnCT_TblWidth_type( 0 ),
//    mbCT_Spacing_afterSet( false ),
    mbCT_Spacing_lineSet( false ),
    mbCT_Spacing_lineRuleSet( false ),
    mbCT_TrPrBase_tblHeaderSet( false ),
    mbCT_TrPrBase_jcSet( false ),
    mbCT_TcPrBase_vAlignSet( false ),
    mbCT_TblWidth_wSet( false ),
    mbCT_TblWidth_typeSet( false ),
    mnListId( -1 ),
    mnListLevel( -1 )
{
}
/*-- 14.06.2007 13:57:43---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetPropertyMap::~StyleSheetPropertyMap()
{
}
/*-- 28.12.2007 08:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
ParagraphProperties::ParagraphProperties() :
    m_bFrameMode( false ),
    m_nDropCap(NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none),
    m_nLines(0),
    m_w(-1),
    m_h(-1),
    m_nWrap(-1),
    m_hAnchor(-1),
    m_vAnchor(-1),
    m_x(-1),
    m_bxValid( false ),
    m_y(-1),
    m_byValid( false ),
    m_hSpace(-1),
    m_vSpace(-1),
    m_hRule(-1),
    m_xAlign(-1),
    m_yAlign(-1),
    m_bAnchorLock(false),
    m_nDropCapLength(0)
{
}
/*-- 28.12.2007 08:28:24---------------------------------------------------

  -----------------------------------------------------------------------*/
ParagraphProperties::ParagraphProperties(const ParagraphProperties& rCopy) :
    m_bFrameMode ( rCopy.m_bFrameMode),
    m_nDropCap   ( rCopy.m_nDropCap),
    m_nLines     ( rCopy.m_nLines),
    m_w          ( rCopy.m_w),
    m_h          ( rCopy.m_h),
    m_nWrap      ( rCopy.m_nWrap),
    m_hAnchor    ( rCopy.m_hAnchor),
    m_vAnchor    ( rCopy.m_vAnchor),
    m_x          ( rCopy.m_x),
    m_bxValid    ( rCopy.m_bxValid),
    m_y          ( rCopy.m_y),
    m_byValid    ( rCopy.m_byValid),
    m_hSpace     ( rCopy.m_hSpace),
    m_vSpace     ( rCopy.m_vSpace),
    m_hRule      ( rCopy.m_hRule),
    m_xAlign     ( rCopy.m_xAlign),
    m_yAlign     ( rCopy.m_yAlign),
    m_bAnchorLock( rCopy.m_bAnchorLock),
    m_nDropCapLength( rCopy.m_nDropCapLength ),
    m_sParaStyleName( rCopy.m_sParaStyleName),
    m_xStartingRange( rCopy.m_xStartingRange ),
    m_xEndingRange( rCopy.m_xEndingRange)
{
}
/*-- 28.12.2007 11:29:18---------------------------------------------------

  -----------------------------------------------------------------------*/
ParagraphProperties::~ParagraphProperties()
{
}
/*-- 28.12.2007 09:05:45---------------------------------------------------

  -----------------------------------------------------------------------*/
int ParagraphProperties::operator==(const ParagraphProperties& rCompare)
{
    return
        m_bFrameMode == rCompare.m_bFrameMode &&
        m_nDropCap   == rCompare.m_nDropCap &&
        m_nLines     == rCompare.m_nLines &&
        m_w          == rCompare.m_w &&
        m_h          == rCompare.m_h &&
        m_nWrap      == rCompare.m_nWrap &&
        m_hAnchor    == rCompare.m_hAnchor &&
        m_vAnchor    == rCompare.m_vAnchor &&
        m_x          == rCompare.m_x &&
        m_bxValid    == rCompare.m_bxValid &&
        m_y          == rCompare.m_y &&
        m_byValid    == rCompare.m_byValid &&
        m_hSpace     == rCompare.m_hSpace &&
        m_vSpace     == rCompare.m_vSpace &&
        m_hRule      == rCompare.m_hRule &&
        m_xAlign     == rCompare.m_xAlign &&
        m_yAlign     == rCompare.m_yAlign &&
        m_bAnchorLock== rCompare.m_bAnchorLock;
}
/*-- 27.12.2007 13:32:36---------------------------------------------------

  -----------------------------------------------------------------------*/
ParagraphPropertyMap::ParagraphPropertyMap()
{
}
/*-- 27.12.2007 13:32:36---------------------------------------------------

  -----------------------------------------------------------------------*/
ParagraphPropertyMap::~ParagraphPropertyMap()
{
}


}//namespace dmapper
}//namespace writerfilter
