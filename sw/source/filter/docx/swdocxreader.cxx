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

#include "swdocxreader.hxx"

#include <swerror.h>

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportDOCX()
{
    return new SwDOCXReader;
}

sal_uLong SwDOCXReader::Read( SwDoc& /* rDoc */, const OUString& /* rBaseURL */, SwPaM& /* rPaM */, const OUString& /* FileName */ )
{
    return ERR_SWG_READ_ERROR;
}

int SwDOCXReader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

bool SwDOCXReader::HasGlossaries() const
{
    // TODO
    return true;
}

bool SwDOCXReader::ReadGlossaries( SwTextBlocks& /* rBlocks */, bool /* bSaveRelFiles */ ) const
{
    // TODO
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
