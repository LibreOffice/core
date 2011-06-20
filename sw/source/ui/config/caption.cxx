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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <osl/diagnose.h>

#include "numrule.hxx"
#include "caption.hxx"

#define VERSION_01      1
#define CAPTION_VERSION VERSION_01

InsCaptionOpt::InsCaptionOpt(const SwCapObjType eType, const SvGlobalName* pOleId) :
    bUseCaption(sal_False),
    eObjType(eType),
    nNumType(SVX_NUM_ARABIC),
    sNumberSeparator(RTL_CONSTASCII_USTRINGPARAM((". "))),
    nPos(1),
    nLevel(0),
    sSeparator( String::CreateFromAscii( ": " ) ),
    bIgnoreSeqOpts(sal_False),
    bCopyAttributes(sal_False)
{
    if (pOleId)
        aOleId = *pOleId;
}

InsCaptionOpt::InsCaptionOpt(const InsCaptionOpt& rOpt)
{
    *this = rOpt;
}

InsCaptionOpt::~InsCaptionOpt()
{
}

InsCaptionOpt& InsCaptionOpt::operator=( const InsCaptionOpt& rOpt )
{
    bUseCaption = rOpt.bUseCaption;
    eObjType = rOpt.eObjType;
    aOleId = rOpt.aOleId;
    sCategory = rOpt.sCategory;
    nNumType = rOpt.nNumType;
    sNumberSeparator = rOpt.sNumberSeparator;
    sCaption = rOpt.sCaption;
    nPos = rOpt.nPos;
    nLevel = rOpt.nLevel;
    sSeparator = rOpt.sSeparator;
    bIgnoreSeqOpts = rOpt.bIgnoreSeqOpts;
    sCharacterStyle = rOpt.sCharacterStyle;
    bCopyAttributes = rOpt.bCopyAttributes;

    return *this;
}

sal_Bool InsCaptionOpt::operator==( const InsCaptionOpt& rOpt ) const
{
    return (eObjType == rOpt.eObjType &&
            aOleId == rOpt.aOleId); // So that identical Ole-IDs can't be added multiple
                                    // times, don't compare against anything else.


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
