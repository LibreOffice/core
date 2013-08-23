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

#ifndef _MIECLIP_HXX
#define _MIECLIP_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>
#include <tools/string.hxx>

class SvStream;
class SotDataObject;
class SvData;

class SFX2_DLLPUBLIC MSE40HTMLClipFormatObj
{
    SvStream* pStrm;
    OUString  sBaseURL;

public:
    MSE40HTMLClipFormatObj() : pStrm(0)         {}
    ~MSE40HTMLClipFormatObj();

//JP 31.01.2001: old interfaces
    SAL_DLLPRIVATE sal_Bool GetData( SotDataObject& );
    SAL_DLLPRIVATE sal_Bool GetData( SvData& );
//JP 31.01.2001: the new one
    SvStream* IsValid( SvStream& );

    const SvStream* GetStream() const       { return pStrm; }
          SvStream* GetStream()             { return pStrm; }
    const OUString& GetBaseURL() const      { return sBaseURL; }
};


#endif //_MIECLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
