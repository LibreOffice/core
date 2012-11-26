/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include "sfx2/minfitem.hxx"

// STATIC DATA -----------------------------------------------------------
IMPL_POOLITEM_FACTORY(SfxMacroInfoItem)

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


