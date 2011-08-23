/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/debug.hxx>
#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
#define _SVSTDARR_STRINGSSORTDTOR
#include <bf_svtools/svstdarr.hxx>
#endif

#include "xmlnmspe.hxx"

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>

#include "xmlaustp.hxx"
#include "xmlexp.hxx"
#include "XMLEventExport.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

using rtl::OUString;
//using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

using ::com::sun::star::document::XEventsSupplier;

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
          const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        const OUString* pPrefix )
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

    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sName );

    // style:family="..."
    if( rXMLFamily.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, rXMLFamily);

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
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME,
                                    sParent );

    // style:next-style-name="..." (paragraph styles only)
    if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
    {
        aAny = xPropSet->getPropertyValue( sFollowStyle );
        OUString sNextName;
        aAny >>= sNextName;
        if( sName != sNextName )
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
                          sNextName );
        }
    }

    // style:auto-update="..." (SW only)
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        aAny = xPropSet->getPropertyValue( sIsAutoUpdate );
        if( *(sal_Bool *)aAny.getValue() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_AUTO_UPDATE,
                                      XML_TRUE );
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
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_LIST_STYLE_NAME,
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
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_STYLE,
                                  sal_True, sal_True );
        // <style:properties>
        ::std::vector< XMLPropertyState > xPropStates =
            rPropMapper->Filter( xPropSet );
        rPropMapper->exportXML( GetExport(), xPropStates,
                                XML_EXPORT_FLAG_IGN_WS );
        exportStyleContent( rStyle );

        // <script:events>, if they are supported by this style
        Reference<XEventsSupplier> xEventsSupp(rStyle, UNO_QUERY);
        GetExport().GetEventExport().Export(xEventsSupp);
    }
    return sal_True;
}

sal_Bool XMLStyleExport::exportDefaultStyle(
        const Reference< XPropertySet >& xPropSet,
          const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper )
{
    Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();

    Any aAny;

    // <style:default-style ...>
    GetExport().CheckAttrList();

    {
        // style:family="..."
        if( rXMLFamily.getLength() > 0 )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY,
                                      rXMLFamily );
        // <style:style>
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                  XML_DEFAULT_STYLE,
                                  sal_True, sal_True );
        // <style:properties>
        //::std::vector< XMLPropertyState > xPropStates =
        //	rPropMapper->FilterDefaults( xPropSet );
        ::std::vector< XMLPropertyState > xPropStates =
            rPropMapper->FilterDefaults( xPropSet );
        rPropMapper->exportXML( GetExport(), xPropStates,
                                     XML_EXPORT_FLAG_IGN_WS );
//		exportStyleContent( rStyle );
    }
    return sal_True;
}

#if 0
void XMLStyleExport::exportStyleFamily(
    const sal_Char *pFamily,
    const OUString& rXMLFamily,
    const UniReference < XMLPropertySetMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    const OUString sFamily(OUString::createFromAscii(pFamily ));
    UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
        new SvXMLExportPropertyMapper( rPropMapper );
    exportStyleFamily( sFamily, rXMLFamily, xExpPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const OUString& rFamily, const OUString& rXMLFamily,
    const UniReference < XMLPropertySetMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
        new SvXMLExportPropertyMapper( rPropMapper );
    exportStyleFamily( rFamily, rXMLFamily, xExpPropMapper, bUsed, nFamily,
                       pPrefix);
}
#endif

void XMLStyleExport::exportStyleFamily(
    const sal_Char *pFamily,
    const OUString& rXMLFamily,
    const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
    sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    const OUString sFamily(OUString::createFromAscii(pFamily ));
    exportStyleFamily( sFamily, rXMLFamily, rPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const OUString& rFamily, const OUString& rXMLFamily,
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
    sal_Int32 i;

    for( i=0; i < nStyles; i++ )
    {
        Reference< XStyle > xStyle;
        try
        {
            xStyles->getByIndex( i ) >>= xStyle;
        }
        catch( lang::IndexOutOfBoundsException )
        {
            // due to bugs in prior versions it is possible that
            // a binary file is missing some critical styles.
            // The only possible way to deal with this is to
            // not export them here and remain silent.
            continue;
        }

        DBG_ASSERT( xStyle.is(), "Style not found for export!" );
        if( xStyle.is() )
        {
            if( !bUsed || xStyle->isInUse() )
            {
                BOOL bExported = exportStyle( xStyle, rXMLFamily, rPropMapper,
                                              pPrefix );
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

                    if( xStyle.is() && exportStyle( xStyle, rXMLFamily, rPropMapper, pPrefix ) )
                        pExportedStyles->Insert( new String( sTmp ) );
                }
            }
        }
    }

    delete pExportedStyles;
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
