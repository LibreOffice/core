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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <tools/debug.hxx>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <comphelper/extract.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "layerimp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#include "XMLStringBufferImportContext.hxx"

using namespace ::std;
using namespace ::cppu;
using namespace ::xmloff::token;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using ::xmloff::token::IsXMLToken;

class SdXMLLayerContext : public SvXMLImportContext
{
public:
    SdXMLLayerContext( SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager );
    virtual ~SdXMLLayerContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const Reference< XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > mxLayerManager;
    ::rtl::OUString msName;
    ::rtl::OUStringBuffer sDescriptionBuffer;
    ::rtl::OUStringBuffer sTitleBuffer;
};

SdXMLLayerContext::SdXMLLayerContext( SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager )
: SvXMLImportContext(rImport, nPrefix, rLocalName)
, mxLayerManager( xLayerManager )
{
    const OUString strName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );

    OUString aName;

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString aLocalName;
        if( GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( i ), &aLocalName ) == XML_NAMESPACE_DRAW )
        {
            const OUString sValue( xAttrList->getValueByIndex( i ) );

            if( IsXMLToken( aLocalName, XML_NAME ) )
            {
                msName = sValue;
                break; // no more attributes needed
            }
        }
    }

}

SdXMLLayerContext::~SdXMLLayerContext()
{
}

SvXMLImportContext * SdXMLLayerContext::CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const Reference< XAttributeList >& )
{
    if( (XML_NAMESPACE_SVG == nPrefix) && IsXMLToken(rLocalName, XML_TITLE) )
    {
        return new XMLStringBufferImportContext( GetImport(), nPrefix, rLocalName, sTitleBuffer);
    }
    else if( (XML_NAMESPACE_SVG == nPrefix) && IsXMLToken(rLocalName, XML_DESC) )
    {
        return new XMLStringBufferImportContext( GetImport(), nPrefix, rLocalName, sDescriptionBuffer);
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}

void SdXMLLayerContext::EndElement()
{
    DBG_ASSERT( msName.getLength(), "xmloff::SdXMLLayerContext::EndElement(), draw:layer element without draw:name!" );
    if( msName.getLength() ) try
    {
        Reference< XPropertySet > xLayer;

        if( mxLayerManager->hasByName( msName ) )
        {
            mxLayerManager->getByName( msName ) >>= xLayer;
            DBG_ASSERT( xLayer.is(), "xmloff::SdXMLLayerContext::EndElement(), failed to get existing XLayer!" );
        }
        else
        {
            Reference< XLayerManager > xLayerManager( mxLayerManager, UNO_QUERY );
            if( xLayerManager.is() )
                xLayer = Reference< XPropertySet >::query( xLayerManager->insertNewByIndex( xLayerManager->getCount() ) );
            DBG_ASSERT( xLayer.is(), "xmloff::SdXMLLayerContext::EndElement(), failed to create new XLayer!" );

            if( xLayer.is() )
                xLayer->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), Any( msName ) );
        }

        if( xLayer.is() )
        {
            xLayer->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ), Any( sTitleBuffer.makeStringAndClear() ) );
            xLayer->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Description") ), Any( sDescriptionBuffer.makeStringAndClear() ) );
        }
    }
    catch( Exception& )
    {
        DBG_ERROR("SdXMLLayerContext::EndElement(), exception caught!");
    }
}


TYPEINIT1( SdXMLLayerSetContext, SvXMLImportContext );

SdXMLLayerSetContext::SdXMLLayerSetContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>&)
: SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference< XLayerSupplier > xLayerSupplier( rImport.GetModel(), UNO_QUERY );
    DBG_ASSERT( xLayerSupplier.is(), "xmloff::SdXMLLayerSetContext::SdXMLLayerSetContext(), XModel is not supporting XLayerSupplier!" );
    if( xLayerSupplier.is() )
        mxLayerManager = xLayerSupplier->getLayerManager();
}

SdXMLLayerSetContext::~SdXMLLayerSetContext()
{
}

SvXMLImportContext * SdXMLLayerSetContext::CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLLayerContext( GetImport(), nPrefix, rLocalName, xAttrList, mxLayerManager );
}
