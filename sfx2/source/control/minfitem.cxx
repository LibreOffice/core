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
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include "sfx2/minfitem.hxx"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(SfxMacroInfoItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxMacroInfoItem::SfxMacroInfoItem(
    sal_uInt16 nWhichId,        // Slot-ID
    const BasicManager* pMgr,
    const String &rLibName,
    const String &rModuleName,
    const String &rMethodName,
    const String &rComment) :
    SfxPoolItem(nWhichId),
    pBasicManager(pMgr),
    aLibName(rLibName),
    aModuleName(rModuleName),
    aMethodName(rMethodName),
    aCommentText(rComment)
{
}

// -----------------------------------------------------------------------

// copy ctor

SfxMacroInfoItem::SfxMacroInfoItem(const SfxMacroInfoItem& rCopy):
    SfxPoolItem(rCopy),
    pBasicManager(rCopy.pBasicManager),
    aLibName(rCopy.aLibName),
    aModuleName(rCopy.aModuleName),
    aMethodName(rCopy.aMethodName),
    aCommentText(rCopy.aCommentText)
{
}

// -----------------------------------------------------------------------

// op ==

int SfxMacroInfoItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
            pBasicManager == ((const SfxMacroInfoItem&)rCmp).pBasicManager &&
            aLibName == ((const SfxMacroInfoItem&)rCmp).aLibName &&
            aModuleName == ((const SfxMacroInfoItem&)rCmp).aModuleName &&
            aMethodName == ((const SfxMacroInfoItem&)rCmp).aMethodName &&
            aCommentText == ((const SfxMacroInfoItem&)rCmp).aCommentText;
}

// -----------------------------------------------------------------------

SfxPoolItem *SfxMacroInfoItem::Clone( SfxItemPool *) const
{
    return new SfxMacroInfoItem(*this);
}

// -----------------------------------------------------------------------

String SfxMacroInfoItem::GetQualifiedName() const
{
    String aMacroName = aLibName;
    aMacroName += '.';
    aMacroName += aModuleName;
    aMacroName += '.';
    aMacroName += aMethodName;
    return aMacroName;
}


