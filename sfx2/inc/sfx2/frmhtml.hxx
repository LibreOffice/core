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

#ifndef _FRMHTML_HXX
#define _FRMHTML_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxhtml.hxx>

class SfxFrameHTMLParser;
class SfxMedium;
namespace svtools { class AsynchronLink; }

class SFX2_DLLPUBLIC SfxFrameHTMLParser : public SfxHTMLParser
{
protected:
   SfxFrameHTMLParser( SvStream& rStream, sal_Bool bIsNewDoc=sal_True, SfxMedium *pMediumPtr=0 ):
   SfxHTMLParser( rStream, bIsNewDoc, pMediumPtr ) {};

public:
    // These methods can also be used by other parsers.
    static void ParseFrameOptions(SfxFrameDescriptor*, const HTMLOptions&, const String& );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
