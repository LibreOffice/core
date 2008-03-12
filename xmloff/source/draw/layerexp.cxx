/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerexp.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:33:07 $
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

#ifndef _COM_SUN_STAR_DRAWING_XLAYERSUPPLIER_HPP_
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_LAYEREXP_HXX
#include "layerexp.hxx"
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;

void SdXMLayerExporter::exportLayer( SvXMLExport& rExport )
{
    Reference< XLayerSupplier > xLayerSupplier( rExport.GetModel(), UNO_QUERY );
    if( !xLayerSupplier.is() )
        return;

    Reference< XIndexAccess > xLayerManager( xLayerSupplier->getLayerManager(), UNO_QUERY );
    if( !xLayerManager.is() )
        return;

    const sal_Int32 nCount = xLayerManager->getCount();
    if( nCount == 0 )
        return;

    const OUString strName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
    const OUString strTitle( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    const OUString strDescription( RTL_CONSTASCII_USTRINGPARAM( "Description" ) );

    OUString sTmp;

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER_SET, sal_True, sal_True );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        try
        {
            Reference< XPropertySet> xLayer( xLayerManager->getByIndex( nIndex ), UNO_QUERY_THROW );
            xLayer->getPropertyValue( strName ) >>= sTmp;
            if(sTmp.getLength())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, sTmp );

            SvXMLElementExport aEle( rExport, XML_NAMESPACE_DRAW, XML_LAYER, sal_True, sal_True );

            // title property (as <svg:title> element)
            xLayer->getPropertyValue(strTitle) >>= sTmp;
            if(sTmp.getLength())
            {
                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SVG, XML_TITLE, sal_True, sal_False);
                rExport.Characters(sTmp);
            }

            // description property (as <svg:desc> element)
            xLayer->getPropertyValue(strDescription) >>= sTmp;
            if(sTmp.getLength() > 0)
            {
                SvXMLElementExport aDesc(rExport, XML_NAMESPACE_SVG, XML_DESC, sal_True, sal_False);
                rExport.Characters(sTmp);
            }
        }
        catch( Exception& )
        {
            DBG_ERROR("SdXMLayerExporter::exportLayer(), exception caught during export of one layer!");
        }
    }
}
