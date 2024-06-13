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

#include <fmtrfmrk.hxx>

#include <libxml/xmlwriter.h>

#include <hintids.hxx>
#include <hints.hxx>
#include <txtrfmrk.hxx>
#include <unorefmark.hxx>
#include <utility>
#include <sfx2/viewsh.hxx>
#include <tools/json_writer.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <translatehelper.hxx>
#include <wrtsh.hxx>

SwFormatRefMark::~SwFormatRefMark( )
{
}

SwFormatRefMark::SwFormatRefMark( OUString aName )
    : SfxPoolItem(RES_TXTATR_REFMARK)
    , sw::BroadcastingModify()
    , m_pTextAttr(nullptr)
    , m_aRefName(std::move(aName))
{
    setNonShareable();
}

SwFormatRefMark::SwFormatRefMark( const SwFormatRefMark& rAttr )
    : SfxPoolItem(RES_TXTATR_REFMARK)
    , sw::BroadcastingModify()
    , m_pTextAttr(nullptr)
    , m_aRefName(rAttr.m_aRefName)
{
    setNonShareable();
}

void SwFormatRefMark::SetXRefMark(rtl::Reference<SwXReferenceMark> const& xMark)
{ m_wXReferenceMark = xMark.get(); }

bool SwFormatRefMark::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return m_aRefName == static_cast<const SwFormatRefMark&>(rAttr).m_aRefName;
}

SwFormatRefMark* SwFormatRefMark::Clone( SfxItemPool* ) const
{
    return new SwFormatRefMark( *this );
}

void SwFormatRefMark::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CallSwClientNotify(rHint);
    if(RES_REMOVE_UNO_OBJECT == pLegacy->GetWhich())
        SetXRefMark(nullptr);
}

void SwFormatRefMark::InvalidateRefMark()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<sw::BroadcastingModify&>(*this)); // cast to base class (void*)
    CallSwClientNotify(sw::LegacyModifyHint(&item, &item));
}

void SwFormatRefMark::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatRefMark"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("m_pTextAttr"), "%p", m_pTextAttr);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("ref-name"),
                                      BAD_CAST(m_aRefName.toUtf8().getStr()));
    SfxPoolItem::dumpAsXml(pWriter);


    (void)xmlTextWriterEndElement(pWriter);
}

// attribute for content references in the text

SwTextRefMark::SwTextRefMark(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 const nStartPos,
    sal_Int32 const*const pEnd)
    : SwTextAttr(rAttr, nStartPos)
    , SwTextAttrEnd( rAttr, nStartPos, nStartPos )
    , m_pTextNode( nullptr )
    , m_pEnd( nullptr )
{
    SwFormatRefMark& rSwFormatRefMark(static_cast<SwFormatRefMark&>(GetAttr()));
    rSwFormatRefMark.m_pTextAttr = this;
    if ( pEnd )
    {
        m_nEnd = *pEnd;
        m_pEnd = & m_nEnd;
    }
    else
    {
        SetHasDummyChar(true);
    }
    SetDontMoveAttr( true );
    SetOverlapAllowedAttr( true );
  /* FIXME: Setting the DontExpand flag would solve tdf#81720,
   * but old behavior was restored due to regressions; see tdf#157287.
   * After applying a proper fix, remember to restore testDontExpandRefmark!
   *SetDontExpand( true );  // like hyperlinks, reference markers shouldn't expand
   *SetLockExpandFlag( true ); // protect the flag
   */
}

SwTextRefMark::~SwTextRefMark()
{
    if (!comphelper::LibreOfficeKit::isActive() || GetTextNode().GetDoc().IsClipBoard())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
        return;

    OUString fieldCommand = GetRefMark().GetRefName();
    tools::JsonWriter aJson;
    aJson.put("commandName", ".uno:DeleteField");
    aJson.put("success", true);
    {
        auto result = aJson.startNode("result");
        aJson.put("DeleteField", fieldCommand);
    }

    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());
}

const sal_Int32* SwTextRefMark::GetEnd() const
{
    return m_pEnd;
}

void SwTextRefMark::SetEnd(sal_Int32 n)
{
    *m_pEnd = n;
    if (m_pHints)
        m_pHints->EndPosChanged();
}

void SwTextRefMark::UpdateFieldContent(SwDoc* pDoc, SwWrtShell& rWrtSh, OUString aContent)
{
    if (!this->End())
    {
        return;
    }

    // Insert markers to remember where the paste positions are.
    const SwTextNode& rTextNode = this->GetTextNode();
    SwPaM aMarkers(SwPosition(rTextNode, *this->End()));
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
  /* FIXME: see above re: expanding behavior
   *this->SetLockExpandFlag(false);
   *this->SetDontExpand(false);
   */
    if (rIDCO.InsertString(aMarkers, u"XY"_ustr))
    {
        SwPaM aPasteEnd(SwPosition(rTextNode, *this->End()));
        aPasteEnd.Move(fnMoveBackward, GoInContent);

        // Paste HTML content.
        SwPaM* pCursorPos = rWrtSh.GetCursor();
        *pCursorPos = aPasteEnd;
        SwTranslateHelper::PasteHTMLToPaM(rWrtSh, pCursorPos, aContent.toUtf8());

        // Update the refmark to point to the new content.
        sal_Int32 nOldStart = this->GetStart();
        sal_Int32 nNewStart = *this->End();
        // First grow it to include text till the end of the paste position.
        this->SetEnd(aPasteEnd.GetPoint()->GetContentIndex());
        // Then shrink it to only start at the paste start: we know that the refmark was
        // truncated to the paste start, as the refmark has to stay inside a single text node
        this->SetStart(nNewStart);
        rTextNode.GetSwpHints().SortIfNeedBe();
        SwPaM aEndMarker(*aPasteEnd.GetPoint());
        aEndMarker.SetMark();
        aEndMarker.GetMark()->AdjustContent(1);
        SwPaM aStartMarker(SwPosition(rTextNode, nOldStart), SwPosition(rTextNode, nNewStart));

        // Remove markers. The start marker includes the old content as well.
        rIDCO.DeleteAndJoin(aStartMarker);
        rIDCO.DeleteAndJoin(aEndMarker);
    }
    // Restore flags.
  /* FIXME: see above re: expanding behavior
   *this->SetDontExpand(true);
   *this->SetLockExpandFlag(true);
   */
}


void SwTextRefMark::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextRefMark"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    SwTextAttr::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
