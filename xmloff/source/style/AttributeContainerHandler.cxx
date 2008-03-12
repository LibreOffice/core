/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AttributeContainerHandler.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:41:02 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif

#include "AttributeContainerHandler.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLAttributeContainerHandler
//

XMLAttributeContainerHandler::~XMLAttributeContainerHandler()
{
    // nothing to do
}

bool XMLAttributeContainerHandler::equals(
        const Any& r1,
        const Any& r2 ) const
{
    Reference< XNameContainer > xContainer1;
    Reference< XNameContainer > xContainer2;

    if( ( r1 >>= xContainer1 ) && ( r2 >>= xContainer2 ) )
    {
        uno::Sequence< OUString > aAttribNames1( xContainer1->getElementNames() );
        uno::Sequence< OUString > aAttribNames2( xContainer2->getElementNames() );
        const sal_Int32 nCount = aAttribNames1.getLength();

        if( aAttribNames2.getLength() == nCount )
        {
            const OUString* pAttribName = aAttribNames1.getConstArray();

            xml::AttributeData aData1;
            xml::AttributeData aData2;

            for( sal_Int32 i=0; i < nCount; i++, pAttribName++ )
            {
                if( !xContainer2->hasByName( *pAttribName ) )
                    return sal_False;

                xContainer1->getByName( *pAttribName ) >>= aData1;
                xContainer2->getByName( *pAttribName ) >>= aData2;

                if( ( aData1.Namespace != aData2.Namespace ) ||
                    ( aData1.Type      != aData2.Type      ) ||
                    ( aData1.Value     != aData2.Value     ) )
                    return sal_False;
            }

            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool XMLAttributeContainerHandler::importXML( const OUString& /*rStrImpValue*/, Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_True;
}

sal_Bool XMLAttributeContainerHandler::exportXML( OUString& /*rStrExpValue*/, const Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_True;
}
