/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2006-12-13 14:51:20 $
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
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif

using namespace ::com::sun::star;

namespace dmapper{

/*-- 21.06.2006 09:30:56---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMap::PropertyMap()
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
        const ::rtl::OUString& rParaStyle = rPropNameSupplier.GetName( PROP_PARA_STYLE_NAME );
        PropertyMap::iterator aParaStyleIter = find(rParaStyle);
        if( aParaStyleIter != end())
        {
            pValues[nValue].Name = aParaStyleIter->first;
            pValues[nValue].Value = aParaStyleIter->second;
            ++nValue;
        }

        const ::rtl::OUString& rCharStyle = rPropNameSupplier.GetName( PROP_CHAR_STYLE_NAME );
        PropertyMap::iterator aCharStyleIter = find(rCharStyle);
        if( aCharStyleIter != end())
        {
            pValues[nValue].Name = aCharStyleIter->first;
            pValues[nValue].Value = aCharStyleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aMapIter = begin();
        for( ; nValue < m_aValues.getLength(); ++aMapIter )
        {
            if( aMapIter != aParaStyleIter && aMapIter != aCharStyleIter)
            {
                pValues[nValue].Name = aMapIter->first;
                pValues[nValue].Value = aMapIter->second;
                ++nValue;
            }
        }
    }
    return m_aValues;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void PropertyMap::Insert( PropertyIds eId, const uno::Any& rAny )
{
    const ::rtl::OUString& rInsert = PropertyNameSupplier::
                           GetPropertyNameSupplier().GetName(eId);
    PropertyMap::iterator aElement = find(rInsert);
    if( aElement != end())
        erase( aElement );
    _PropertyMap::insert( PropertyMap::value_type
                          (rInsert,
                           rAny));
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
void PropertyMap::insert( PropertyMapPtr pMap )
{
    ::std::for_each( pMap->begin(), pMap->end(), removeExistingElements<PropertyMap::value_type>(*this) );
    _PropertyMap::insert(pMap->begin(), pMap->end());
}
/*-- 24.07.2006 08:29:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionPropertyMap::SectionPropertyMap() :
    m_bTitlePage( false )
    ,m_nColumnCount( 0 )
    ,m_nColumnDistance( 1249 )
    ,m_bSeparatorLineIsOn( false )
    ,m_bEvenlySpaced( false )
    ,m_bIsLandscape( false )
    ,m_nPageNumber( -1 )
    ,m_nBreakType( -1 )
    ,m_nPaperBin( -1 )
    ,m_nFirstPaperBin( -1 )
{
    memset(&m_pBorderLines, 0x00, sizeof(m_pBorderLines));
    memset(&m_nBorderDistances, 0x00, sizeof(m_nBorderDistances));
    //todo: set defaults in ApplyPropertiesToPageStyles
    //initialize defaults
    //page height, todo: rounded to default values, default: 0x3dc0 (15808) twip  27883 1/100 mm
    Insert( PROP_HEIGHT, uno::makeAny( (sal_Int32) 27883 ) );
    //page width, todo: rounded to default values, default 0x2fd0 (12240) twip -> 21590 1/100 mm
    Insert( PROP_WIDTH, uno::makeAny( (sal_Int32)21590 ) );
    //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_LEFT_MARGIN, uno::makeAny( (sal_Int32) 3175 ) );
    //page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_RIGHT_MARGIN, uno::makeAny( (sal_Int32) 3175 ) );
    //page top margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_TOP_MARGIN, uno::makeAny( (sal_Int32)2540 ) );
    //page bottom margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, uno::makeAny( (sal_Int32) 2540 ) );


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
            const uno::Reference< container::XNameContainer >& /*xStyles*/,
            const uno::Reference < lang::XMultiServiceFactory >& /*xTextFactory*/,
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
    switch( nValue & 0x07)
    {
        case 0: /*all styles*/ break;
        case 1: /*first page*/ break;
        case 2: /*left and right*/ break;
        case 3: //whole document?
        break;
    }
//    uno::Reference< beans::XPropertySet > xStyle = GetPageStyle( ePageType );

}
/*-- 11.12.2006 08:31:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::ApplyPropertiesToPageStyles(
    const uno::Reference< container::XNameContainer >& xStyles,
    const uno::Reference < lang::XMultiServiceFactory >& xTextFactory )
{
    //depending on the break type no page styles should be created
    if(m_nBreakType == 0)
    {
        //todo: insert a section or access the already inserted section
    }
    else
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        //get the properties and create appropriate page styles
        uno::Reference< beans::XPropertySet > xFollowPageStyle = GetPageStyle( xStyles, xTextFactory, false );
        _ApplyProperties( xFollowPageStyle );
        const ::rtl::OUString sTrayIndex = rPropNameSupplier.GetName( PROP_PRINTER_PAPER_TRAY_INDEX );
        if( m_nPaperBin >= 0 )
            xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( m_nPaperBin ) );

        uno::Reference< text::XTextColumns > xColumns;
        const ::rtl::OUString sTextColumns = rPropNameSupplier.GetName( PROP_TEXT_COLUMNS );
        if( m_nColumnCount > 0 )
        {
            try
            {
                xFollowPageStyle->getPropertyValue(sTextColumns) >>= xColumns;
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
                xFollowPageStyle->setPropertyValue( sTextColumns, uno::makeAny( xColumns ) );
            }
            catch( const uno::Exception& )
            {
            }

        }

        //todo: creating a "First Page" style depends on HasTitlePage und _fFacingPage_
        if( m_bTitlePage )
        {
            uno::Reference< beans::XPropertySet > xFirstPageStyle = GetPageStyle( xStyles, xTextFactory, true );
            _ApplyProperties( xFirstPageStyle );
            sal_Int32 nPaperBin = m_nFirstPaperBin >= 0 ? m_nFirstPaperBin : m_nPaperBin >= 0 ? m_nPaperBin : 0;
            if( nPaperBin )
                xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( nPaperBin ) );
            if( xColumns.is() )
                xFollowPageStyle->setPropertyValue(sTextColumns, uno::makeAny( xColumns ));
        }
    }
}
/*-- 11.12.2006 08:31:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::_ApplyProperties( uno::Reference< beans::XPropertySet > xStyle )
{
    PropertyMap::iterator aMapIter = begin();
    try
    {
        for( ; aMapIter != end(); ++aMapIter )
        {
                xStyle->setPropertyValue( aMapIter->first, aMapIter->second );
        }
    }
    catch( const uno::Exception& )
    {
        OSL_ASSERT("Exception in <PageStyle>::setPropertyValue");
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

}//namespace dmapper
