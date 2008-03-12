/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFootnoteImportContext.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:01:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"


#include "XMLFootnoteImportContext.hxx"


#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLFOOTNOTEBODYIMPORTCONTEXT_HXX
#include "XMLFootnoteBodyImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif



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

static __FAR_DATA SvXMLTokenMapEntry aFootnoteChildTokenMap[] =
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
        xListBlock = rHelper.GetListBlock();
        xListItem = rHelper.GetListItem();
        rHelper.SetListBlock( NULL );
        rHelper.SetListItem( NULL );

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
    rHelper.SetListBlock( (XMLTextListBlockContext*)&xListBlock );
    rHelper.SetListItem( (XMLTextListItemContext*)&xListItem );
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
