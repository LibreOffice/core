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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"


#include "XMLFootnoteImportContext.hxx"

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>

#include "XMLFootnoteBodyImportContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "XMLTextListItemContext.hxx"

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XFootnote.hpp>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1(XMLFootnoteImportContext, SvXMLImportContext);

const sal_Char sAPI_service_footnote[] = "com.sun.star.text.Footnote";
const sal_Char sAPI_service_endnote[] = "com.sun.star.text.Endnote";

enum XMLFootnoteChildToken {
    XML_TOK_FTN_NOTE_CITATION,
    XML_TOK_FTN_NOTE_BODY
};

static SvXMLTokenMapEntry aFootnoteChildTokenMap[] =
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
,   sPropertyReferenceId(RTL_CONSTASCII_USTRINGPARAM("ReferenceId"))
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
    if( xFactory.is() )
    {
        // create endnote or footnote
        sal_Bool bIsEndnote = sal_False;
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
                    bIsEndnote = sal_True;
                break;
            }
        }

        Reference<XInterface> xIfc = xFactory->createInstance(
            bIsEndnote ?
            OUString(RTL_CONSTASCII_USTRINGPARAM(sAPI_service_endnote)) :
            OUString(RTL_CONSTASCII_USTRINGPARAM(sAPI_service_footnote)) );

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
                Any aAny =xPropertySet->getPropertyValue(sPropertyReferenceId);
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
    }
    // else: ignore footnote! Content will be merged into document.
}

void XMLFootnoteImportContext::Characters(const OUString&)
{
    // ignore characters! Text must be contained in paragraphs!
    // rHelper.InsertString(rString);
}

void XMLFootnoteImportContext::EndElement()
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


SvXMLImportContext *XMLFootnoteImportContext::CreateChildContext(
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    SvXMLTokenMap aTokenMap(aFootnoteChildTokenMap);

    switch(aTokenMap.Get(p_nPrefix, rLocalName))
    {
        case XML_TOK_FTN_NOTE_CITATION:
        {
            // little hack: we only care for one attribute of the citation
            //              element. We handle that here, and then return a
            //              default context.
            sal_Int16 nLength = xAttrList->getLength();
            for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
            {
                OUString sLocalName;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                    GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                                      &sLocalName );

                if ( (nPrefix == XML_NAMESPACE_TEXT) &&
                     IsXMLToken( sLocalName, XML_LABEL ) )
                {
                    xFootnote->setLabel(xAttrList->getValueByIndex(nAttr));
                }
            }

            // ignore content: return default context
            pContext = new SvXMLImportContext(GetImport(),
                                              p_nPrefix, rLocalName);
            break;
        }

        case XML_TOK_FTN_NOTE_BODY:
            // return footnote body
            pContext = new XMLFootnoteBodyImportContext(GetImport(),
                                                        p_nPrefix, rLocalName);
            break;
        default:
            // default:
            pContext = SvXMLImportContext::CreateChildContext(p_nPrefix,
                                                              rLocalName,
                                                              xAttrList);
            break;
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
