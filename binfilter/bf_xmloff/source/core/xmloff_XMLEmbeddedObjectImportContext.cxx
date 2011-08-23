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

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_ 
#include <com/sun/star/frame/XStorable.hpp>
#endif

// #110680#
//#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
//#include <comphelper/processfactory.hxx>
//#endif

#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _XMLOFF_XMLFILTERSERVICENAMES_H
#include "XMLFilterServiceNames.h"
#endif
#ifndef _XMLOFF_XMLEMBEDDEDOBJECTIMPORTCONTEXT_HXX
#include "XMLEmbeddedObjectImportContext.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::xml::sax;
using namespace ::binfilter::xmloff::token;

struct XMLServiceMapEntry_Impl
{
    enum XMLTokenEnum eClass;
    const sal_Char *sFilterService;
    sal_Int32	   nFilterServiceLen;
};

#define SERVICE_MAP_ENTRY( cls, app ) \
    { XML_##cls, \
      XML_IMPORT_FILTER_##app, sizeof(XML_IMPORT_FILTER_##app)-1 }

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( TEXT, WRITER ),
    SERVICE_MAP_ENTRY( ONLINE_TEXT, WRITER ),
    SERVICE_MAP_ENTRY( SPREADSHEET, CALC ),
    SERVICE_MAP_ENTRY( DRAWING, DRAW ),
    SERVICE_MAP_ENTRY( PRESENTATION, IMPRESS ),
    SERVICE_MAP_ENTRY( CHART, CHART ),
    { XML_TOKEN_INVALID, 0, 0 }
};

class XMLEmbeddedObjectImportContext_Impl : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xHandler;

public:
    TYPEINFO();

    XMLEmbeddedObjectImportContext_Impl( SvXMLImport& rImport, USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler >& rHandler );

    virtual ~XMLEmbeddedObjectImportContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );
};

TYPEINIT1( XMLEmbeddedObjectImportContext_Impl, SvXMLImportContext );

XMLEmbeddedObjectImportContext_Impl::XMLEmbeddedObjectImportContext_Impl(
        SvXMLImport& rImport, USHORT nPrfx,
        const OUString& rLName,
        const Reference< XDocumentHandler >& rHandler ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xHandler( rHandler )
{
}

XMLEmbeddedObjectImportContext_Impl::~XMLEmbeddedObjectImportContext_Impl()
{
}

SvXMLImportContext *XMLEmbeddedObjectImportContext_Impl::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList )
{
    return new XMLEmbeddedObjectImportContext_Impl( GetImport(),
                                                    nPrefix, rLocalName,
                                                    xHandler );
}

void XMLEmbeddedObjectImportContext_Impl::StartElement(
        const Reference< XAttributeList >& xAttrList )
{
    xHandler->startElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                GetPrefix(), GetLocalName() ),
                            xAttrList );
}

void XMLEmbeddedObjectImportContext_Impl::EndElement()
{
    xHandler->endElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                GetPrefix(), GetLocalName() ) );
}

void XMLEmbeddedObjectImportContext_Impl::Characters( const OUString& rChars )
{
    xHandler->characters( rChars );
}

//-----------------------------------------------------------------------------

TYPEINIT1( XMLEmbeddedObjectImportContext, SvXMLImportContext );

sal_Bool XMLEmbeddedObjectImportContext::SetComponent(
        Reference< XComponent >& rComp )
{
    if( !rComp.is() || !sFilterService.getLength() )
        return sal_False;


    Sequence<Any> aArgs( 0 );

    // #110680#
    // Reference< XMultiServiceFactory > xServiceFactory = comphelper::getProcessServiceFactory();
    Reference< XMultiServiceFactory > xServiceFactory = GetImport().getServiceFactory();

    xHandler = Reference < XDocumentHandler >(
        xServiceFactory->createInstanceWithArguments( sFilterService, aArgs),
                                               UNO_QUERY);

    if( !xHandler.is() )
        return sal_False;

    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument( rComp );

    xComp = rComp;	// keep ref to component only if there is a handler
    return sal_True;
}

