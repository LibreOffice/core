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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/AttributeData.hpp>




#include "AttributeContainerHandler.hxx"
namespace binfilter {

using namespace ::rtl;
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

sal_Bool XMLAttributeContainerHandler::equals(
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

sal_Bool XMLAttributeContainerHandler::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    return sal_True;
}

sal_Bool XMLAttributeContainerHandler::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    return sal_True;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
