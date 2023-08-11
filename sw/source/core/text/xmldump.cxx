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
#include <flyfrm.hxx>
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
#include <flyfrms.hxx>
#include <svx/svdobj.hxx>

#include "porlay.hxx"

const char* sw::PortionTypeToString(PortionType nType)
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
        case PortionType::ContentControl:
            return "PortionType::ContentControl";
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

        case PortionType::Tab:
            return "PortionType::Tab";

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
    const char *name = nullptr;

    switch ( GetType(  ) )
    {
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

        if (IsTextFrame())
        {
            const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
            sw::MergedPara const*const pMerged(pTextFrame->GetMergedPara());
            if (pMerged)
            {
                (void)xmlTextWriterStartElement( writer, BAD_CAST( "merged" ) );
                (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "paraPropsNodeIndex" ), "%" SAL_PRIdINT32, sal_Int32(pMerged->pParaPropsNode->GetIndex()) );
                for (auto const& e : pMerged->extents)
                {
                    (void)xmlTextWriterStartElement( writer, BAD_CAST( "extent" ) );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), "%" SAL_PRIdINT32, sal_Int32(e.pNode->GetIndex()) );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "start" ), "%" SAL_PRIdINT32, e.nStart );
                    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "end" ), "%" SAL_PRIdINT32, e.nEnd );
                    (void)xmlTextWriterEndElement( writer );
                }
                (void)xmlTextWriterEndElement( writer );
            }
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
            auto nTextOffset = static_cast<sal_Int32>(pTextFrame->GetOffset());
            sal_Int32 nTextLength = aText.getLength() - nTextOffset;
            if (const SwTextFrame* pTextFrameFollow = pTextFrame->GetFollow())
            {
                nTextLength = static_cast<sal_Int32>(pTextFrameFollow->GetOffset() - pTextFrame->GetOffset());
            }
            OString aText8
                = OUStringToOString(aText.subView(nTextOffset, nTextLength), RTL_TEXTENCODING_UTF8);
            (void)xmlTextWriterWriteString( writer,
                                      reinterpret_cast<const xmlChar *>(aText8.getStr(  )) );
            if (const SwParaPortion* pPara = pTextFrame->GetPara())
            {
                (void)xmlTextWriterStartElement(writer, BAD_CAST("SwParaPortion"));
                TextFrameIndex nOffset(0);
                const OUString& rText = pTextFrame->GetText();
                (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ptr"), "%p", pPara);
                const SwLineLayout* pLine = pPara;
                if (pTextFrame->IsFollow())
                {
                    nOffset += pTextFrame->GetOffset();
                }
                while (pLine)
                {
                    (void)xmlTextWriterStartElement(writer, BAD_CAST("SwLineLayout"));
                    pLine->dumpAsXmlAttributes(writer, rText, nOffset);
                    const SwLinePortion* pPor = pLine->GetFirstPortion();
                    while (pPor)
                    {
                        pPor->dumpAsXml(writer, rText, nOffset);
                        pPor = pPor->GetNextPortion();
                    }
                    (void)xmlTextWriterEndElement(writer);
                    pLine = pLine->GetNext();
                }
                (void)xmlTextWriterEndElement(writer);
            }

        }
        else
        {
            dumpChildrenAsXml( writer );
        }
        (void)xmlTextWriterEndElement( writer );
    }
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
        (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("sectionNodeIndex"), "%" SAL_PRIdINT32, pNode ? sal_Int32(pNode->GetIndex()) : -1);
    }
    if ( IsTextFrame(  ) )
    {
        const SwTextFrame *pTextFrame = static_cast<const SwTextFrame *>(this);
        const SwTextNode *pTextNode = pTextFrame->GetTextNodeFirst();
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), "%" SAL_PRIdINT32, sal_Int32(pTextNode->GetIndex()) );

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
    (void)xmlTextWriterStartElement( writer, BAD_CAST( getElementName() ) );
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("anchor-frame"), BAD_CAST(OString::number(mpAnchorFrame->GetFrameId()).getStr()));
    SwTextFrame* pAnchorCharFrame = const_cast<SwAnchoredObject*>(this)->FindAnchorCharFrame();
    if (pAnchorCharFrame)
    {
        (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("anchor-char-frame"), BAD_CAST(OString::number(pAnchorCharFrame->GetFrameId()).getStr()));
    }

    (void)xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    // don't call GetObjBoundRect(), it modifies the layout
    SwRect(GetDrawObj()->GetLastBoundRect()).dumpAsXmlAttributes(writer);
    (void)xmlTextWriterEndElement( writer );

    if (const SdrObject* pObject = GetDrawObj())
        pObject->dumpAsXml(writer);

    (void)xmlTextWriterEndElement( writer );
}

void SwFont::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, BAD_CAST("SwFont"));
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ptr"), "%p", this);
    // do not use Color::AsRGBHexString() as that omits the transparency
    (void)xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("color"), "%08" SAL_PRIxUINT32, sal_uInt32(GetColor()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("height"), BAD_CAST(OString::number(GetSize(GetActual()).Height()).getStr()));
    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("width"), BAD_CAST(OString::number(GetSize(GetActual()).Width()).getStr()));
    {
        std::stringstream ss;
        ss << GetWeight();
        (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("weight"), BAD_CAST(ss.str().c_str()));
    }
    (void)xmlTextWriterEndElement(writer);
}

void SwTextFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTextFrame*>(m_pPrecede)->GetFrameId() );

    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("offset"), BAD_CAST(OString::number(static_cast<sal_Int32>(mnOffset)).getStr()));
}

void SwFlyAtContentFrame::dumpAsXmlAttributes(xmlTextWriterPtr pWriter) const
{
    SwFlyFreeFrame::dumpAsXmlAttributes(pWriter);

    if (m_pFollow != nullptr)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("follow"),
            BAD_CAST(OString::number(m_pFollow->GetFrame().GetFrameId()).getStr()));
    }
    if (m_pPrecede != nullptr)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("precede"),
            BAD_CAST(OString::number(m_pPrecede->GetFrame().GetFrameId()).getStr()));
    }
}

void SwSectionFrame::dumpAsXmlAttributes( xmlTextWriterPtr writer ) const
{
    SwFrame::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwSectionFrame*>( m_pPrecede )->GetFrameId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
