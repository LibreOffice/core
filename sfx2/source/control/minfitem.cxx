/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: minfitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:17:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#pragma hdrstop
#endif

#include "minfitem.hxx"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(SfxMacroInfoItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxMacroInfoItem::SfxMacroInfoItem(
    USHORT nWhichId,        // Slot-ID
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


