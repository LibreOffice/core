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

#include <formatflysplit.hxx>

#include <libxml/xmlwriter.h>

SwFormatFlySplit::SwFormatFlySplit(bool bSplit)
    : SfxBoolItem(RES_FLY_SPLIT, bSplit)
{
    // Once this pool item is true, a floating table (text frame + table inside it) is meant to
    // split across multiple pages.
    //
    // The layout representation is the following:
    //
    // - We assume that the anchor type is at-para for such fly frames.
    //
    // - We also assume that AutoSize is true for such fly frames.
    //
    // - SwFlyAtContentFrame derives from SwFlowFrame to be able to split in general.
    //
    // - Both the master fly and the follow flys need an anchor. At the same time, we want all text
    // of the anchor frame to be wrapped around the last follow fly frame, for Word compatibility.
    // These are solved by splitting the anchor frame as many times as needed, always at
    // TextFrameIndex 0.
    //
    // - The vertical offset is only considered on the master fly, all the follow flys ignore it, so
    // they start at the top of a next page.
}

SwFormatFlySplit* SwFormatFlySplit::Clone(SfxItemPool*) const
{
    return new SwFormatFlySplit(*this);
}

void SwFormatFlySplit::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatFlySplit"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"),
                                      BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(OString::boolean(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
