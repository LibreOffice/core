/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    String sBaseURL;

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
    const String& GetBaseURL() const        { return sBaseURL; }
};


#endif //_MIECLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
