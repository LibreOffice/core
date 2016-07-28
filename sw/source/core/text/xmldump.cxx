/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "frame.hxx"
#include "frmfmt.hxx"
#include "sectfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "hffrm.hxx"
#include "rootfrm.hxx"
#include "editsh.hxx"
#include "porlin.hxx"
#include "porlay.hxx"
#include "portxt.hxx"
#include "sortedobjs.hxx"
#include <anchoredobject.hxx>
#include <libxml/xmlwriter.h>
#include <SwPortionHandler.hxx>
#include <view.hxx>
#include <svx/svdobj.hxx>

class XmlPortionDumper:public SwPortionHandler
{
  private:
    xmlTextWriterPtr writer;
    sal_Int32 ofs;

    static const char* getTypeName( sal_uInt16 nType )
    {
        switch ( nType )
        {
            case POR_LIN: return "POR_LIN";
            case POR_FLYCNT: return "POR_FLYCNT";

            case POR_HOLE: return "POR_HOLE";
            case POR_TMPEND: return "POR_TMPEND";
            case POR_BRK: return "POR_BRK";
            case POR_KERN: return "POR_KERN";
            case POR_ARROW: return "POR_ARROW";
            case POR_MULTI: return "POR_MULTI";
            case POR_HIDDEN_TXT: return "POR_HIDDEN_TXT";
            case POR_CONTROLCHAR: return "POR_CONTROLCHAR";

            case POR_TXT: return "POR_TXT";
            case POR_LAY: return "POR_LAY";
            case POR_PARA: return "POR_PARA";
            case POR_URL: return "POR_URL";
            case POR_HNG: return "POR_HNG";

            case POR_DROP: return "POR_DROP";
            case POR_TOX: return "POR_TOX";
            case POR_ISOTOX: return "POR_ISOTOX";
            case POR_REF: return "POR_REF";
            case POR_ISOREF: return "POR_ISOREF";
            case POR_META: return "POR_META";

            case POR_EXP: return "POR_EXP";
            case POR_BLANK: return "POR_BLANK";
            case POR_POSTITS: return "POR_POSTITS";

            case POR_HYPH: return "POR_HYPH";
            case POR_HYPHSTR: return "POR_HYPHSTR";
            case POR_SOFTHYPH: return "POR_SOFTHYPH";
            case POR_SOFTHYPHSTR: return "POR_SOFTHYPHSTR";
            case POR_SOFTHYPH_COMP: return "POR_SOFTHYPH_COMP";

            case POR_FLD: return "POR_FLD";
            case POR_HIDDEN: return "POR_HIDDEN";
            case POR_QUOVADIS: return "POR_QUOVADIS";
            case POR_ERGOSUM: return "POR_ERGOSUM";
            case POR_COMBINED: return "POR_COMBINED";
            case POR_FTN: return "POR_FTN";

            case POR_FTNNUM: return "POR_FTNNUM";
            case POR_NUMBER: return "POR_NUMBER";
            case POR_BULLET: return "POR_BULLET";
            case POR_GRFNUM: return "POR_GRFNUM";

            case POR_GLUE: return "POR_GLUE";

            case POR_MARGIN: return "POR_MARGIN";

            case POR_FIX: return "POR_FIX";
            case POR_FLY: return "POR_FLY";

            case POR_TAB: return "POR_TAB";

            case POR_TABRIGHT: return "POR_TABRIGHT";
            case POR_TABCENTER: return "POR_TABCENTER";
            case POR_TABDECIMAL: return "POR_TABDECIMAL";

            case POR_TABLEFT: return "POR_TABLEFT";
            default:
                return "Unknown";
        }
    }

  public:

    explicit XmlPortionDumper( xmlTextWriterPtr some_writer ):writer( some_writer ), ofs( 0 )
    {
    }

