/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2006-11-02 12:37:24 $
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
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
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
        PropertyMap::iterator aParaStyleIter = find(::rtl::OUString::createFromAscii("ParaStyleName"));
        if( aParaStyleIter != end())
        {
            pValues[nValue].Name = aParaStyleIter->first;
            pValues[nValue].Value = aParaStyleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aCharStyleIter = find(::rtl::OUString::createFromAscii("CharStyleName"));
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
    _PropertyMap::insert( PropertyMap::value_type
                          (PropertyNameSupplier::
                           GetPropertyNameSupplier().GetName(eId),
                           rAny));
    Invalidate();
}


void PropertyMap::insert( PropertyMapPtr pMap)
{
    _PropertyMap::insert(pMap->begin(), pMap->end());
}

/*-- 24.07.2006 08:29:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionPropertyMap::SectionPropertyMap()
{
    memset(&pBorderLines, 0x00, sizeof(pBorderLines));
    memset(&nBorderDistances, 0x00, sizeof(nBorderDistances));
}
/*-- 24.07.2006 08:29:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionPropertyMap::~SectionPropertyMap()
{
    for( sal_Int16 ePos = BORDER_LEFT; ePos <= BORDER_BOTTOM; ++ePos)
        delete pBorderLines[ePos];
}
/*-- 24.07.2006 08:31:07---------------------------------------------------

  -----------------------------------------------------------------------*/
const ::rtl::OUString&  SectionPropertyMap::GetPageStyleName( SectionPropertyMap::PageType eType)
{
    return sPageStyleNames[eType];
}
/*-- 24.07.2006 08:31:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SectionPropertyMap::SetPageStyleName( PageType eType, const ::rtl::OUString& rName)
{
    sPageStyleNames[eType] = rName;
}
/*-- 28.07.2006 13:00:43---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySet > SectionPropertyMap::GetPageStyle(
        const uno::Reference< container::XNameContainer >& xStyles, SectionPropertyMap::PageType eType)
{
    uno::Reference< beans::XPropertySet > xRet;
    try
    {
        if(!aPageStyles[eType].is() && sPageStyleNames[eType].getLength())
        {
            xStyles->getByName(sPageStyleNames[eType]) >>= aPageStyles[eType];
        }
        xRet = aPageStyles[eType];
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
    delete pBorderLines[ePos];
    pBorderLines[ePos] = new table::BorderLine( rBorderLine );
    nBorderDistances[ePos] = nLineDistance;
}
/*-- 28.07.2006 10:56:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionPropertyMap::ApplyBorderToPageStyles(
            const uno::Reference< container::XNameContainer >& /*xStyles*/,
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

}//namespace dmapper
