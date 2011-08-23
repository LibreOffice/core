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

#include "XMLTextCharStyleNamesElementExport.hxx"
#include "xmlnmspe.hxx"
#include "nmspmap.hxx"
#include "xmlexp.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }
namespace binfilter {

using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::XPropertySet;
using ::rtl::OUString;
using namespace ::binfilter::xmloff::token;

XMLTextCharStyleNamesElementExport::XMLTextCharStyleNamesElementExport(
    SvXMLExport& rExp,
    sal_Bool bDoSth,
    const Reference < XPropertySet > & rPropSet,
    const OUString& rPropName ) :
    rExport( rExp ),
    nCount( 0 )
{
    if( bDoSth )
    {
        Any aAny = rPropSet->getPropertyValue( rPropName );
        Sequence < OUString > aNames;
        if( aAny >>= aNames )
        {
            nCount = aNames.getLength();
            OSL_ENSURE( nCount > 0, "no char style found" );
            if( nCount > 1 )
            {
                aName = rExport.GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT, GetXMLToken(XML_SPAN) );
                sal_Int32 i = nCount;
                const OUString *pName = aNames.getConstArray();
                while( --i )
                {
                    rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          *pName );
                    rExport.StartElement( aName, sal_False );
                    ++pName;
                }
            }
        }
    }
}

XMLTextCharStyleNamesElementExport::~XMLTextCharStyleNamesElementExport()
{
    if( nCount > 1 )
    {
        sal_Int32 i = nCount;
        while( --i )
            rExport.EndElement( aName, sal_False );
    }
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
