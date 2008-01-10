/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:41:29 $
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
        class XFootnote;
    }
    namespace table{
        struct BorderLine;
    }
}}}

namespace writerfilter {
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
struct PropertyDefinition
{
    PropertyIds eId;
    bool        bIsTextProperty;

    PropertyDefinition( PropertyIds _eId, bool _bIsTextProperty ) :
        eId( _eId ),
        bIsTextProperty( _bIsTextProperty ){}

    bool    operator== (const PropertyDefinition& rDef) const
            {   return rDef.eId == eId; }
    bool    operator< (const PropertyDefinition& rDef) const
            {   return eId < rDef.eId; }
};
typedef std::map < PropertyDefinition, ::com::sun::star::uno::Any > _PropertyMap;
class PropertyMap : public _PropertyMap
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aValues;
    //marks context as footnote context - ::text( ) events contain either the footnote character or can be ignored
    //depending on sprmCSymbol
    sal_Unicode                                                                 m_cFootnoteSymbol; // 0 == invalid
    sal_Int32                                                                   m_nFootnoteFontId; // negative values are invalid ids
    ::rtl::OUString                                                             m_sFootnoteFontName;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote >       m_xFootnote;

    public:
        PropertyMap();
        virtual ~PropertyMap();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropertyValues();
    /** Add property, usually overwrites already available attributes. It shouldn't overwrite in case of default attributes
     */
    void Insert( PropertyIds eId, bool bIsTextProperty, const ::com::sun::star::uno::Any& rAny, bool bOverwrite = true );
    using _PropertyMap::insert;
    void insert(const boost::shared_ptr<PropertyMap> pMap, bool bOverwrite = true);

    void Invalidate()
        {
            if(m_aValues.getLength())
                m_aValues.realloc( 0 );
        }

    const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote>&  GetFootnote() const;
    void SetFootnote( ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote> xF ) { m_xFootnote = xF; }

    sal_Unicode GetFootnoteSymbol() const { return m_cFootnoteSymbol;}
    void        SetFootnoteSymbol(sal_Unicode cSet) { m_cFootnoteSymbol = cSet;}

    sal_Int32   GetFootnoteFontId() const { return m_nFootnoteFontId;}
    void        SetFootnoteFontId(sal_Int32 nSet) { m_nFootnoteFontId = nSet;}

    const ::rtl::OUString&      GetFootnoteFontName() const { return m_sFootnoteFontName;}
    void                        SetFootnoteFontName( const ::rtl::OUString& rSet ) { m_sFootnoteFontName = rSet;}
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

/*-- 14.06.2007 12:12:34---------------------------------------------------
    property map of a stylesheet
  -----------------------------------------------------------------------*/
class StyleSheetPropertyMap : public PropertyMap

{
    //special table style properties
//    sal_Int32               mnCT_Spacing_after;
    sal_Int32               mnCT_Spacing_line;
    sal_Int32               mnCT_Spacing_lineRule;

    ::rtl::OUString         msCT_Fonts_ascii;
    bool                    mbCT_TrPrBase_tblHeader;
    sal_Int32               mnCT_TrPrBase_jc;
    sal_Int32               mnCT_TcPrBase_vAlign;

    sal_Int32               mnCT_TblWidth_w;
    sal_Int32               mnCT_TblWidth_type;

//    bool                    mbCT_Spacing_afterSet;
    bool                    mbCT_Spacing_lineSet;
    bool                    mbCT_Spacing_lineRuleSet;

    bool                    mbCT_TrPrBase_tblHeaderSet;
    bool                    mbCT_TrPrBase_jcSet;
    bool                    mbCT_TcPrBase_vAlignSet;

    bool                    mbCT_TblWidth_wSet;
    bool                    mbCT_TblWidth_typeSet;

    sal_Int32               mnListId;
    sal_Int16               mnListLevel;
public:
    explicit StyleSheetPropertyMap();
    ~StyleSheetPropertyMap();

//    void SetCT_Spacing_after(      sal_Int32 nSet )
//        {mnCT_Spacing_after = nSet;    mbCT_Spacing_afterSet = true;        }
    void SetCT_Spacing_line(       sal_Int32 nSet )
        {mnCT_Spacing_line = nSet;     mbCT_Spacing_lineSet = true;         }
    void SetCT_Spacing_lineRule(   sal_Int32  nSet )
        {mnCT_Spacing_lineRule = nSet; mbCT_Spacing_lineRuleSet = true;     }

    void SetCT_Fonts_ascii(  const ::rtl::OUString& rSet )
        {msCT_Fonts_ascii = rSet;          }
    void SetCT_TrPrBase_tblHeader( bool bSet )
        {mbCT_TrPrBase_tblHeader = bSet; mbCT_TrPrBase_tblHeaderSet = true; }
    void SetCT_TrPrBase_jc(        sal_Int32 nSet )
        {mnCT_TrPrBase_jc = nSet;        mbCT_TrPrBase_jcSet = true;     }
    void SetCT_TcPrBase_vAlign(    sal_Int32 nSet )
        {mnCT_TcPrBase_vAlign = nSet;    mbCT_TcPrBase_vAlignSet = true; }

    void SetCT_TblWidth_w( sal_Int32 nSet )
        { mnCT_TblWidth_w = nSet;    mbCT_TblWidth_wSet = true; }
    void SetCT_TblWidth_type( sal_Int32 nSet )
        {mnCT_TblWidth_type = nSet;    mbCT_TblWidth_typeSet = true; }

//    bool GetCT_Spacing_after(   sal_Int32& rToFill) const
//    {
//        if( mbCT_Spacing_afterSet )
//            rToFill = mnCT_Spacing_after;
//        return mbCT_Spacing_afterSet;
//    }
    bool GetCT_Spacing_line(    sal_Int32& rToFill) const
    {
        if( mbCT_Spacing_lineSet )
            rToFill = mnCT_Spacing_line;
        return mbCT_Spacing_lineSet;
    }
    bool GetCT_Spacing_lineRule(sal_Int32& rToFill) const
    {
        if( mbCT_Spacing_lineRuleSet )
            rToFill = mnCT_Spacing_lineRule;
        return mbCT_Spacing_lineRuleSet;
    }

    bool GetCT_Fonts_ascii(::rtl::OUString& rToFill) const
    {
        if( msCT_Fonts_ascii.getLength() > 0 )
            rToFill = msCT_Fonts_ascii;
        return msCT_Fonts_ascii.getLength() > 0;
    }
    bool GetCT_TrPrBase_tblHeader(bool& rToFill) const
    {
        if( mbCT_TrPrBase_tblHeaderSet )
            rToFill = mbCT_TrPrBase_tblHeader;
        return mbCT_TrPrBase_tblHeaderSet;
    }
    bool GetCT_TrPrBase_jc(     sal_Int32& rToFill)const
    {
        if( mbCT_TrPrBase_jcSet )
            rToFill = mnCT_TrPrBase_jc;
        return mbCT_TrPrBase_jcSet;
    }
    bool GetCT_TcPrBase_vAlign( sal_Int32& rToFill)const
    {
        if( mbCT_TcPrBase_vAlignSet )
            rToFill = mnCT_TcPrBase_vAlign;
        return mbCT_TcPrBase_vAlignSet;
    }
    sal_Int32   GetListId() const               { return mnListId; }
    void        SetListId(sal_Int32 nId)        { mnListId = nId; }

    sal_Int16   GetListLevel() const            { return mnListLevel; }
    void        SetListLevel(sal_Int16 nLevel)  { mnListLevel = nLevel; }

};
} //namespace dmapper
} //namespace writerfilter
#endif
