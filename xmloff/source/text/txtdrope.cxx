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
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "txtdrope.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;


XMLTextDropCapExport::XMLTextDropCapExport( SvXMLExport& rExp ) :
    rExport(rExp)
{
}

XMLTextDropCapExport::~XMLTextDropCapExport()
{
}

void XMLTextDropCapExport::exportXML( const Any& rAny,
                                      sal_Bool bWholeWord,
                                      const OUString& rStyleName )
{
    DropCapFormat aFormat;
    rAny >>= aFormat;
    OUString sValue;
    OUStringBuffer sBuffer;
    if( aFormat.Lines > 1 )
    {
        SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

        // style:lines
        rUnitConv.convertNumber( sBuffer, (sal_Int32)aFormat.Lines );
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LINES,
                              sBuffer.makeStringAndClear() );

        // style:length
        if( bWholeWord )
        {
            sValue = GetXMLToken(XML_WORD);
        }
        else if( aFormat.Count > 1 )
        {
            rUnitConv.convertNumber( sBuffer, (sal_Int32)aFormat.Count );
            sValue = sBuffer.makeStringAndClear();
        }
        if( sValue.getLength() )
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LENGTH, sValue );

        // style:distance
        if( aFormat.Distance > 0 )
        {
            rUnitConv.convertMeasure( sBuffer, aFormat.Distance );
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_DISTANCE,
                                  sBuffer.makeStringAndClear() );
        }

        // style:style-name
        if( rStyleName.getLength() )
            rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_STYLE_NAME,
                                  rExport.EncodeStyleName( rStyleName ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_DROP_CAP,
                              sal_False, sal_False );
}



