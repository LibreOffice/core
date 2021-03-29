/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <frame.hxx>
#include <frmfmt.hxx>
#include <ftnfrm.hxx>
#include <sectfrm.hxx>
#include <tabfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <cellfrm.hxx>
#include <hffrm.hxx>
#include <rootfrm.hxx>
#include <ndtxt.hxx>
#include <sortedobjs.hxx>
#include <swfont.hxx>
#include <txttypes.hxx>
#include <anchoredobject.hxx>
#include <libxml/xmlwriter.h>
#include <SwPortionHandler.hxx>
#include <view.hxx>
#include <svx/svdobj.hxx>

namespace {

class XmlPortionDumper:public SwPortionHandler
{
  private:
      xmlTextWriterPtr m_Writer;
      TextFrameIndex m_Ofs;
      const OUString& m_rText;
      OUString m_aLine;

      static const char* getTypeName(PortionType nType)
      {
          switch (nType)
          {
              case PortionType::NONE:
                  return "PortionType::NONE";
              case PortionType::FlyCnt:
                  return "PortionType::FlyCnt";

              case PortionType::Hole:
                  return "PortionType::Hole";
              case PortionType::TempEnd:
                  return "PortionType::TempEnd";
              case PortionType::Break:
                  return "PortionType::Break";
              case PortionType::Kern:
                  return "PortionType::Kern";
              case PortionType::Arrow:
                  return "PortionType::Arrow";
              case PortionType::Multi:
                  return "PortionType::Multi";
              case PortionType::HiddenText:
                  return "PortionType::HiddenText";
              case PortionType::ControlChar:
                  return "PortionType::ControlChar";
              case PortionType::Bookmark:
                  return "PortionType::Bookmark";

              case PortionType::Text:
                  return "PortionType::Text";
              case PortionType::Lay:
                  return "PortionType::Lay";
              case PortionType::Para:
                  return "PortionType::Para";
              case PortionType::Hanging:
                  return "PortionType::Hanging";

              case PortionType::Drop:
                  return "PortionType::Drop";
              case PortionType::Tox:
                  return "PortionType::Tox";
              case PortionType::IsoTox:
                  return "PortionType::IsoTox";
              case PortionType::Ref:
                  return "PortionType::Ref";
              case PortionType::IsoRef:
                  return "PortionType::IsoRef";
              case PortionType::Meta:
                  return "PortionType::Meta";
              case PortionType::FieldMark:
                  return "PortionType::FieldMark";
              case PortionType::FieldFormCheckbox:
                  return "PortionType::FieldFormCheckbox";
              case PortionType::InputField:
                  return "PortionType::InputField";

              case PortionType::Expand:
                  return "PortionType::Expand";
              case PortionType::Blank:
                  return "PortionType::Blank";
              case PortionType::PostIts:
                  return "PortionType::PostIts";

              case PortionType::Hyphen:
                  return "PortionType::Hyphen";
              case PortionType::HyphenStr:
                  return "PortionType::HyphenStr";
              case PortionType::SoftHyphen:
                  return "PortionType::SoftHyphen";
              case PortionType::SoftHyphenStr:
                  return "PortionType::SoftHyphenStr";
              case PortionType::SoftHyphenComp:
                  return "PortionType::SoftHyphenComp";

              case PortionType::Field:
                  return "PortionType::Field";
              case PortionType::Hidden:
                  return "PortionType::Hidden";
              case PortionType::QuoVadis:
                  return "PortionType::QuoVadis";
              case PortionType::ErgoSum:
                  return "PortionType::ErgoSum";
              case PortionType::Combined:
                  return "PortionType::Combined";
              case PortionType::Footnote:
                  return "PortionType::Footnote";

              case PortionType::FootnoteNum:
                  return "PortionType::FootnoteNum";
              case PortionType::Number:
                  return "PortionType::Number";
              case PortionType::Bullet:
                  return "PortionType::Bullet";
              case PortionType::GrfNum:
                  return "PortionType::GrfNum";

              case PortionType::Glue:
                  return "PortionType::Glue";

              case PortionType::Margin:
                  return "PortionType::Margin";

              case PortionType::Fix:
                  return "PortionType::Fix";
              case PortionType::Fly:
                  return "PortionType::Fly";

              case PortionType::Table:
                  return "PortionType::Table";

              case PortionType::TabRight:
                  return "PortionType::TabRight";
              case PortionType::TabCenter:
                  return "PortionType::TabCenter";
              case PortionType::TabDecimal:
                  return "PortionType::TabDecimal";

              case PortionType::TabLeft:
                  return "PortionType::TabLeft";
              default:
                  return "Unknown";
          }
    }

