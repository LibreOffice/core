/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtstyle.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:11:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _TOOLS_DEBUG_HXX
//#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHSTYLECATEGORY_HPP_
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif


#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
//#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_XMLSMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
//#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_TXTPARAE_HXX
#include <xmloff/txtparae.hxx>
#endif
#ifndef _XMLOFF_XMLNUME_HXX
#include <xmloff/xmlnume.hxx>
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif
#ifndef _XMLOFF_XMLSECTIONEXPORT_HXX
#include "XMLSectionExport.hxx"
#endif
#ifndef _XMLOFF_XMLLINENUMBERINGEXPORT_HXX_
#include "XMLLineNumberingExport.hxx"
#endif
#ifndef _XMLOFF_TXTEXPPR_HXX
#include "txtexppr.hxx"
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif

#ifndef _XMLOFF_STYLEEXP_HXX
//#include <xmloff/styleexp.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

void XMLTextParagraphExport::exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle )
{
    OUString sName;
    Any aAny;
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo(
            xPropSet->getPropertySetInfo());
    if( xPropSetInfo->hasPropertyByName( sCategory ) )
    {
        sal_Int16 nCategory = 0;
        xPropSet->getPropertyValue( sCategory ) >>= nCategory;
        enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
        if( -1 != nCategory )
        {
            switch( nCategory )
            {
            case ParagraphStyleCategory::TEXT:
                eValue = XML_TEXT;
                break;
            case ParagraphStyleCategory::CHAPTER:
                eValue = XML_CHAPTER;
                break;
            case ParagraphStyleCategory::LIST:
                eValue = XML_LIST;
                break;
            case ParagraphStyleCategory::INDEX:
                eValue = XML_INDEX;
                break;
            case ParagraphStyleCategory::EXTRA:
                eValue = XML_EXTRA;
                break;
            case ParagraphStyleCategory::HTML:
                eValue = XML_HTML;
                break;
            }
        }
        if( eValue != XML_TOKEN_INVALID )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_CLASS, eValue);
    }
    if( xPropSetInfo->hasPropertyByName( sPageDescName ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( sPageDescName  ) )
        {
            xPropSet->getPropertyValue( sPageDescName ) >>= sName;
            // fix for #i5551#  if( sName.getLength() > 0 )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_MASTER_PAGE_NAME,
                                          GetExport().EncodeStyleName( sName ) );
        }
    }
    sal_Int32 nOutlineLevel =
        GetExport().GetTextParagraphExport()->GetHeadingLevel( rStyle->getName() );
    if( nOutlineLevel != -1 )
    {
        OUStringBuffer sTmp;
                    sTmp.append( static_cast<sal_Int32>(nOutlineLevel+1L) );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                            XML_DEFAULT_OUTLINE_LEVEL,
                            sTmp.makeStringAndClear() );
    }

    if( bProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
            pProgress->SetValue( pProgress->GetValue()+2 );
    }
}

void XMLTextParagraphExport::exportNumStyles( sal_Bool bUsed )
{
    SvxXMLNumRuleExport aNumRuleExport( GetExport() );
    aNumRuleExport.exportStyles( bUsed, pListAutoPool, !IsBlockMode() );
}

void XMLTextParagraphExport::exportTextStyles( sal_Bool bUsed, sal_Bool bProg )
{
    sal_Bool bOldProg = bProgress;
    bProgress = bProg;

    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        OUString sTextDefaults ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.Defaults" ) );
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( sTextDefaults ), UNO_QUERY);
        if (xPropSet.is())
        {
            exportDefaultStyle( xPropSet, GetXMLToken(XML_PARAGRAPH), GetParaPropMapper());

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TEXT_PROP_MAP_TABLE_DEFAULTS ),
                    GetExport() ) );

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE_ROW),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TEXT_PROP_MAP_TABLE_ROW_DEFAULTS ),
                    GetExport() ) );
        }
    }
    exportStyleFamily( "ParagraphStyles", GetXMLToken(XML_PARAGRAPH), GetParaPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_PARAGRAPH, 0);
    exportStyleFamily( "CharacterStyles", GetXMLToken(XML_TEXT), GetTextPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_TEXT );
    // get shape export to make sure the the frame family is added correctly.
    GetExport().GetShapeExport();
    exportStyleFamily( "FrameStyles", OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)), GetFramePropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_FRAME, 0);
    exportNumStyles( bUsed );
    if( !IsBlockMode() )
    {
        exportTextFootnoteConfiguration();
        XMLSectionExport::ExportBibliographyConfiguration(GetExport());
        XMLLineNumberingExport aLineNumberingExport(GetExport());
        aLineNumberingExport.Export();
    }

    bProgress = bOldProg;
}
