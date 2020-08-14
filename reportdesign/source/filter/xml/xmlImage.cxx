/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "xmlImage.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include <unotools/pathoptions.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/awt/ImageScaleMode.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;


OXMLImage::OXMLImage( ORptFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XImageControl > & _xComponent
                ,OXMLTable* _pContainer) :
    OXMLReportElementBase( rImport,_xComponent.get(),_pContainer)
{

    OSL_ENSURE(m_xReportComponent.is(),"Component is NULL!");
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);

    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            OUString sValue = aIter.toString();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(FORM, XML_IMAGE_DATA):
                {
                    SvtPathOptions aPathOptions;
                    sValue = aPathOptions.SubstituteVariable(sValue);
                    _xComponent->setImageURL(rImport.GetAbsoluteReference( sValue ));
                    break;
                }
                case XML_ELEMENT(REPORT, XML_PRESERVE_IRI):
                    _xComponent->setPreserveIRI(s_sTRUE == sValue);
                    break;
                case XML_ELEMENT(REPORT, XML_SCALE):
                {
                    sal_Int16 nRet = awt::ImageScaleMode::NONE;
                    if ( s_sTRUE == sValue )
                    {
                        nRet = awt::ImageScaleMode::ANISOTROPIC;
                    }
                    else
                    {
                        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetImageScaleOptions();
                        bool bConvertOk = SvXMLUnitConverter::convertEnum( nRet, sValue, aXML_EnumMap );
                        SAL_WARN_IF(!bConvertOk, "reportdesign", "convertEnum failed");
                    }
                    _xComponent->setScaleMode( nRet );
                    break;
                }
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    _xComponent->setDataField(ORptFilter::convertFormula(sValue));
                    break;
                default:
                    SAL_WARN("reportdesign", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " = " << sValue);
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception caught while filling the image props");
    }
}


OXMLImage::~OXMLImage()
{

}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
