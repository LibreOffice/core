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
#include "xmlCondPrtExpr.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlControlProperty.hxx"
#include "xmlComponent.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLCondPrtExpr )

OXMLCondPrtExpr::OXMLCondPrtExpr( ORptFilter& _rImport,
                sal_uInt16 nPrfx
                ,const ::rtl::OUString& rLName
                ,const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const Reference< XPropertySet > & _xComponent ) :
    SvXMLImportContext( _rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLCondPrtExpr,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();
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
                case XML_TOK_FUNCTION_FORMULA:
                    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,uno::makeAny(ORptFilter::convertFormula(sValue)));
                    break;
                default:
                    break;
            }

        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("Exception catched while putting Function props!");
    }
}
// -----------------------------------------------------------------------------

OXMLCondPrtExpr::~OXMLCondPrtExpr()
{

    DBG_DTOR( rpt_OXMLCondPrtExpr,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OXMLCondPrtExpr::Characters( const ::rtl::OUString& rChars )
{
    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,makeAny(rChars));
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
