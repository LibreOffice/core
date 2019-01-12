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

#include "XMLTextCharStyleNamesElementExport.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <osl/diagnose.h>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::XPropertySet;
using namespace ::xmloff::token;

XMLTextCharStyleNamesElementExport::XMLTextCharStyleNamesElementExport(
    SvXMLExport& rExp,
    bool bDoSth,
    bool bAllStyles,
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
            if ( bAllStyles ) ++nCount;
            if( nCount > 1 )
            {
                aName = rExport.GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT, GetXMLToken(XML_SPAN) );
                sal_Int32 i = nCount;
                const OUString *pName = aNames.getConstArray();
                while( --i )
                {
                    rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          rExport.EncodeStyleName( *pName ) );
                    rExport.StartElement( aName, false );
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
            rExport.EndElement( aName, false );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
