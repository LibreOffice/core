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
#include "xmlFormattedField.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlControlProperty.hxx"
#include "xmlHelper.hxx"
#include <xmloff/xmluconv.hxx>
#include "xmlReportElement.hxx"
#include "xmlComponent.hxx"
#include <tools/debug.hxx>


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLFormattedField )

OXMLFormattedField::OXMLFormattedField( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName
                ,const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const uno::Reference< XFormattedField > & _xComponent
                ,OXMLTable* _pContainer
                ,bool _bPageCount) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
{
    DBG_CTOR( rpt_OXMLFormattedField,NULL);
    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetControlElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_DATA_FORMULA:
                    _xComponent->setDataField(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_SELECT_PAGE:
                    {
                        static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("rpt:PageNumber()"));
                        _xComponent->setDataField(s_sPageNumber);
                    }
                    break;
                default:
                    break;
            }
        }
        if ( _bPageCount )
        {
            static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("rpt:PageCount()"));
            _xComponent->setDataField(s_sPageNumber);
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------
OXMLFormattedField::~OXMLFormattedField()
{
    DBG_DTOR( rpt_OXMLFormattedField,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
