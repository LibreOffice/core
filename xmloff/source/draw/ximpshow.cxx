/*************************************************************************
 *
 *  $RCSfile: ximpshow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:35:56 $
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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XCUSTOMPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif

#include "xmlkywd.hxx"

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XIMPSHOW_HXX
#include "ximpshow.hxx"
#endif

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

///////////////////////////////////////////////////////////////////////

class ShowsImpImpl
{
public:
    Reference< XSingleServiceFactory > mxShowFactory;
    Reference< XNameContainer > mxShows;
    Reference< XPropertySet > mxPresProps;
    Reference< XNameAccess > mxPages;
    OUString maCustomShowName;
    SdXMLImport& mrImport;

    ShowsImpImpl( SdXMLImport& rImport )
    :   mrImport( rImport )
    {}
};

///////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLShowsContext, SvXMLImportContext );

SdXMLShowsContext::SdXMLShowsContext( SdXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    mpImpl = new ShowsImpImpl( rImport );

    Reference< XCustomPresentationSupplier > xShowsSupplier( rImport.GetModel(), UNO_QUERY );
    if( xShowsSupplier.is() )
    {
        mpImpl->mxShows = xShowsSupplier->getCustomPresentations();
        mpImpl->mxShowFactory = Reference< XSingleServiceFactory >::query( mpImpl->mxShows );
    }

    Reference< XDrawPagesSupplier > xDrawPagesSupplier( rImport.GetModel(), UNO_QUERY );
    if( xDrawPagesSupplier.is() )
        mpImpl->mxPages = Reference< XNameAccess >::query( xDrawPagesSupplier->getDrawPages() );

    Reference< XPresentationSupplier > xPresentationSupplier( rImport.GetModel(), UNO_QUERY );
    if( xPresentationSupplier.is() )
        mpImpl->mxPresProps = Reference< XPropertySet >::query( xPresentationSupplier->getPresentation() );


    if( mpImpl->mxPresProps.is() )
    {
        sal_Bool bAll = sal_True;
        uno::Any aAny;

        // read attributes
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_PRESENTATION:
                if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_start_page ) ) )
                {
                    aAny <<= sValue;
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ), aAny );
                    bAll = sal_False;
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_show ) ) )
                {
                    mpImpl->maCustomShowName = sValue;
                    bAll = sal_False;
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_pause ) ) )
                {
                    DateTime aTime;
                    if( !SvXMLUnitConverter::convertTime( aTime,  sValue ) )
                        continue;

                    const sal_Int32 nMS = ( ( aTime.Hours * 60 + aTime.Minutes ) * 60 + aTime.Seconds ) * 100 + aTime.HundredthSeconds;
                    aAny <<= nMS;
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Pause" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_animations ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_enabled ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowAnimations" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_stay_on_top ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_force_manual ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_endless ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_full_screen ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_mouse_visible ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_start_with_navigator ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_mouse_as_pen ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_transition_on_click ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_enabled ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTransitionOnClick" ) ), aAny );
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_show_logo ) ) )
                {
                    aAny = bool2any( sValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_true ) ) );
                    mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowLogo" ) ), aAny );
                }
            }
        }
        aAny = bool2any( bAll );
        mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowAll" ) ), aAny );
    }
}

SdXMLShowsContext::~SdXMLShowsContext()
{
    if( mpImpl && mpImpl->maCustomShowName.getLength() )
    {
        uno::Any aAny;
        aAny <<= mpImpl->maCustomShowName;
        mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ), aAny );
    }

    delete mpImpl;
}

SvXMLImportContext * SdXMLShowsContext::CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
    if( mpImpl && nPrefix == XML_NAMESPACE_PRESENTATION && rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_show ) ) )
    {
        OUString aName;
        OUString aPages;

        // read attributes
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_PRESENTATION:
                if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_name ) ) )
                {
                    aName = sValue;
                }
                else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_pages ) ) )
                {
                    aPages = sValue;
                }
            }
        }

        if( aName.getLength() != 0 && aPages.getLength() != 0 )
        {
            Reference< XIndexContainer > xShow( mpImpl->mxShowFactory->createInstance(), UNO_QUERY );
            if( xShow.is() )
            {
                SvXMLTokenEnumerator aPageNames( aPages, sal_Unicode(',') );
                OUString sPageName;
                Any aAny;

                while( aPageNames.getNextToken( sPageName ) )
                {
                    if( !mpImpl->mxPages->hasByName( sPageName ) )
                        continue;

                    Reference< XDrawPage > xPage;
                    mpImpl->mxPages->getByName( sPageName ) >>= xPage;
                    if( xPage.is() )
                    {
                        aAny <<= xPage;
                        xShow->insertByIndex( xShow->getCount(), aAny );
                    }
                }

                aAny <<= xShow;

                if( mpImpl->mxShows->hasByName( aName ) )
                {
                    mpImpl->mxShows->replaceByName( aName, aAny );
                }
                else
                {
                    mpImpl->mxShows->insertByName( aName, aAny );
                }
            }
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

