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
    }
    return "Unknown";
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
