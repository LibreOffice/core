/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2007-02-21 12:25:07 $
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
    namespace container{
        class XNameAccess;
        class XNameContainer;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace text{
        class XTextRange;
        class XTextColumns;
    };
    namespace table{
        struct BorderLine;
    }
}}}

namespace dmapper{
class DomainMapper_Impl;
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
    void Insert( PropertyIds eId, const ::com::sun::star::uno::Any& rAny );

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
    //--> debug
    sal_Int32 nSectionNumber;
    //<-- debug
    //'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported

    bool                                                                        m_bIsFirstSection;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >      m_xStartingRange;

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

    bool                                    m_bPageNoRestart;
    sal_Int32                               m_nPageNumber;
    sal_Int32                               m_nBreakType;
    sal_Int32                               m_nPaperBin;
    sal_Int32                               m_nFirstPaperBin;

    sal_Int32                               m_nLeftMargin;
    sal_Int32                               m_nRightMargin;
    sal_Int32                               m_nTopMargin;
    sal_Int32                               m_nBottomMargin;
    sal_Int32                               m_nHeaderTop;
    sal_Int32                               m_nHeaderBottom;

    sal_Int32                               m_nDzaGutter;
    bool                                    m_bGutterRTL;
    bool                                    m_bSFBiDi;

    sal_Int32                               m_nGridLinePitch;
    sal_Int32                               m_nDxtCharSpace;

    //line numbering
    sal_Int32                               m_nLnnMod;
    sal_Int32                               m_nLnc;
    sal_Int32                               m_ndxaLnn;
    sal_Int32                               m_nLnnMin;

    void _ApplyProperties( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xStyle );
    ::com::sun::star::uno::Reference< com::sun::star::text::XTextColumns > ApplyColumnProperties(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xFollowPageStyle );
    void PrepareHeaderFooterProperties( bool bFirstPage );
    bool HasHeader( bool bFirstPage ) const;
    bool HasFooter( bool bFirstPage ) const;

public:
        explicit SectionPropertyMap(bool bIsFirstSection);
        ~SectionPropertyMap();

    enum PageType
    {
        PAGE_FIRST,
        PAGE_LEFT,
        PAGE_RIGHT
    };

    void SetStart( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange )
    {
        m_xStartingRange = xRange;
    }

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
    void SetPageNoRestart( bool bSet ) { m_bPageNoRestart = bSet; }
    void SetPageNumber( sal_Int32 nSet ) { m_nPageNumber = nSet; }
    void SetBreakType( sal_Int32 nSet ) { m_nBreakType = nSet; }
    void SetPaperBin( sal_Int32 nSet );
    void SetFirstPaperBin( sal_Int32 nSet );

    void SetLeftMargin(    sal_Int32 nSet ) { m_nLeftMargin = nSet; }
    void SetRightMargin( sal_Int32 nSet ) { m_nRightMargin = nSet; }
    void SetTopMargin(    sal_Int32 nSet ) { m_nTopMargin = nSet; }
    void SetBottomMargin( sal_Int32 nSet ) { m_nBottomMargin = nSet; }
    void SetHeaderTop(    sal_Int32 nSet ) { m_nHeaderTop = nSet; }
    void SetHeaderBottom( sal_Int32 nSet ) { m_nHeaderBottom = nSet; }

    void SetGutterRTL( bool bSet ) { m_bGutterRTL = bSet;}
    void SetDzaGutter( sal_Int32 nSet ) {m_nDzaGutter = nSet; }
    void SetSFBiDi( bool bSet ) { m_bSFBiDi = bSet;}

    void SetGridLinePitch( sal_Int32 nSet ) { m_nGridLinePitch = nSet; }
    void SetDxtCharSpace( sal_Int32 nSet ) { m_nDxtCharSpace = nSet; }

    void SetLnnMod( sal_Int32 nValue ) { m_nLnnMod = nValue; }
    void SetLnc(    sal_Int32 nValue ) { m_nLnc    = nValue; }
    void SetdxaLnn( sal_Int32 nValue ) { m_ndxaLnn  = nValue; }
    void SetLnnMin( sal_Int32 nValue ) { m_nLnnMin = nValue; }

    //determine which style gets the borders
    void ApplyBorderToPageStyles(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory,
            sal_Int32 nValue );

    void CloseSectionGroup( DomainMapper_Impl& rDM_Impl );
};

} //namespace dmapper
#endif
