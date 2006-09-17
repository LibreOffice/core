/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerexp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:27:27 $
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
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_LAYEREXP_HXX
#include "layerexp.hxx"
#endif

using namespace ::rtl;
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

    Reference< XPropertySet> xLayer;
    const OUString strName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );

    OUStringBuffer sTmp;
    OUString aName;

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER_SET, sal_True, sal_True );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        xLayerManager->getByIndex( nIndex ) >>= xLayer;

        if( xLayer.is() )
        {
            if( xLayer->getPropertyValue( strName ) >>= aName )
            {
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aName );
            }

            SvXMLElementExport aEle( rExport, XML_NAMESPACE_DRAW, XML_LAYER, sal_True, sal_True );
        }
    }
}
