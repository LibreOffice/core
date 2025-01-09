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

#include <formatwraptextatflystart.hxx>

#include <libxml/xmlwriter.h>

SwFormatWrapTextAtFlyStart::SwFormatWrapTextAtFlyStart(bool bAtStart)
    : SfxBoolItem(RES_WRAP_TEXT_AT_FLY_START, bAtStart)
{
    // Once this pool item is true, the text from the anchor text of the fly wraps an all pages, not
    // only on the last page of the fly chain.
}

SwFormatWrapTextAtFlyStart* SwFormatWrapTextAtFlyStart::Clone(SfxItemPool*) const
{
    return new SwFormatWrapTextAtFlyStart(*this);
}

void SwFormatWrapTextAtFlyStart::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatWrapTextAtFlyStart"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"),
                                      BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(OString::boolean(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