    virtual ~ XmlPortionDumper(  )
    {
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
      */
    virtual void Text( sal_Int32 nLength,
                       sal_uInt16 nType,
                       sal_Int32 nHeight,
                       sal_Int32 nWidth) override
    {
        ofs += nLength;
        xmlTextWriterStartElement( writer, BAD_CAST( "Text" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%s", getTypeName( nType ) );
        if (nHeight > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nHeight"), "%i", (int)nHeight);
        if (nWidth > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nWidth"), "%i", (int)nWidth);

        xmlTextWriterEndElement( writer );
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
        @param nType
                type of this portion
        @param nHeight
                font size of the painted text
      */
    virtual void Special( sal_Int32 nLength,
                          const OUString & rText,
                          sal_uInt16 nType,
                          sal_Int32 nHeight,
                          sal_Int32 nWidth,
                          const SwFont* pFont ) override
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Special" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%s", getTypeName( nType ) );
        OString sText8 = OUStringToOString( rText, RTL_TEXTENCODING_UTF8 );
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "rText" ),
                                           "%s", sText8.getStr(  ) );

        if (nHeight > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nHeight"), "%i", (int)nHeight);

        if (nWidth > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nWidth"), "%i", (int)nWidth);

        if (pFont)
            pFont->dumpAsXml(writer);

        xmlTextWriterEndElement( writer );
        ofs += nLength;
    }

    virtual void LineBreak( sal_Int32 nWidth ) override
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "LineBreak" ) );
        if (nWidth > 0)
            xmlTextWriterWriteFormatAttribute( writer,
                                               BAD_CAST( "nWidth" ),
                                               "%i", ( int ) nWidth );
        xmlTextWriterEndElement( writer );
    }

    /**
      * @param nLength
      *         number of 'model string' characters to be skipped
      */
    virtual void Skip( sal_Int32 nLength ) override
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Skip" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterEndElement( writer );
        ofs += nLength;
    }

    virtual void Finish(  ) override
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Finish" ) );
        xmlTextWriterEndElement( writer );
    }

};

namespace
{
    xmlTextWriterPtr lcl_createDefaultWriter()
    {
        xmlTextWriterPtr writer = xmlNewTextWriterFilename( "layout.xml", 0 );
        xmlTextWriterStartDocument( writer, nullptr, nullptr, nullptr );
        return writer;
    }

    void lcl_freeWriter( xmlTextWriterPtr writer )
    {
        xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
    }
}

void SwFrame::dumpAsXml( xmlTextWriterPtr writer ) const
{
    bool bCreateWriter = ( nullptr == writer );
    if ( bCreateWriter )
        writer = lcl_createDefaultWriter();

    const char *name = nullptr;

    switch ( GetType(  ) )
    {
    case SwFrameType::Root:
        name = "root";
        break;
    case SwFrameType::Page:
        name = "page";
        break;
    case SwFrameType::Column:
        name = "column";
        break;
    case SwFrameType::Header:
        name = "header";
        break;
    case SwFrameType::Footer:
        name = "footer";
        break;
    case SwFrameType::FtnCont:
        name = "ftncont";
        break;
    case SwFrameType::Ftn:
        name = "ftn";
        break;
    case SwFrameType::Body:
        name = "body";
        break;
    case SwFrameType::Fly:
        name = "fly";
        break;
    case SwFrameType::Section:
        name = "section";
        break;
    case SwFrameType::Tab:
        name = "tab";
        break;
    case SwFrameType::Row:
        name = "row";
        break;
    case SwFrameType::Cell:
        name = "cell";
        break;
    case SwFrameType::Txt:
        name = "txt";
        break;
    case SwFrameType::NoTxt:
        name = "notxt";
        break;
    default: break;
    };

    if ( name != nullptr )
    {
        xmlTextWriterStartElement( writer, reinterpret_cast<const xmlChar *>(name) );

        dumpAsXmlAttributes( writer );

        if (IsRootFrame())
        {
            const SwRootFrame* pRootFrame = static_cast<const SwRootFrame*>(this);
            xmlTextWriterStartElement(writer, BAD_CAST("sfxViewShells"));
            SwView* pView = static_cast<SwView*>(SfxViewShell::GetFirst(true, checkSfxViewShell<SwView>));
            while (pView)
            {
                if (pView->GetObjectShell() == pRootFrame->GetCurrShell()->GetSfxViewShell()->GetObjectShell())
                    pView->dumpAsXml(writer);
                pView = static_cast<SwView*>(SfxViewShell::GetNext(*pView, true, checkSfxViewShell<SwView>));
            }
            xmlTextWriterEndElement(writer);
        }

        xmlTextWriterStartElement( writer, BAD_CAST( "infos" ) );
        dumpInfosAsXml( writer );
        xmlTextWriterEndElement( writer );

        // Dump Anchored objects if any
        const SwSortedObjs* pAnchored = GetDrawObjs();
        if ( pAnchored && pAnchored->size() > 0 )
        {
            xmlTextWriterStartElement( writer, BAD_CAST( "anchored" ) );

            for (SwAnchoredObject* pObject : *pAnchored)
            {
                pObject->dumpAsXml( writer );
            }

            xmlTextWriterEndElement( writer );
        }

        // Dump the children
        if ( IsTextFrame(  ) )
        {
            const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
            OUString aText = pTextFrame->GetText(  );
            for ( int i = 0; i < 32; i++ )
            {
                aText = aText.replace( i, '*' );
            }
            OString aText8 =OUStringToOString( aText,
                                                          RTL_TEXTENCODING_UTF8 );
            xmlTextWriterWriteString( writer,
                                      reinterpret_cast<const xmlChar *>(aText8.getStr(  )) );
            XmlPortionDumper pdumper( writer );
            pTextFrame->VisitPortions( pdumper );

        }
        else
        {
            dumpChildrenAsXml( writer );
        }
        xmlTextWriterEndElement( writer );
    }

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwFrame::dumpInfosAsXml( xmlTextWriterPtr writer ) const
{
    // output the Frame
    xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "left" ), "%ld", Frame().Left() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "top" ), "%ld", Frame().Top() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "width" ), "%ld", Frame().Width() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "height" ), "%ld", Frame().Height() );
    xmlTextWriterWriteAttribute(writer, BAD_CAST("mbFixSize"), BAD_CAST(OString::boolean(HasFixSize()).getStr()));
    xmlTextWriterEndElement( writer );

    // output the Prt
    xmlTextWriterStartElement( writer, BAD_CAST( "prtBounds" ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "left" ), "%ld", Prt().Left() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "top" ), "%ld", Prt().Top() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "width" ), "%ld", Prt().Width() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "height" ), "%ld", Prt().Height() );
    xmlTextWriterEndElement( writer );
}

