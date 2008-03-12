/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextShapeStyleContext.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:06:49 $
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
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTPROPERTYSETCONTEXT_HXX
#include "XMLTextPropertySetContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include <xmloff/XMLEventsImportContext.hxx>
#endif
#ifndef _XMLOFF_XMLSHAPEPROPERTYSETCONTEXT_HXX
#include "XMLShapePropertySetContext.hxx"
#endif
#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#include "XMLTextColumnsContext.hxx"
#endif
#ifndef _XMLBACKGROUNDIMAGECONTEXT_HXX
#include "XMLBackgroundImageContext.hxx"
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif

#ifndef _XMLOFF_XMLTEXTSHAPESTYLECONTEXT_HXX
#include <xmloff/XMLTextShapeStyleContext.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

class XMLTextShapePropertySetContext_Impl : public XMLShapePropertySetContext
{
public:
    XMLTextShapePropertySetContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< XAttributeList >& xAttrList,
                 sal_uInt32 nFamily,
        ::std::vector< XMLPropertyState > &rProps,
        const UniReference < SvXMLImportPropertyMapper > &rMap );

    virtual ~XMLTextShapePropertySetContext_Impl();

    using SvXMLPropertySetContext::CreateChildContext;
    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList,
        ::std::vector< XMLPropertyState > &rProperties,
        const XMLPropertyState& rProp);
};

XMLTextShapePropertySetContext_Impl::XMLTextShapePropertySetContext_Impl(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< XAttributeList > & xAttrList,
                 sal_uInt32 nFamily,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap ) :
    XMLShapePropertySetContext( rImport, nPrfx, rLName, xAttrList, nFamily,
                                rProps, rMap )
{
}

XMLTextShapePropertySetContext_Impl::~XMLTextShapePropertySetContext_Impl()
{
}

SvXMLImportContext *XMLTextShapePropertySetContext_Impl::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( mxMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_TEXTCOLUMNS:
        pContext = new XMLTextColumnsContext( GetImport(), nPrefix,
                                                   rLocalName, xAttrList, rProp,
                                                   rProperties );
        break;

    case CTF_BACKGROUND_URL:
        DBG_ASSERT( rProp.mnIndex >= 3 &&
                    CTF_BACKGROUND_TRANSPARENCY ==
                        mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-3 ) &&
                    CTF_BACKGROUND_POS  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    CTF_BACKGROUND_FILTER  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");
        pContext =
            new XMLBackgroundImageContext( GetImport(), nPrefix,
                                           rLocalName, xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           rProp.mnIndex-3,
                                           rProperties );
        break;
    }

    if( !pContext )
        pContext = XMLShapePropertySetContext::CreateChildContext(
                        nPrefix, rLocalName, xAttrList, rProperties, rProp );

    return pContext;
}

//-----------------------------------------------------------------------------

void XMLTextShapeStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey &&
        IsXMLToken( rLocalName, XML_AUTO_UPDATE ) )
    {
          if( IsXMLToken( rValue, XML_TRUE ) )
            bAutoUpdate = sal_True;
    }
    else
    {
        XMLShapeStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( XMLTextShapeStyleContext, XMLShapeStyleContext );

XMLTextShapeStyleContext::XMLTextShapeStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        sal_Bool /*bDefaultStyle*/ ) :
    XMLShapeStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles,
                          nFamily ),
    sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) ),
    bAutoUpdate( sal_False )
{
}

XMLTextShapeStyleContext::~XMLTextShapeStyleContext()
{
}

SvXMLImportContext *XMLTextShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        if( nFamily )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
            {
                pContext = new XMLTextShapePropertySetContext_Impl(
                        GetImport(), nPrefix, rLocalName, xAttrList, nFamily,
                        GetProperties(), xImpPrMap );
            }
        }
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        // create and remember events import context
        // (for delayed processing of events)
        pContext = new XMLEventsImportContext( GetImport(), nPrefix,
                                                   rLocalName);
        xEventContext = pContext;
    }

    if( !pContext )
        pContext = XMLShapeStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLTextShapeStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    XMLShapeStyleContext::CreateAndInsert( bOverwrite );
    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() || !(bOverwrite || IsNew()) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        Any aAny;
        sal_Bool bTmp = bAutoUpdate;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsAutoUpdate, aAny );
    }

    // tell the style about it's events (if applicable)
    if( xEventContext.Is() )
    {
        // set event suppplier and release reference to context
        Reference<XEventsSupplier> xEventsSupplier(xStyle, UNO_QUERY);
        ((XMLEventsImportContext *)&xEventContext)->SetEvents(xEventsSupplier);
        xEventContext = 0;
    }
}


void XMLTextShapeStyleContext::Finish( sal_Bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );
}
