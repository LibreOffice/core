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

#include "XMLFootnoteImportContext.hxx"

#include <rtl/ustring.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

#include "XMLFootnoteBodyImportContext.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <sal/log.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

namespace {

enum XMLFootnoteChildToken {
    XML_TOK_FTN_NOTE_CITATION,
    XML_TOK_FTN_NOTE_BODY
};

}

const SvXMLTokenMapEntry aFootnoteChildTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_NOTE_CITATION,
      XML_TOK_FTN_NOTE_CITATION },
    { XML_NAMESPACE_TEXT, XML_NOTE_BODY, XML_TOK_FTN_NOTE_BODY },
    XML_TOKEN_MAP_END
};


XMLFootnoteImportContext::XMLFootnoteImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& rLocalName )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   mbListContextPushed(false)
,   rHelper(rHlp)
{
}

void XMLFootnoteImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // create footnote
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                             UNO_QUERY);
    if( !xFactory.is() )
        return;

    // create endnote or footnote
    bool bIsEndnote = false;
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr1 = 0; nAttr1 < nLength; nAttr1++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr1),
                              &sLocalName );
        if( XML_NAMESPACE_TEXT == nPrefix && IsXMLToken( sLocalName,
                                                        XML_NOTE_CLASS ) )
        {
            const OUString& rValue = xAttrList->getValueByIndex( nAttr1 );
            if( IsXMLToken( rValue, XML_ENDNOTE ) )
                bIsEndnote = true;
            break;
        }
    }

    Reference<XInterface> xIfc = xFactory->createInstance(
        bIsEndnote ?
        OUString("com.sun.star.text.Endnote") :
        OUString("com.sun.star.text.Footnote") );

    // attach footnote to document
    Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
    rHelper.InsertTextContent(xTextContent);

    // process id attribute
    for(sal_Int16 nAttr2 = 0; nAttr2 < nLength; nAttr2++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr2),
                              &sLocalName );

        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken( sLocalName, XML_ID )   )
        {
            // get ID ...
            Reference<XPropertySet> xPropertySet(xTextContent, UNO_QUERY);
            Any aAny =xPropertySet->getPropertyValue("ReferenceId");
            sal_Int16 nID = 0;
            aAny >>= nID;

            // ... and insert into map
            rHelper.InsertFootnoteID(
                xAttrList->getValueByIndex(nAttr2),
                nID);
        }
    }

    // save old cursor and install new one
    xOldCursor = rHelper.GetCursor();
    Reference<XText> xText(xTextContent, UNO_QUERY);
    rHelper.SetCursor(xText->createTextCursor());

    // remember old list item and block (#89891#) and reset them
    // for the footnote
    rHelper.PushListContext();
    mbListContextPushed = true;

    // remember footnote (for CreateChildContext)
    Reference<XFootnote> xNote(xTextContent, UNO_QUERY);
    xFootnote = xNote;

    // else: ignore footnote! Content will be merged into document.
}

void XMLFootnoteImportContext::endFastElement(sal_Int32 )
{
    // get rid of last dummy paragraph
    rHelper.DeleteParagraph();

    // reinstall old cursor
    rHelper.SetCursor(xOldCursor);

    // reinstall old list item
    if (mbListContextPushed) {
        rHelper.PopListContext();
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFootnoteImportContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    switch(nElement)
    {
        case XML_ELEMENT(TEXT, XML_NOTE_CITATION):
        {
            // little hack: we only care for one attribute of the citation
            //              element. We handle that here, and then return a
            //              default context.
            for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
            {
                if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_LABEL) )
                    xFootnote->setLabel(aIter.toString());
            }

            // ignore content: return default context
            break;
        }

        case XML_ELEMENT(TEXT, XML_NOTE_BODY):
            // return footnote body
            xContext = new XMLFootnoteBodyImportContext(GetImport());
            break;

        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return xContext.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
