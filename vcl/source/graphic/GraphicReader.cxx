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

#include <graphic/GraphicReader.hxx>

#include <sal/config.h>
#include <sal/log.hxx>

class ReaderData
{
public:
    Size maPreviewSize;
};

GraphicReader::GraphicReader() {}

GraphicReader::~GraphicReader() {}

void GraphicReader::DisablePreviewMode()
{
    if (mpReaderData)
        mpReaderData->maPreviewSize = Size(0, 0);
}

void GraphicReader::SetPreviewSize(const Size& rSize)
{
    if (!mpReaderData)
        mpReaderData.reset(new ReaderData);
    mpReaderData->maPreviewSize = rSize;
}

Size GraphicReader::GetPreviewSize() const
{
    Size aSize(0, 0);
    if (mpReaderData)
        aSize = mpReaderData->maPreviewSize;
    return aSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
