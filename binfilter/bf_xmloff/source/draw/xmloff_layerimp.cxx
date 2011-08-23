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

#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>



#include "xmlimp.hxx"

#include "xmlnmspe.hxx"


#include "nmspmap.hxx"

#include "layerimp.hxx"
namespace binfilter {

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_NAME;

using rtl::OUString;


TYPEINIT1( SdXMLLayerSetContext, SvXMLImportContext );

SdXMLLayerSetContext::SdXMLLayerSetContext( SvXMLImport& rImport, sal_uInt16 nPrfx,	const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList)
: SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference< XLayerSupplier > xLayerSupplier( rImport.GetModel(), UNO_QUERY );
    DBG_ASSERT( xLayerSupplier.is(), "XModel is not supporting XLayerSupplier!" );
    if( xLayerSupplier.is() )
        mxLayerManager = xLayerSupplier->getLayerManager();
}

SdXMLLayerSetContext::~SdXMLLayerSetContext()
{
}

SvXMLImportContext * SdXMLLayerSetContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( mxLayerManager.is() )
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
                    aName = sValue;
                }
            }
        }

        DBG_ASSERT( aName.getLength(), "draw:layer element without draw:name!" );
        if( aName.getLength() )
        {
            Reference< XPropertySet > xLayer;

            if( mxLayerManager->hasByName( aName ) )
            {
                mxLayerManager->getByName( aName ) >>= xLayer;
                DBG_ASSERT( xLayer.is(), "failed to get existing XLayer!" );
            }
            else
            {
                Reference< XLayerManager > xLayerManager( mxLayerManager, UNO_QUERY );
                if( xLayerManager.is() )
                    xLayer = Reference< XPropertySet >::query( xLayerManager->insertNewByIndex( xLayerManager->getCount() ) );
                DBG_ASSERT( xLayer.is(), "failed to create new XLayer!" );

                if( xLayer.is() )
                {
                    Any aAny;
                    aAny <<= aName;
                    xLayer->setPropertyValue( strName, aAny );
                }
            }
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