  public:
      explicit XmlPortionDumper(xmlTextWriterPtr some_writer, const OUString& rText)
          : m_Writer(some_writer)
          , m_Ofs(0)
          , m_rText(rText)
      {
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
      */
    virtual void Text( TextFrameIndex nLength,
                       PortionType nType,
                       sal_Int32 nHeight,
                       sal_Int32 nWidth) override
    {
        (void)xmlTextWriterStartElement(m_Writer, BAD_CAST("Text"));
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nLength"), "%i",
                                                static_cast<int>(static_cast<sal_Int32>(nLength)));
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nType"), "%s",
                                                getTypeName(nType));
        if (nHeight > 0)
            (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nHeight"), "%i",
                                                    static_cast<int>(nHeight));
        if (nWidth > 0)
            (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nWidth"), "%i",
                                                    static_cast<int>(nWidth));
        if (nLength > TextFrameIndex(0))
            (void)xmlTextWriterWriteAttribute(
                m_Writer, BAD_CAST("Portion"),
                BAD_CAST(m_rText.copy(sal_Int32(m_Ofs), sal_Int32(nLength)).toUtf8().getStr()));

        (void)xmlTextWriterEndElement(m_Writer);
        m_aLine += m_rText.subView(sal_Int32(m_Ofs), sal_Int32(nLength));
        m_Ofs += nLength;
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
    virtual void Special( TextFrameIndex nLength,
                          const OUString & rText,
                          PortionType nType,
                          sal_Int32 nHeight,
                          sal_Int32 nWidth,
                          const SwFont* pFont ) override
    {
        (void)xmlTextWriterStartElement(m_Writer, BAD_CAST("Special"));
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nLength"), "%i",
                                                static_cast<int>(static_cast<sal_Int32>(nLength)));
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nType"), "%s",
                                                getTypeName(nType));
        OString sText8 = OUStringToOString( rText, RTL_TEXTENCODING_UTF8 );
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("rText"), "%s", sText8.getStr());

        if (nHeight > 0)
            (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nHeight"), "%i",
                                                    static_cast<int>(nHeight));

        if (nWidth > 0)
            (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nWidth"), "%i",
                                                    static_cast<int>(nWidth));

        if (pFont)
            pFont->dumpAsXml(m_Writer);

        (void)xmlTextWriterEndElement(m_Writer);
        m_aLine += rText;
        m_Ofs += nLength;
    }

    virtual void LineBreak( sal_Int32 nWidth ) override
    {
        (void)xmlTextWriterStartElement(m_Writer, BAD_CAST("LineBreak"));
        if (nWidth > 0)
            (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nWidth"), "%i",
                                                    static_cast<int>(nWidth));
        if (!m_aLine.isEmpty())
        {
            (void)xmlTextWriterWriteAttribute(m_Writer, BAD_CAST("Line"),
                                              BAD_CAST(m_aLine.toUtf8().getStr()));
            m_aLine.clear();
        }
        (void)xmlTextWriterEndElement(m_Writer);
    }

    /**
      * @param nLength
      *         number of 'model string' characters to be skipped
      */
    virtual void Skip( TextFrameIndex nLength ) override
    {
        (void)xmlTextWriterStartElement(m_Writer, BAD_CAST("Skip"));
        (void)xmlTextWriterWriteFormatAttribute(m_Writer, BAD_CAST("nLength"), "%i",
                                                static_cast<int>(static_cast<sal_Int32>(nLength)));
        (void)xmlTextWriterEndElement(m_Writer);
        m_Ofs += nLength;
    }

    virtual void Finish(  ) override
    {
        (void)xmlTextWriterStartElement(m_Writer, BAD_CAST("Finish"));
        (void)xmlTextWriterEndElement(m_Writer);
    }

};

    xmlTextWriterPtr lcl_createDefaultWriter()
    {
        xmlTextWriterPtr writer = xmlNewTextWriterFilename( "layout.xml", 0 );
        xmlTextWriterSetIndent(writer,1);
        (void)xmlTextWriterSetIndentString(writer, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument( writer, nullptr, nullptr, nullptr );
        return writer;
    }

    void lcl_freeWriter( xmlTextWriterPtr writer )
    {
        (void)xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
    }
}

