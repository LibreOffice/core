/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "doc.hxx"
#include "ndtxt.hxx"
#include "MarkManager.hxx"
#include "docary.hxx"
#include "switerator.hxx"
#include "fmtfld.hxx"
#include "docufld.hxx"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

namespace
{

// Small helper class to ensure that we write to nodes.xml if nothing
// has been explicitly specified.
// Always use at the beginning of dumpAsXml().
// Also, there are some functions to save typing.
class WriterHelper
{
public:
    WriterHelper( xmlTextWriterPtr );
    ~WriterHelper();
    operator xmlTextWriterPtr();
    xmlTextWriterPtr operator->();
    void startElement( const char* element );
    void endElement();
    void writeFormatAttribute( const char* attribute, const char* format, ... )
#ifdef LIBXML_ATTR_FORMAT
        LIBXML_ATTR_FORMAT(3,4)
#endif
        ;
private:
    xmlTextWriterPtr writer;
    bool owns;
};

WriterHelper::WriterHelper( xmlTextWriterPtr w )
    : writer( w )
    , owns( false )
{
    if( writer == NULL )
    {
        writer = xmlNewTextWriterFilename( "nodes.xml", 0 );
        xmlTextWriterStartDocument( writer, NULL, NULL, NULL );
        owns = true;
    }
}

WriterHelper::~WriterHelper()
{
    if( owns )
    {
        xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
    }
}

WriterHelper::operator xmlTextWriterPtr()
{
    return writer;
}

xmlTextWriterPtr WriterHelper::operator->()
{
    return writer;
}

void WriterHelper::startElement( const char* element )
{
    xmlTextWriterStartElement( writer, BAD_CAST( element ));
}

void WriterHelper::endElement()
{
    xmlTextWriterEndElement( writer );
}

void WriterHelper::writeFormatAttribute( const char* attribute, const char* format, ... )
{
    va_list va;
    va_start( va, format );
    xmlTextWriterWriteVFormatAttribute( writer, BAD_CAST( attribute ), format, va );
    va_end( va );
}

}

void SwDoc::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer( w );
    writer.startElement( "doc" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    m_pNodes->dumpAsXml( writer );
    pMarkManager->dumpAsXml( writer );
    pFldTypes->dumpAsXml( writer );
    writer.endElement();
}

namespace sw {
namespace mark {
void MarkManager::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer(w);
    writer.startElement("markManager");
    writer.startElement("fieldmarks");
    for (const_iterator_t it = m_vFieldmarks.begin(); it != m_vFieldmarks.end(); ++it)
    {
        pMark_t pMark = *it;
        writer.startElement("fieldmark");
        writer.writeFormatAttribute("startNode", "%lu", pMark->GetMarkStart().nNode.GetIndex());
        writer.writeFormatAttribute("startOffset", "%d", pMark->GetMarkStart().nContent.GetIndex());
        writer.writeFormatAttribute("endNode", "%lu", pMark->GetMarkEnd().nNode.GetIndex());
        writer.writeFormatAttribute("endOffset", "%d", pMark->GetMarkEnd().nContent.GetIndex());
        OString txt8 = OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
        writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
        writer.endElement();
    }
    writer.endElement();
    writer.endElement();
}
} // namespace mark
} // namespace sw

void SwFldTypes::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer(w);
    writer.startElement("swfldtypes");
    sal_uInt16 nCount = size();
    for (sal_uInt16 nType = 0; nType < nCount; ++nType)
    {
        const SwFieldType *pCurType = (*this)[nType];
        SwIterator<SwFmtFld, SwFieldType> aIter(*pCurType);
        for (const SwFmtFld* pCurFldFmt = aIter.First(); pCurFldFmt; pCurFldFmt = aIter.Next())
        {
            writer.startElement("swfmtfld");
            writer.writeFormatAttribute("ptr", "%p", pCurFldFmt);
            writer.writeFormatAttribute("pTxtAttr", "%p", pCurFldFmt->GetTxtFld());
            const char* name = "???";
            switch(pCurFldFmt->GetFld()->GetTyp()->Which())
            {
                case RES_POSTITFLD:
                    name = "swpostitfield";
                    break;
                default:
                    SAL_INFO("sw.core", "unhandled field type " << pCurFldFmt->GetFld()->GetTyp()->Which());
                    break;
            }
            writer.startElement(name);
            writer.writeFormatAttribute("ptr", "%p", pCurFldFmt->GetFld());
            if (pCurFldFmt->GetFld()->GetTyp()->Which() == RES_POSTITFLD)
            {
                const SwPostItField* pField = dynamic_cast<const SwPostItField*>(pCurFldFmt->GetFld());
                OString txt8 = OUStringToOString(pField->GetName(), RTL_TEXTENCODING_UTF8);
                writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
            }
            writer.endElement();
            writer.endElement();
        }
    }
    writer.endElement();
}

void SwNodes::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer( w );
    writer.startElement( "swnodes" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    for( unsigned int i = 0; i < Count(); ++i )
    {
        ( *this )[ i ]->dumpAsXml( writer );
    }
    writer.endElement();
}

void SwNode::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer( w );
    const char* name = "???";
    switch( GetNodeType())
    {
        case ND_ENDNODE:
            name = "end";
            break;
        case ND_STARTNODE:
        case ND_TEXTNODE:
            abort(); // overriden
        case ND_TABLENODE:
            name = "table";
            break;
        case ND_GRFNODE:
            name = "grf";
            break;
        case ND_OLENODE:
            name = "ole";
            break;
    }
    writer.startElement( name );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "index", "%lu", GetIndex() );
    writer.endElement();
    if( GetNodeType() == ND_ENDNODE )
        writer.endElement(); // end start node
}

void SwStartNode::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer( w );
    const char* name = "???";
    switch( GetStartNodeType())
    {
        case SwNormalStartNode:
            name = "start";
            break;
        case SwTableBoxStartNode:
            name = "tablebox";
            break;
        case SwFlyStartNode:
            name = "fly";
            break;
        case SwFootnoteStartNode:
            name = "footnote";
            break;
        case SwHeaderStartNode:
            name = "header";
            break;
        case SwFooterStartNode:
            name = "footer";
            break;
    }
    writer.startElement( name );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "index", "%lu", GetIndex() );
    // writer.endElement(); - it is a start node, so don't end, will make xml better nested
}

void SwTxtNode::dumpAsXml( xmlTextWriterPtr w )
{
    WriterHelper writer( w );
    writer.startElement( "text" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "index", "%lu", GetIndex() );
    rtl::OUString txt = GetTxt();
    for( int i = 0; i < 32; ++i )
        txt = txt.replace( i, '*' );
    rtl::OString txt8 = ::rtl::OUStringToOString( txt, RTL_TEXTENCODING_UTF8 );
    xmlTextWriterWriteString( writer, BAD_CAST( txt8.getStr()));
    writer.endElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
