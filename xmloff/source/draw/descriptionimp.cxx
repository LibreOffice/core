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

#include <tools/debug.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>

#include "descriptionimp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////

SdXMLDescriptionContext::SdXMLDescriptionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,   const OUString& rLocalName,
        const Reference< XAttributeList>&, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLDescriptionContext::~SdXMLDescriptionContext()
{
}

void SdXMLDescriptionContext::EndElement()
{
    if( msText.getLength() ) try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY_THROW);
        if(IsXMLToken(GetLocalName(),XML_TITLE))
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Title")), Any(msText));
        }
        else
        {
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Description")), Any(msText));
        }
    }
    catch( uno::Exception& )
    {
    }
}

// This method is called for all characters that are contained in the
// current element. The default is to ignore them.
void SdXMLDescriptionContext::Characters( const ::rtl::OUString& rChars )
{
    msText += rChars;
}

