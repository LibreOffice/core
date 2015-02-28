/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "doc.hxx"
#include "drawdoc.hxx"
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <UndoManager.hxx>
#include "ndtxt.hxx"
#include "MarkManager.hxx"
#include "switerator.hxx"
#include "docufld.hxx"
#include "txatbase.hxx"
#include "redline.hxx"
#include <swmodule.hxx>
#include <svl/itemiter.hxx>
#include <tools/datetimeutils.hxx>

#include <libxml/xmlwriter.h>

using namespace com::sun::star;

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
    operator xmlTextWriterPtr() { return writer;}
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

// Hack: somehow conversion from "..." to va_list does
// bomb on two string litterals in the format.
static const char* TMP_FORMAT = "%" SAL_PRIuUINTPTR;

}

void SwFldTypes::dumpAsXml( xmlTextWriterPtr w ) const
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
            const char* name = "FIXME_unhandledfield";
            switch(pCurFldFmt->GetField()->GetTyp()->Which())
            {
                case RES_PAGENUMBERFLD: name = "swpagenumberfield"; break;
                case RES_POSTITFLD: name = "swpostitfield"; break;
                case RES_DATETIMEFLD: name = "swdatetimefield"; break;
                default:
                    SAL_INFO("sw.core", "unhandled field type " << pCurFldFmt->GetField()->GetTyp()->Which());
                    break;
            }
            writer.startElement(name);
            writer.writeFormatAttribute("ptr", "%p", pCurFldFmt->GetField());
            const SwPostItField* pField = NULL;
            if (pCurFldFmt->GetField()->GetTyp()->Which() == RES_POSTITFLD)
                pField = dynamic_cast<const SwPostItField*>(pCurFldFmt->GetField());
            if (pField)
            {
                OString txt8 = OUStringToOString(pField->GetName(), RTL_TEXTENCODING_UTF8);
                writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
            }
            writer.endElement();
            writer.endElement();
        }
    }
    writer.endElement();
}

void SwNode::dumpAsXml( xmlTextWriterPtr w ) const
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
            abort(); // overridden
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
    writer.writeFormatAttribute( "type", "0x%04x", GetNodeType() );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );
    writer.endElement();
    if( GetNodeType() == ND_ENDNODE )
        writer.endElement(); // end start node
}

void SwStartNode::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    const char* name = "???";
    switch( GetNodeType() )
    {
        case ND_TABLENODE:
            name = "table";
            break;
        case ND_SECTIONNODE:
            name = "section";
            break;
        default:
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
            break;
    }
    writer.startElement( name );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "type", "0x%04x", GetNodeType() );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );

    if (IsTableNode())
    {
        writer.startElement("attrset");
        GetTableNode()->GetTable().GetFrmFmt()->GetAttrSet().dumpAsXml(writer);
        writer.endElement();
    }

    // writer.endElement(); - it is a start node, so don't end, will make xml better nested
}

void SwTxtNode::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    writer.startElement( "text" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );
    OUString txt = GetTxt();
    for( int i = 0; i < 32; ++i )
        txt = txt.replace( i, '*' );
    OString txt8 = OUStringToOString( txt, RTL_TEXTENCODING_UTF8 );
    writer.startElement("inner_text");
    xmlTextWriterWriteString( writer, BAD_CAST( txt8.getStr()));
    writer.endElement( );

    if (GetFmtColl())
    {
        SwTxtFmtColl* pColl = static_cast<SwTxtFmtColl*>(GetFmtColl());
        writer.startElement("swtxtfmtcoll");
        OString aName = OUStringToOString(pColl->GetName(), RTL_TEXTENCODING_UTF8);
        writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));
        writer.endElement();
    }

    if (HasSwAttrSet())
    {
        writer.startElement("attrset");
        GetSwAttrSet().dumpAsXml(writer);
        writer.endElement();
    }

    if (HasHints())
    {
        writer.startElement("hints");
        const SwpHints& rHints = GetSwpHints();
        for (size_t i = 0; i < rHints.Count(); ++i)
            rHints.GetTextHint(i)->dumpAsXml(w);
        writer.endElement();
    }
    if (GetNumRule())
        GetNumRule()->dumpAsXml(w);

    writer.endElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