XMLEmbeddedObjectImportContext::XMLEmbeddedObjectImportContext(
        SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName,
        const Reference< XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    // #i55761#
    bNeedToUnlockControllers(false)
{
    SvGlobalName aName;

    if( nPrfx == XML_NAMESPACE_MATH &&
        IsXMLToken( rLName, XML_MATH ) )
    {
        sFilterService = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_IMPORT_FILTER_MATH) );
        aName = SvGlobalName(SO3_SM_CLASSID);
    }
    else if( nPrfx == XML_NAMESPACE_OFFICE &&
        IsXMLToken( rLName, XML_DOCUMENT ) )
    {
        OUString sClass;

        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,	&aLocalName );
            if( nPrefix == XML_NAMESPACE_OFFICE &&
                IsXMLToken( aLocalName, XML_CLASS ) )
            {
                sClass = xAttrList->getValueByIndex( i );
                break;
            }
        }

        if( sClass.getLength() )
        {
            const XMLServiceMapEntry_Impl *pEntry = aServiceMap;
            while( pEntry->eClass != XML_TOKEN_INVALID )
            {
                if( IsXMLToken( sClass, pEntry->eClass ) )
                {
                    sFilterService = OUString( pEntry->sFilterService,
                                               pEntry->nFilterServiceLen,
                                               RTL_TEXTENCODING_ASCII_US );

                    switch( pEntry->eClass )
                    {
                    case XML_TEXT:			aName = SvGlobalName(SO3_SW_CLASSID); break;
                    case XML_ONLINE_TEXT:	aName = SvGlobalName(SO3_SWWEB_CLASSID); break;
                    case XML_SPREADSHEET:	aName = SvGlobalName(SO3_SC_CLASSID); break;
                    case XML_DRAWING:		aName = SvGlobalName(SO3_SDRAW_CLASSID); break;
                    case XML_PRESENTATION:	aName = SvGlobalName(SO3_SIMPRESS_CLASSID); break;
                    case XML_CHART:			aName = SvGlobalName(SO3_SCH_CLASSID); break;
                    }

                    break;
                }
                pEntry++;
            }
        }
    }

    sCLSID = aName.GetHexName();
}

XMLEmbeddedObjectImportContext::~XMLEmbeddedObjectImportContext()
{
}

SvXMLImportContext *XMLEmbeddedObjectImportContext::CreateChildContext(
        USHORT nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList )
{
    if( xHandler.is() )
        return new XMLEmbeddedObjectImportContext_Impl( GetImport(),
                                                        nPrefix, rLocalName,
                                                        xHandler );
    else
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void XMLEmbeddedObjectImportContext::StartElement(
        const Reference< XAttributeList >& xAttrList )
{
    if( xHandler.is() )
    {
        // #i55761#
        Reference< XModel > xModel( xComp, UNO_QUERY );
        if( xModel.is() && !xModel->hasControllersLocked())
        {
            xModel->lockControllers();
            bNeedToUnlockControllers = true;
        }

        xHandler->startDocument();
        xHandler->startElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                    GetPrefix(), GetLocalName() ),
                                xAttrList );
    }
}

void XMLEmbeddedObjectImportContext::EndElement()
{
    if( xHandler.is() )
    {
        xHandler->endElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                    GetPrefix(), GetLocalName() ) );
        xHandler->endDocument();

        // #i55761#
        if(bNeedToUnlockControllers)
        {
            Reference< XModel > xModel( xComp, UNO_QUERY );

            if(xModel.is())
            {
                xModel->unlockControllers();
            }
        }

        // Save the object. That's required because the object should not be 
        // modified (it has been loaded just now). Setting it to unmodified
        // only does not work, because it is then assumed that it has been
        // stored.
        Reference < XStorable > xStorable( xComp, UNO_QUERY );
        if( xStorable.is() )
        {
            try
            {
                xStorable->store();
            }
            catch( ::com::sun::star::beans::PropertyVetoException& e )
            {
                Sequence<OUString> aSeq( 0 );
                GetImport().SetError( XMLERROR_FLAG_WARNING | 
                                  XMLERROR_API,
                                  aSeq );
            }
        }
#if 0
        // reset modifies state for the object since it has been imported
        // completly and therfor hasn't been modified.
        Reference < XModifiable > xModifiable( xComp, UNO_QUERY );
        if( xModifiable.is() )
        {
            try
            {
                xModifiable->setModified( sal_False );
            }
            catch( ::com::sun::star::beans::PropertyVetoException& e )
            {
                Sequence<OUString> aSeq( 0 );
                GetImport().SetError( XMLERROR_FLAG_WARNING | 
                                  XMLERROR_API,
                                  aSeq );
            }
        }
#endif

    }
}

void XMLEmbeddedObjectImportContext::Characters( const ::rtl::OUString& rChars )
{
    if( xHandler.is() )
        xHandler->characters( rChars );
}
    
}//end of namespace binfilter
