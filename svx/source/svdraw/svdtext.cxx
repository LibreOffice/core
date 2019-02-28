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

#include <svx/svdotext.hxx>
#include <svx/svdetc.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <libxml/xmlwriter.h>
#include <memory>

SdrText::SdrText( SdrTextObj& rObject )
: mrObject( rObject )
, mbPortionInfoChecked( false )
{
    OSL_ENSURE(&mrObject, "SdrText created without SdrTextObj (!)");
}

SdrText::~SdrText()
{
    clearWeak();
}

void SdrText::CheckPortionInfo( SdrOutliner& rOutliner )
{
    if(!mbPortionInfoChecked)
    {
        // #i102062# no action when the Outliner is the HitTestOutliner,
        // this will remove WrongList info at the OPO
        if(&rOutliner == &mrObject.getSdrModelFromSdrObject().GetHitTestOutliner())
            return;

        // TODO: optimization: we could create a BigTextObject
        mbPortionInfoChecked=true;

        if(mpOutlinerParaObject!=nullptr && rOutliner.ShouldCreateBigTextObject())
        {
            // #i102062# MemoryLeak closed
            mpOutlinerParaObject = rOutliner.CreateParaObject();
        }
    }
}

void SdrText::ReformatText()
{
    mbPortionInfoChecked=false;
    mpOutlinerParaObject->ClearPortionInfo();
}

const SfxItemSet& SdrText::GetItemSet() const
{
    return const_cast< SdrText* >(this)->GetObjectItemSet();
}

void SdrText::SetOutlinerParaObject( std::unique_ptr<OutlinerParaObject> pTextObject )
{
    assert ( !mpOutlinerParaObject || (mpOutlinerParaObject.get() != pTextObject.get()) );

    // Update HitTestOutliner
    const SdrTextObj* pTestObj(mrObject.getSdrModelFromSdrObject().GetHitTestOutliner().GetTextObj());

    if(pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject.get())
    {
        mrObject.getSdrModelFromSdrObject().GetHitTestOutliner().SetTextObj(nullptr);
    }

    mpOutlinerParaObject = std::move(pTextObject);
    mbPortionInfoChecked = false;
}

OutlinerParaObject* SdrText::GetOutlinerParaObject() const
{
    return mpOutlinerParaObject.get();
}

/** returns the current OutlinerParaObject and removes it from this instance */
std::unique_ptr<OutlinerParaObject> SdrText::RemoveOutlinerParaObject()
{
    // Update HitTestOutliner
    const SdrTextObj* pTestObj(mrObject.getSdrModelFromSdrObject().GetHitTestOutliner().GetTextObj());

    if(pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject.get())
    {
        mrObject.getSdrModelFromSdrObject().GetHitTestOutliner().SetTextObj(nullptr);
    }

    std::unique_ptr<OutlinerParaObject> pOPO = std::move(mpOutlinerParaObject);
    mbPortionInfoChecked = false;

    return pOPO;
}

void SdrText::ForceOutlinerParaObject( OutlinerMode nOutlMode )
{
    if(!mpOutlinerParaObject)
    {
        std::unique_ptr<Outliner> pOutliner(
            SdrMakeOutliner(
                nOutlMode,
                mrObject.getSdrModelFromSdrObject()));

        if(pOutliner)
        {
            Outliner& aDrawOutliner(mrObject.getSdrModelFromSdrObject().GetDrawOutliner());
            pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
            pOutliner->SetStyleSheet( 0, GetStyleSheet());
            SetOutlinerParaObject( pOutliner->CreateParaObject() );
        }
    }
}

const SfxItemSet& SdrText::GetObjectItemSet()
{
    return mrObject.GetObjectItemSet();
}

void SdrText::SetObjectItem(const SfxPoolItem& rItem)
{
    mrObject.SetObjectItem( rItem );
}

SfxStyleSheet* SdrText::GetStyleSheet() const
{
    return mrObject.GetStyleSheet();
}

void SdrText::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrText"));
    mpOutlinerParaObject->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
