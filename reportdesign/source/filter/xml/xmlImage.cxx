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


#include "precompiled_reportdesign.hxx"
#include "xmlImage.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlEnums.hxx"
#include "xmlComponent.hxx"
#include "xmlReportElement.hxx"
#include "xmlControlProperty.hxx"
#include "xmlHelper.hxx"
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include <comphelper/componentcontext.hxx>
#include <com/sun/star/awt/ImageScaleMode.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME( rpt_OXMLImage )


// -----------------------------------------------------------------------------
OXMLImage::OXMLImage( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XImageControl > & _xComponent
                ,OXMLTable* _pContainer) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
{
    DBG_CTOR( rpt_OXMLImage,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = m_rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = m_rImport.GetControlElemTokenMap();
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            /* const */ rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_IMAGE_DATA:
                    {
                        SvtPathOptions aPathOptions;
                        sValue = aPathOptions.SubstituteVariable(sValue);
                        _xComponent->setImageURL(rImport.GetAbsoluteReference( sValue ));
                    }

                    break;
                case XML_TOK_PRESERVE_IRI:
                    _xComponent->setPreserveIRI(s_sTRUE == sValue);
                    break;
                case XML_TOK_SCALE:
                    {
                        sal_uInt16 nRet = awt::ImageScaleMode::NONE;
                        if ( s_sTRUE == sValue )
                        {
                            nRet = awt::ImageScaleMode::ANISOTROPIC;
                        }
                        else
                        {
                                   const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetImageScaleOptions();
                                   SvXMLUnitConverter::convertEnum( nRet, sValue, aXML_EnumMap );
                        }
                        _xComponent->setScaleMode( nRet );
                    }
                    break;
                case XML_TOK_DATA_FORMULA:
                    _xComponent->setDataField(ORptFilter::convertFormula(sValue));
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the image props");
    }
}
// -----------------------------------------------------------------------------

OXMLImage::~OXMLImage()
{

    DBG_DTOR( rpt_OXMLImage,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
