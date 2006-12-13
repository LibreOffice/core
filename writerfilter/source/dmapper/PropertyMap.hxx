/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.4 $
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
#define INCLUDED_DMAPPER_PROPERTYMAP_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYIDS_HXX
#include <PropertyIds.hxx>
#endif
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
namespace com{namespace sun{namespace star{
    namespace beans{
    struct PropertyValue;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace table{
        struct BorderLine;
    }
    namespace container{
        class XNameAccess;
        class XNameContainer;
    }
}}}

namespace dmapper{
enum BorderPosition
{
    BORDER_LEFT,
    BORDER_RIGHT,
    BORDER_TOP,
    BORDER_BOTTOM
};
/*-- 15.06.2006 08:22:33---------------------------------------------------

  -----------------------------------------------------------------------*/
typedef std::map < ::rtl::OUString, ::com::sun::star::uno::Any > _PropertyMap;
class PropertyMap : public _PropertyMap
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aValues;

    public:
        PropertyMap();
        virtual ~PropertyMap();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropertyValues();
    void PropertyMap::Insert( PropertyIds eId, const ::com::sun::star::uno::Any& rAny );

    void insert(const boost::shared_ptr<PropertyMap> pMap);

    void Invalidate()
        {
            if(m_aValues.getLength())
                m_aValues.realloc( 0 );
        }
};
typedef boost::shared_ptr<PropertyMap>  PropertyMapPtr;

/*-- 24.07.2006 08:26:33---------------------------------------------------

  -----------------------------------------------------------------------*/
class SectionPropertyMap : public PropertyMap
{
    //'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported
    ::rtl::OUString                                                             m_sFirstPageStyleName;
    ::rtl::OUString                                                             m_sFollowPageStyleName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_aFirstPageStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_aFollowPageStyle;

    ::com::sun::star::table::BorderLine*    m_pBorderLines[4];
    sal_Int32                               m_nBorderDistances[4];

    bool                                    m_bTitlePage;
    sal_Int16                               m_nColumnCount;
    sal_Int32                               m_nColumnDistance;
    ::std::vector< sal_Int32 >              m_aColWidth;
    ::std::vector< sal_Int32 >              m_aColDistance;

    bool                                    m_bSeparatorLineIsOn;
    bool                                    m_bEvenlySpaced;
    bool                                    m_bIsLandscape;

    sal_Int32                               m_nPageNumber;
    sal_Int32                               m_nBreakType;
    sal_Int32                               m_nPaperBin;
    sal_Int32                               m_nFirstPaperBin;


    void _ApplyProperties( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xStyle );

public:
        SectionPropertyMap();
        ~SectionPropertyMap();

    enum PageType
    {
        PAGE_FIRST,
        PAGE_LEFT,
        PAGE_RIGHT
    };

    const ::rtl::OUString&  GetPageStyleName( bool bFirst );
    void                    SetPageStyleName( bool bFirst, const ::rtl::OUString& rName);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > GetPageStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory,
            bool bFirst );

    void SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const ::com::sun::star::table::BorderLine& rBorderLine );

    void SetColumnCount( sal_Int16 nCount ) { m_nColumnCount = nCount; }
    void SetColumnDistance( sal_Int32 nDist ) { m_nColumnDistance = nDist; }
    void AppendColumnWidth( sal_Int32 nWidth ) { m_aColWidth.push_back( nWidth ); }
    void AppendColumnSpacing( sal_Int32 nDist ) {m_aColDistance.push_back( nDist ); }

    void SetTitlePage( bool bSet ) { m_bTitlePage = bSet; }
    void SetSeparatorLine( bool bSet ) { m_bSeparatorLineIsOn = bSet; }
    void SetEvenlySpaced( bool bSet ) {    m_bEvenlySpaced = bSet; }
    void SetLandscape( bool bSet ) { m_bIsLandscape = bSet; }
    void SetPageNumber( sal_Int32 nSet ) { m_nPageNumber = nSet; }
    void SetBreakType( sal_Int32 nSet ) { m_nBreakType = nSet; }
    void SetPaperBin( sal_Int32 nSet );
    void SetFirstPaperBin( sal_Int32 nSet );

    //determine which style gets the borders
    void ApplyBorderToPageStyles(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory,
            sal_Int32 nValue );

    void ApplyPropertiesToPageStyles(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory);
};

} //namespace dmapper
#endif
