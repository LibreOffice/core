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

#include "XMLTextPContext.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/text/XTextCursor.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

class ScXMLTextTContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLTextTContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTextPContext* pTextPContext);

    virtual ~ScXMLTextTContext();
};


ScXMLTextTContext::ScXMLTextTContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLTextPContext* pTextPContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if (pTextPContext)
    {
        sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
        rtl::OUString aLocalName;
        sal_Int32 nCount(1);
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                xAttrList->getNameByIndex( i ), &aLocalName ));
            const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

            if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(aLocalName, XML_C))
                nCount = sValue.toInt32();
        }
        pTextPContext->AddSpaces(nCount);
    }
}

ScXMLTextTContext::~ScXMLTextTContext()
{
}

//------------------------------------------------------------------

ScXMLTextPContext::ScXMLTextPContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xAttrList(xTempAttrList),
    pTextPContext(NULL),
    pCellContext(pTempCellContext),
    sLName(rLName),
    sSimpleContent(),
    pContentBuffer(NULL),
    nPrefix(nPrfx)
{
    // here are no attributes
}

ScXMLTextPContext::~ScXMLTextPContext()
{
    if (pTextPContext)
        delete pTextPContext;
    if (pContentBuffer)
        delete pContentBuffer;
}

void ScXMLTextPContext::AddSpaces(sal_Int32 nSpaceCount)
{
    // use pContentBuffer
    if ( !pContentBuffer )
        pContentBuffer = new rtl::OUStringBuffer( sSimpleContent );

    sal_Char* pChars = new sal_Char[nSpaceCount];
    memset(pChars, ' ', nSpaceCount);
    pContentBuffer->appendAscii(pChars, nSpaceCount);
}

SvXMLImportContext *ScXMLTextPContext::CreateChildContext( sal_uInt16 nTempPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList )
{
    SvXMLImportContext *pContext(NULL);
    if (!pTextPContext &&
        (nTempPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLName, XML_S))
        pContext = new ScXMLTextTContext( GetScImport(), nTempPrefix, rLName, xTempAttrList, this);
    else
    {
        if (!pTextPContext)
        {
            rtl::OUString sSetString;
            if ( pContentBuffer )
                sSetString = pContentBuffer->makeStringAndClear();
            else
                sSetString = sSimpleContent;

            sal_Unicode cNonSpace(0);

            sal_Int32 nLength = sSetString.getLength();
            if ( nLength > 0 )
            {
                sal_Unicode cLast = sSetString.getStr()[ nLength - 1 ];
                if ( cLast != (sal_Unicode)' ' )
                {
                    // #i53253# To keep XMLParaContext's whitespace handling in sync,
                    // if there's a non-space character at the end of the existing string,
                    // it has to be processed by XMLParaContext.

                    cNonSpace = cLast;
                    sSetString = sSetString.copy( 0, nLength - 1 );  // remove from the string for SetCursorOnTextImport
                }
            }

            pCellContext->SetCursorOnTextImport( sSetString );

            pTextPContext = GetScImport().GetTextImport()->CreateTextChildContext(
                                    GetScImport(), nPrefix, sLName, xAttrList);

            if ( cNonSpace != 0 )
            {
                // pass non-space character through XMLParaContext, so a following space isn't ignored
                pTextPContext->Characters( rtl::OUString( cNonSpace ) );
            }
        }
        if (pTextPContext)
            pContext = pTextPContext->CreateChildContext(nTempPrefix, rLName, xTempAttrList);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetScImport(), nTempPrefix, rLName );

    return pContext;
}

void ScXMLTextPContext::Characters( const ::rtl::OUString& rChars )
{
    if (!pTextPContext)
    {
        // For the first call to an empty context, copy (ref-counted) the OUString.
        // The OUStringBuffer is used only if there is more complex content.

        if ( !pContentBuffer && sSimpleContent.isEmpty() )
            sSimpleContent = rChars;
        else
        {
            if ( !pContentBuffer )
                pContentBuffer = new rtl::OUStringBuffer( sSimpleContent );
            pContentBuffer->append(rChars);
        }
    }
    else
        pTextPContext->Characters(rChars);
}

void ScXMLTextPContext::EndElement()
{
    if (!pTextPContext)
    {
        if ( pContentBuffer )
            pCellContext->SetString(pContentBuffer->makeStringAndClear());
        else
            pCellContext->SetString(sSimpleContent);
    }
    else
    {
        pTextPContext->EndElement();
        GetScImport().SetRemoveLastChar(sal_True);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
