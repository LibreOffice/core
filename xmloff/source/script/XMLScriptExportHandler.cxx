/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLScriptExportHandler.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLScriptExportHandler.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <tools/debug.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlnmspe.hxx"


using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;


XMLScriptExportHandler::XMLScriptExportHandler() :
    sURL(RTL_CONSTASCII_USTRINGPARAM("Script"))
{
}

XMLScriptExportHandler::~XMLScriptExportHandler()
{
}

void XMLScriptExportHandler::Export(
    SvXMLExport& rExport,
    const OUString& rEventQName,
    Sequence<PropertyValue> & rValues,
    sal_Bool bUseWhitespace)
{

    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                         rExport.GetNamespaceMap().GetQNameByKey(
                             XML_NAMESPACE_OOO, GetXMLToken(XML_SCRIPT) ) );
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, rEventQName);

    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        if (sURL.equals(rValues[i].Name))
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sTmp);
        }
        // else: disregard
    }

    SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT,
                                   XML_EVENT_LISTENER,
                                   bUseWhitespace, sal_False);
}
