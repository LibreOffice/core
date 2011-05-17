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
                        sal_uInt16 nRet = awt::ImageScaleMode::None;
                        if ( s_sTRUE == sValue )
                        {
                            nRet = awt::ImageScaleMode::Anisotropic;
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
        OSL_FAIL("Exception catched while filling the image props");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
