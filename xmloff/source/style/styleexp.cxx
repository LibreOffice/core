/*************************************************************************
 *
 *  $RCSfile: styleexp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-08 12:35:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLITMAP_HXX
//#include "xmlitmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif
#ifndef _XMLOFF_STYLEEXP_HXX
#include "styleexp.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::text;

XMLStyleExport::XMLStyleExport(
        SvXMLExport& rExp,
        const ::rtl::OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP ) :
    rExport( rExp ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) ),
    sNumberingStyleName( RTL_CONSTASCII_USTRINGPARAM( "NumberingStyleName" ) ),
    sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) ),
    sPoolStyleName( rPoolStyleName ),
    pAutoStylePool( pAutoStyleP  )
{
}

XMLStyleExport::~XMLStyleExport()
{
}

void XMLStyleExport::exportStyleAttributes( const Reference< XStyle >& rStyle )
{
}

void XMLStyleExport::exportStyleContent( const Reference< XStyle >& rStyle )
{
}

sal_Bool XMLStyleExport::exportStyle(
        const Reference< XStyle >& rStyle,
          const sal_Char *pXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        const OUString* pPrefix)
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*(sal_Bool *)aAny.getValue() )
            return sal_False;
    }

    // <style:style ...>
    GetExport().CheckAttrList();

    // style:name="..."
    OUString sName;

    if(pPrefix)
        sName = *pPrefix;
    sName += rStyle->getName();

    GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_name, sName );

    // style:family="..."
    if( pXMLFamily )
        GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_family,
                                         pXMLFamily );

    // style:parent-style-name="..."
    OUString sParentString(rStyle->getParentStyle());
    OUString sParent;

    if(sParentString.getLength())
    {
        if(pPrefix)
            sParent = *pPrefix;
        sParent += sParentString;
    }
    else
        sParent = sPoolStyleName;

    if( sParent.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_parent_style_name,
                                    sParent );

    // style:next-style-name="..." (paragraph styles only)
    if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
    {
        aAny = xPropSet->getPropertyValue( sFollowStyle );
        OUString sNextName;
        aAny >>= sNextName;
        if( sName != sNextName )
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_next_style_name,
                          sNextName );
        }
    }

    // style:auto-update="..." (SW only)
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        aAny = xPropSet->getPropertyValue( sIsAutoUpdate );
        if( *(sal_Bool *)aAny.getValue() )
            GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_auto_update,
                               sXML_true );
    }

    // style:list-style-name="..." (SW paragarph styles only)
    if( xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( sNumberingStyleName  ) )
        {
            aAny = xPropSet->getPropertyValue( sNumberingStyleName );
            if( aAny.hasValue() )
            {
                OUString sListName;
                aAny >>= sListName;
                if( sListName.getLength() )
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_list_style_name,
                                  sListName );
            }
        }
    }

    // style:pool-id="..." is not required any longer since we use
    // english style names only
    exportStyleAttributes( rStyle );

    // TODO: style:help-file-name="..." and style:help-id="..." can neither
    // be modified by UI nor by API and that for, have not to be exported
    // currently.

    {
        // <style:style>
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, sXML_style,
                                  sal_True, sal_True );
        // <style:properties>
        ::std::vector< XMLPropertyState > xPropStates =
            rPropMapper->Filter( xPropSet );
        rPropMapper->exportXML( GetExport().GetDocHandler(), xPropStates,
                                  GetExport().GetMM100UnitConverter(),
                                     GetExport().GetNamespaceMap(),
                                     XML_EXPORT_FLAG_IGN_WS );
        exportStyleContent( rStyle );
    }
    return sal_True;
}

void XMLStyleExport::exportStyleFamily(
    const sal_Char *pFamily,
    const sal_Char *pXMLFamily,
    const UniReference < XMLPropertySetMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    const OUString sFamily(OUString::createFromAscii(pFamily ));
    UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
        new SvXMLExportPropertyMapper( rPropMapper );
    exportStyleFamily( sFamily, pXMLFamily, xExpPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const OUString& rFamily, const sal_Char *pXMLFamily,
    const UniReference < XMLPropertySetMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
        new SvXMLExportPropertyMapper( rPropMapper );
    exportStyleFamily( rFamily, pXMLFamily, xExpPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const sal_Char *pFamily,
    const sal_Char *pXMLFamily,
    const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    const OUString sFamily(OUString::createFromAscii(pFamily ));
    exportStyleFamily( sFamily, pXMLFamily, rPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const OUString& rFamily, const sal_Char *pXMLFamily,
    const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    DBG_ASSERT( GetExport().GetModel().is(), "There is the model?" );
    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(),
                                                       UNO_QUERY );
    Reference< XNameAccess > xStyleCont;

    Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
    if( xFamilies->hasByName( rFamily ) )
        xFamilies->getByName( rFamily ) >>= xStyleCont;

    if( !xStyleCont.is() )
        return;

    Reference< XIndexAccess > xStyles( xStyleCont, UNO_QUERY );
    const sal_Int32 nStyles = xStyles->getCount();

    // If next styles are supported and used styles should be exported only,
    // the next style may be unused but has to be exported, too. In this case
    // the names of all exported styles are remembered.
    SvStringsSortDtor *pExportedStyles = 0;
    sal_Bool bFirstStyle = sal_True;

    for( sal_Int32 i=0; i < nStyles; i++ )
    {
        Reference< XStyle > xStyle;
        xStyles->getByIndex( i ) >>= xStyle;

        DBG_ASSERT( xStyle.is(), "Style not found for export!" );
        if( xStyle.is() )
        {
            if( !bUsed || xStyle->isInUse() )
            {
                BOOL bExported = exportStyle( xStyle, pXMLFamily, rPropMapper, pPrefix );
                if( bUsed && bFirstStyle && bExported  )
                {
                    // If this is the first style, find out wether next styles
                    // are supported.
                    Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
                    Reference< XPropertySetInfo > xPropSetInfo =
                        xPropSet->getPropertySetInfo();

                    if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
                        pExportedStyles = new SvStringsSortDtor;
                    bFirstStyle = sal_False;
                }

                if( pExportedStyles && bExported )
                {
                    // If next styles are supported, remember this style's name.
                    String *pTmp = new String( xStyle->getName() );
                    if( !pExportedStyles->Insert( pTmp ) )
                        delete pTmp;
                }
            }

            // if an auto style pool is given, remember this style's name as a
            // style name that must not be used by automatic styles.
            if( pAutoStylePool )
                pAutoStylePool->RegisterName( nFamily, xStyle->getName() );
        }
    }

    if( pExportedStyles )
    {
        // if next styles are supported, export all next styles that are
        // unused and that for, haven't been exported in the first loop.
        for( i=0; i < nStyles; i++ )
        {
            Reference< XStyle > xStyle;
            xStyles->getByIndex( i ) >>= xStyle;

            DBG_ASSERT( xStyle.is(), "Style not found for export!" );
            if( xStyle.is() )
            {
                Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
                Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

                // styles that aren't existing realy are ignored.
                if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
                {
                    Any aAny( xPropSet->getPropertyValue( sIsPhysical ) );
                    if( !*(sal_Bool *)aAny.getValue() )
                        continue;
                }

                if( !xStyle->isInUse() )
                    continue;

                if( !xPropSetInfo->hasPropertyByName( sFollowStyle ) )
                {
                    DBG_ASSERT( 0==sFollowStyle.getLength(),
                                "no follow style???" );
                    continue;
                }

                OUString sNextName;
                xPropSet->getPropertyValue( sFollowStyle ) >>= sNextName;
                String sTmp( sNextName );
                // if the next style hasn't been exported by now, export it now
                // and remember its name.
                if( xStyle->getName() != sNextName &&
                    !pExportedStyles->Seek_Entry( &sTmp ) )
                {
                    xStyleCont->getByName( sNextName ) >>= xStyle;
                    DBG_ASSERT( xStyle.is(), "Style not found for export!" );

                    if( xStyle.is() && exportStyle( xStyle, pXMLFamily, rPropMapper, pPrefix ) )
                        pExportedStyles->Insert( new String( sTmp ) );
                }
            }
        }
    }

    delete pExportedStyles;
}


