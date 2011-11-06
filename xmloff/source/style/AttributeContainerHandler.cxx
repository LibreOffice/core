/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>

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
