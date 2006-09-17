/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerimp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:27:39 $
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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_DRAWING_XLAYERMANAGER_HPP_
#include <com/sun/star/drawing/XLayerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XLAYERSUPPLIER_HPP_
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_LAYERIMP_HXX
#include "layerimp.hxx"
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
using namespace ::com::sun::star::container;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_NAME;


TYPEINIT1( SdXMLLayerSetContext, SvXMLImportContext );

SdXMLLayerSetContext::SdXMLLayerSetContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>&)
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
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
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
