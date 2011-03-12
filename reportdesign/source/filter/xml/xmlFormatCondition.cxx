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
#include "xmlFormatCondition.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlEnums.hxx"
#include "xmlHelper.hxx"
#include <com/sun/star/report/XReportControlFormat.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "xmlstrings.hrc"
#include "xmlStyleImport.hxx"
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
    using namespace ::com::sun::star::beans;

DBG_NAME( rpt_OXMLFormatCondition )

OXMLFormatCondition::OXMLFormatCondition( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XFormatCondition > & _xComponent ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_rImport(rImport)
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLFormatCondition,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetFormatElemTokenMap();
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
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
                case XML_TOK_ENABLED:
                    m_xComponent->setEnabled(sValue == s_sTRUE);
                    break;
                case XML_TOK_FORMULA:
                    m_xComponent->setFormula(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_FORMAT_STYLE_NAME:
                    m_sStyleName = sValue;
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------

OXMLFormatCondition::~OXMLFormatCondition()
{

    DBG_DTOR( rpt_OXMLFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
void OXMLFormatCondition::EndElement()
{
    OXMLHelper::copyStyleElements(m_rImport.isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent.get());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