void SwFrame::dumpTopMostAsXml(xmlTextWriterPtr writer) const
{
    const SwFrame* pFrame = this;
    while (pFrame->GetUpper())
    {
        pFrame = pFrame->GetUpper();
    }

    pFrame->dumpAsXml(writer);
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
    }

    if ( name != nullptr )
    {
        (void)xmlTextWriterStartElement( writer, reinterpret_cast<const xmlChar *>(name) );

        dumpAsXmlAttributes( writer );

        if (IsRootFrame())
        {
            const SwRootFrame* pRootFrame = static_cast<const SwRootFrame*>(this);
            (void)xmlTextWriterStartElement(writer, BAD_CAST("sfxViewShells"));
            SwView* pView = static_cast<SwView*>(SfxViewShell::GetFirst(true, checkSfxViewShell<SwView>));
            while (pView)
            {
                if (pRootFrame->GetCurrShell()->GetSfxViewShell() && pView->GetObjectShell() == pRootFrame->GetCurrShell()->GetSfxViewShell()->GetObjectShell())
                    pView->dumpAsXml(writer);
                pView = static_cast<SwView*>(SfxViewShell::GetNext(*pView, true, checkSfxViewShell<SwView>));
            }
            (void)xmlTextWriterEndElement(writer);
        }

        if (IsPageFrame())
        {
            const SwPageFrame* pPageFrame = static_cast<const SwPageFrame*>(this);
            (void)xmlTextWriterStartElement(writer, BAD_CAST("page_status"));
            (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("ValidFlyLayout"), BAD_CAST(OString::boolean(!pPageFrame->IsInvalidFlyLayout()).getStr()));
            (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("ValidFlyContent"), BAD_CAST(OString::boolean(!pPageFrame->IsInvalidFlyContent()).getStr()));
            (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("ValidFlyInCnt"), BAD_CAST(OString::boolean(!pPageFrame->IsInvalidFlyInCnt()).getStr()));
            (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("ValidLayout"), BAD_CAST(OString::boolean(!pPageFrame->IsInvalidLayout()).getStr()));
            (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("ValidContent"), BAD_CAST(OString::boolean(!pPageFrame->IsInvalidContent()).getStr()));
            (void)xmlTextWriterEndElement(writer);
            (void)xmlTextWriterStartElement(writer, BAD_CAST("page_info"));
            (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("phyNum"), "%d", pPageFrame->GetPhyPageNum());
            (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("virtNum"), "%d", pPageFrame->GetVirtPageNum());
            OUString aFormatName = pPageFrame->GetPageDesc()->GetName();
            (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST("pageDesc"), "%s", BAD_CAST(OUStringToOString(aFormatName, RTL_TEXTENCODING_UTF8).getStr()));
            (void)xmlTextWriterEndElement(writer);
        }

        if (IsTextFrame())
        {
            const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
            sw::MergedPara const*const pMerged(pTextFrame->GetMergedPara());
            if (pMerged)
            {
                (void)xmlTextWriterStartElement( writer, BAD_CAST( "merged" ) );
                (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "paraPropsNodeIndex" ), "%" SAL_PRIuUINTPTR, pMerged->pParaPropsNode->GetIndex() );
                for (auto const& e : pMerged->extents)
                {
                    (void)xmlTextWriterStartElement( writer, BAD_CAST( "extent" ) );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), "%" SAL_PRIuUINTPTR, e.pNode->GetIndex() );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "start" ), "%" SAL_PRIdINT32, e.nStart );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "end" ), "%" SAL_PRIdINT32, e.nEnd );
                    (void)xmlTextWriterEndElement( writer );
                }
                (void)xmlTextWriterEndElement( writer );
            }
        }

        if (IsCellFrame())
        {
            SwCellFrame const* pCellFrame(static_cast<SwCellFrame const*>(this));
            (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "rowspan" ), "%ld", pCellFrame->GetLayoutRowSpan() );
        }

        (void)xmlTextWriterStartElement( writer, BAD_CAST( "infos" ) );
        dumpInfosAsXml( writer );
        (void)xmlTextWriterEndElement( writer );

        // Dump Anchored objects if any
        const SwSortedObjs* pAnchored = GetDrawObjs();
        if ( pAnchored && pAnchored->size() > 0 )
        {
            (void)xmlTextWriterStartElement( writer, BAD_CAST( "anchored" ) );

            for (SwAnchoredObject* pObject : *pAnchored)
            {
                pObject->dumpAsXml( writer );
            }

            (void)xmlTextWriterEndElement( writer );
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
            (void)xmlTextWriterWriteString( writer,
                                      reinterpret_cast<const xmlChar *>(aText8.getStr(  )) );
            XmlPortionDumper pdumper( writer, aText );
            pTextFrame->VisitPortions( pdumper );

        }
        else
        {
            dumpChildrenAsXml( writer );
        }
        (void)xmlTextWriterEndElement( writer );
    }

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwFrame::dumpInfosAsXml( xmlTextWriterPtr writer ) const
{
    // output the Frame
    (void)xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    getFrameArea().dumpAsXmlAttributes(writer);
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("mbFixSize"), BAD_CAST(OString::boolean(HasFixSize()).getStr()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("mbFrameAreaPositionValid"), BAD_CAST(OString::boolean(isFrameAreaPositionValid()).getStr()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("mbFrameAreaSizeValid"), BAD_CAST(OString::boolean(isFrameAreaSizeValid()).getStr()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("mbFramePrintAreaValid"), BAD_CAST(OString::boolean(isFramePrintAreaValid()).getStr()));
    (void)xmlTextWriterEndElement( writer );

    // output the print area
    (void)xmlTextWriterStartElement( writer, BAD_CAST( "prtBounds" ) );
    getFramePrintArea().dumpAsXmlAttributes(writer);
    (void)xmlTextWriterEndElement( writer );
}

// Hack: somehow conversion from "..." to va_list does
// bomb on two string literals in the format.
const char* const TMP_FORMAT = "%" SAL_PRIuUINTPTR;

void SwFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "id" ), "%" SAL_PRIuUINT32, GetFrameId() );
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "symbol" ), "%s", BAD_CAST( typeid( *this ).name( ) ) );
    if ( GetNext( ) )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "next" ), "%" SAL_PRIuUINT32, GetNext()->GetFrameId() );
    if ( GetPrev( ) )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "prev" ), "%" SAL_PRIuUINT32, GetPrev()->GetFrameId() );
    if ( GetUpper( ) )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "upper" ), "%" SAL_PRIuUINT32, GetUpper()->GetFrameId() );
    if ( GetLower( ) )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "lower" ), "%" SAL_PRIuUINT32, GetLower()->GetFrameId() );
    if (IsFootnoteFrame())
    {
        SwFootnoteFrame const*const pFF(static_cast<SwFootnoteFrame const*>(this));
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST("ref"), "%" SAL_PRIuUINT32, pFF->GetRef()->GetFrameId() );
        if (pFF->GetMaster())
            (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST("master"), "%" SAL_PRIuUINT32, pFF->GetMaster()->GetFrameId() );
        if (pFF->GetFollow())
            (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST("follow"), "%" SAL_PRIuUINT32, pFF->GetFollow()->GetFrameId() );
    }
    if (IsSctFrame())
    {
        SwSectionFrame const*const pFrame(static_cast<SwSectionFrame const*>(this));
        SwSectionNode const*const pNode(pFrame->GetSection() ? pFrame->GetSection()->GetFormat()->GetSectionNode() : nullptr);
        (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("sectionNodeIndex"), TMP_FORMAT, pNode ? pNode->GetIndex() : -1);
    }
    if ( IsTextFrame(  ) )
    {
        const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
        const SwTextNode *pTextNode = pTextFrame->GetTextNodeFirst();
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), TMP_FORMAT, pTextNode->GetIndex() );

        OString aMode = "Horizontal";
        if (IsVertLRBT())
        {
            aMode = "VertBTLR";
        }
        else if (IsVertLR())
        {
            aMode = "VertLR";
        }
        else if (IsVertical())
        {
            aMode = "Vertical";
        }
        (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("WritingMode"), BAD_CAST(aMode.getStr()));
    }
    if (IsHeaderFrame() || IsFooterFrame())
    {
        const SwHeadFootFrame *pHeadFootFrame = static_cast<const SwHeadFootFrame*>(this);
        OUString aFormatName = pHeadFootFrame->GetFormat()->GetName();
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "fmtName" ), "%s", BAD_CAST(OUStringToOString(aFormatName, RTL_TEXTENCODING_UTF8).getStr()));
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "fmtPtr" ), "%p", pHeadFootFrame->GetFormat());
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

    (void)xmlTextWriterStartElement( writer, BAD_CAST( getElementName() ) );
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );

    (void)xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    GetObjBoundRect().dumpAsXmlAttributes(writer);
    (void)xmlTextWriterEndElement( writer );

    if (const SdrObject* pObject = GetDrawObj())
        pObject->dumpAsXml(writer);

    (void)xmlTextWriterEndElement( writer );

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwFont::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, BAD_CAST("SwFont"));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ptr"), "%p", this);
    // do not use Color::AsRGBHexString() as that omits the transparency
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("color"), "%08" SAL_PRIxUINT32, sal_uInt32(GetColor()));
    (void)xmlTextWriterEndElement(writer);
}

void SwTextFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTextFrame*>(m_pPrecede)->GetFrameId() );
}

void SwSectionFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwSectionFrame*>( m_pPrecede )->GetFrameId() );
}

void SwTabFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTabFrame*>( m_pPrecede )->GetFrameId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