// Hack: somehow conversion from "..." to va_list does
// bomb on two string litterals in the format.
static const char* TMP_FORMAT = "%" SAL_PRIuUINTPTR;

void SwFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "id" ), "%" SAL_PRIuUINT32, GetFrameId() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "symbol" ), "%s", BAD_CAST( typeid( *this ).name( ) ) );
    if ( GetNext( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "next" ), "%" SAL_PRIuUINT32, GetNext()->GetFrameId() );
    if ( GetPrev( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "prev" ), "%" SAL_PRIuUINT32, GetPrev()->GetFrameId() );
    if ( GetUpper( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "upper" ), "%" SAL_PRIuUINT32, GetUpper()->GetFrameId() );
    if ( GetLower( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "lower" ), "%" SAL_PRIuUINT32, GetLower()->GetFrameId() );
    if ( IsTextFrame(  ) )
    {
        const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
        const SwTextNode *pTextNode = pTextFrame->GetTextNode();
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), TMP_FORMAT, pTextNode->GetIndex() );
    }
    if (IsHeaderFrame() || IsFooterFrame())
    {
        const SwHeadFootFrame *pHeadFootFrame = static_cast<const SwHeadFootFrame*>(this);
        OUString aFormatName = pHeadFootFrame->GetFormat()->GetName();
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "fmtName" ), "%s", BAD_CAST(OUStringToOString(aFormatName, RTL_TEXTENCODING_UTF8).getStr()));
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "fmtPtr" ), "%p", pHeadFootFrame->GetFormat());
    }
}

void SwFrame::dumpChildrenAsXml( xmlTextWriterPtr writer ) const
{
    const SwFrame *pFrame = GetLower(  );
    for ( ; pFrame != nullptr; pFrame = pFrame->GetNext(  ) )
    {
        pFrame->dumpAsXml( writer );
    }
}

void SwAnchoredObject::dumpAsXml( xmlTextWriterPtr writer ) const
{
    bool bCreateWriter = ( nullptr == writer );
    if ( bCreateWriter )
        writer = lcl_createDefaultWriter();

    xmlTextWriterStartElement( writer, BAD_CAST( getElementName() ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );

    xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "left" ), "%ld", GetObjBoundRect().Left() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "top" ), "%ld", GetObjBoundRect().Top() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "width" ), "%ld", GetObjBoundRect().Width() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "height" ), "%ld", GetObjBoundRect().Height() );
    xmlTextWriterEndElement( writer );

    if (const SdrObject* pObject = GetDrawObj())
        pObject->dumpAsXml(writer);

    xmlTextWriterEndElement( writer );

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwFont::dumpAsXml(xmlTextWriterPtr writer) const
{
    xmlTextWriterStartElement(writer, BAD_CAST("pFont"));
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("color"), "%s", GetColor().AsRGBHexString().toUtf8().getStr());
    xmlTextWriterEndElement(writer);
}

void SwTextFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTextFrame*>(m_pPrecede)->GetFrameId() );
}

void SwSectionFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwSectionFrame*>( m_pPrecede )->GetFrameId() );
}

void SwTabFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTabFrame*>( m_pPrecede )->GetFrameId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
