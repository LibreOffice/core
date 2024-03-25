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


#include <ContentNode.hxx>
#include <editeng/tstpitem.hxx>
#include "impedit.hxx"

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>

#include <memory>
#include <set>

ContentNode::ContentNode( SfxItemPool& rPool )
    : maContentAttribs( rPool )
{
}

ContentNode::ContentNode( const OUString& rStr, const ContentAttribs& rContentAttribs )
    : maString(rStr)
    , maContentAttribs(rContentAttribs)
{
}

void ContentNode::ExpandAttribs( sal_Int32 nIndex, sal_Int32 nNew )
{
    if ( !nNew )
        return;

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
#endif

    // Since features are treated differently than normal character attributes,
    // but can also affect the order of the start list.    // In every if ...,  in the next (n) opportunities due to bFeature or
    // an existing special case, must (n-1) opportunities be provided with
    // bResort. The most likely possibility receives no bResort, so that is
    // not sorted anew when all attributes are the same.
    bool bResort = false;
    bool bExpandedEmptyAtIndexNull = false;

    std::size_t nAttr = 0;
    CharAttribList::AttribsType& rAttribs = maCharAttribList.GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
    while ( pAttrib )
    {
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Move all attributes behind the insertion point...
            if ( pAttrib->GetStart() > nIndex )
            {
                pAttrib->MoveForward( nNew );
            }
            // 0: Expand empty attribute, if at insertion point
            else if ( pAttrib->IsEmpty() )
            {
                // Do not check Index, an empty one could only be there
                // When later checking it anyhow:
                //   Special case: Start == 0; AbsLen == 1, nNew = 1
                // => Expand, because of paragraph break!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
//              if ( pAttrib->GetStart() == nIndex )
                pAttrib->Expand( nNew );
                bResort = true;
                if ( pAttrib->GetStart() == 0 )
                    bExpandedEmptyAtIndexNull = true;
            }
            // 1: Attribute starts before, goes to index ...
            else if ( pAttrib->GetEnd() == nIndex ) // Start must be before
            {
                // Only expand when there is no feature
                // and if not in exclude list!
                // Otherwise, a UL will go on until a new ULDB, expanding both
//              if ( !pAttrib->IsFeature() && !rExclList.FindAttrib( pAttrib->Which() ) )
                if ( !pAttrib->IsFeature() && !maCharAttribList.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
                {
                    if ( !pAttrib->IsEdge() )
                        pAttrib->Expand( nNew );
                }
                else
                    bResort = true;
            }
            // 2: Attribute starts before, goes past the Index...
            else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                DBG_ASSERT( !pAttrib->IsFeature(), "Large Feature?!" );
                pAttrib->Expand( nNew );
            }
            // 3: Attribute starts on index...
            else if ( pAttrib->GetStart() == nIndex )
            {
                if ( pAttrib->IsFeature() )
                {
                    pAttrib->MoveForward( nNew );
                    bResort = true;
                }
                else
                {
                    bool bExpand = false;
                    if ( nIndex == 0 )
                    {
                        bExpand = true;
                        if( bExpandedEmptyAtIndexNull )
                        {
                            // Check if this kind of attribute was empty and expanded here...
                            sal_uInt16 nW = pAttrib->GetItem()->Which();
                            for ( std::size_t nA = 0; nA < nAttr; nA++ )
                            {
                                const EditCharAttrib& r = *maCharAttribList.GetAttribs()[nA];
                                if ( ( r.GetStart() == 0 ) && ( r.GetItem()->Which() == nW ) )
                                {
                                    bExpand = false;
                                    break;
                                }
                            }

                        }
                    }
                    if ( bExpand )
                    {
                        pAttrib->Expand( nNew );
                        bResort = true;
                    }
                    else
                    {
                        pAttrib->MoveForward( nNew );
                    }
                }
            }
        }

        if ( pAttrib->IsEdge() )
            pAttrib->SetEdge(false);

        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribute distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len() ), "Expand: Attribute larger than paragraph!" );
        if ( pAttrib->IsEmpty() )
        {
            OSL_FAIL( "Empty Attribute after ExpandAttribs?" );
            bResort = true;
            rAttribs.erase(rAttribs.begin()+nAttr);
        }
        else
        {
            ++nAttr;
        }
        pAttrib = GetAttrib(rAttribs, nAttr);
    }

    if ( bResort )
        maCharAttribList.ResortAttribs();

    if (mpWrongList)
    {
        bool bSep = ( maString[ nIndex ] == ' ' ) || IsFeature( nIndex );
        mpWrongList->TextInserted( nIndex, nNew, bSep );
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
#endif
}

void ContentNode::CollapseAttribs( sal_Int32 nIndex, sal_Int32 nDeleted )
{
    if ( !nDeleted )
        return;

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
#endif

    // Since features are treated differently than normal character attributes,
    // but can also affect the order of the start list
    bool bResort = false;
    sal_Int32 nEndChanges = nIndex+nDeleted;

    std::size_t nAttr = 0;
    CharAttribList::AttribsType& rAttribs = maCharAttribList.GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
    while ( pAttrib )
    {
        bool bDelAttr = false;
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Move all Attribute behind the insert point...
            if ( pAttrib->GetStart() >= nEndChanges )
            {
                pAttrib->MoveBackward( nDeleted );
            }
            // 1. Delete Internal attributes...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
            {
                // Special case: Attribute covers the area exactly
                // => keep as empty Attribute.
                if ( !pAttrib->IsFeature() && ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
                {
                    pAttrib->GetEnd() = nIndex; // empty
                    bResort = true;
                }
                else
                    bDelAttr = true;
            }
            // 2. Attribute starts earlier, ends inside or behind it ...
            else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                DBG_ASSERT( !pAttrib->IsFeature(), "Collapsing Feature!" );
                if ( pAttrib->GetEnd() <= nEndChanges ) // ends inside
                    pAttrib->GetEnd() = nIndex;
                else
                    pAttrib->Collaps( nDeleted );       // ends behind
            }
            // 3. Attribute starts inside, ending behind ...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
            {
                // Features not allowed to expand!
                if ( pAttrib->IsFeature() )
                {
                    pAttrib->MoveBackward( nDeleted );
                    bResort = true;
                }
                else
                {
                    pAttrib->GetStart() = nEndChanges;
                    pAttrib->MoveBackward( nDeleted );
                }
            }
        }
        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collapse: Attribute distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len()) || bDelAttr, "Collapse: Attribute larger than paragraph!" );
        if ( bDelAttr )
        {
            bResort = true;
            rAttribs.erase(rAttribs.begin()+nAttr);
        }
        else
        {
            if ( pAttrib->IsEmpty() )
                maCharAttribList.SetHasEmptyAttribs(true);
            nAttr++;
        }

        pAttrib = GetAttrib(rAttribs, nAttr);
    }

    if ( bResort )
        maCharAttribList.ResortAttribs();

    if (mpWrongList)
        mpWrongList->TextDeleted(nIndex, nDeleted);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
#endif
}

void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, bool bKeepEndingAttribs )
{
    assert(pPrevNode);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
    CharAttribList::DbgCheckAttribs(pPrevNode->maCharAttribList);
#endif

    sal_Int32 nCut = pPrevNode->Len();

    std::size_t nAttr = 0;
    CharAttribList::AttribsType& rPrevAttribs = pPrevNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rPrevAttribs, nAttr);
    while ( pAttrib )
    {
        if ( pAttrib->GetEnd() < nCut )
        {
            // remain unchanged...
            nAttr++;
        }
        else if ( pAttrib->GetEnd() == nCut )
        {
            // must be copied as an empty attributes.
            if ( bKeepEndingAttribs && !pAttrib->IsFeature() && !maCharAttribList.FindAttrib( pAttrib->GetItem()->Which(), 0 ) )
            {
                EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, 0 );
                assert(pNewAttrib);
                maCharAttribList.InsertAttrib( pNewAttrib );
            }
            nAttr++;
        }
        else if ( pAttrib->IsInside( nCut ) || ( !nCut && !pAttrib->GetStart() && !pAttrib->IsFeature() ) )
        {
            // If cut is done right at the front then the attribute must be
            // kept! Has to be copied and changed.
            EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, pAttrib->GetEnd()-nCut );
            assert(pNewAttrib);
            maCharAttribList.InsertAttrib( pNewAttrib );
            pAttrib->GetEnd() = nCut;
            nAttr++;
        }
        else
        {
            // Move all attributes in the current node (this)
            CharAttribList::AttribsType::iterator it = rPrevAttribs.begin() + nAttr;
            maCharAttribList.InsertAttrib(it->release());
            rPrevAttribs.erase(it);
            pAttrib->MoveBackward( nCut );
        }
        pAttrib = GetAttrib(rPrevAttribs, nAttr);
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
    CharAttribList::DbgCheckAttribs(pPrevNode->maCharAttribList);
#endif
}

void ContentNode::AppendAttribs( ContentNode* pNextNode )
{
    assert(pNextNode);

    sal_Int32 nNewStart = maString.getLength();

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
    CharAttribList::DbgCheckAttribs(pNextNode->maCharAttribList);
#endif

    std::size_t nAttr = 0;
    CharAttribList::AttribsType& rNextAttribs = pNextNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rNextAttribs, nAttr);
    while ( pAttrib )
    {
        // Move all attributes in the current node (this)
        bool bMelted = false;
        if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
        {
            // Attributes can possibly be summarized as:
            std::size_t nTmpAttr = 0;
            EditCharAttrib* pTmpAttrib = GetAttrib( maCharAttribList.GetAttribs(), nTmpAttr );
            while ( !bMelted && pTmpAttrib )
            {
                ++nTmpAttr;
                if ( pTmpAttrib->GetEnd() == nNewStart )
                {
                    if (pTmpAttrib->Which() == pAttrib->Which())
                    {
                        // prevent adding 2 0-length attributes at same position
                        if ((*(pTmpAttrib->GetItem()) == *(pAttrib->GetItem()))
                                || (0 == pAttrib->GetLen()))
                        {
                            pTmpAttrib->GetEnd() =
                                pTmpAttrib->GetEnd() + pAttrib->GetLen();
                            rNextAttribs.erase(rNextAttribs.begin()+nAttr);
                            // Unsubscribe from the pool?!
                            bMelted = true;
                        }
                        else if (0 == pTmpAttrib->GetLen())
                        {
                            --nTmpAttr; // to cancel earlier increment...
                            maCharAttribList.Remove(nTmpAttr);
                        }
                    }
                }
                pTmpAttrib = GetAttrib( maCharAttribList.GetAttribs(), nTmpAttr );
            }
        }

        if ( !bMelted )
        {
            pAttrib->GetStart() = pAttrib->GetStart() + nNewStart;
            pAttrib->GetEnd() = pAttrib->GetEnd() + nNewStart;
            CharAttribList::AttribsType::iterator it = rNextAttribs.begin() + nAttr;
            maCharAttribList.InsertAttrib(it->release());
            rNextAttribs.erase(it);
        }
        pAttrib = GetAttrib(rNextAttribs, nAttr);
    }
    // For the Attributes that just moved over:
    rNextAttribs.clear();

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(maCharAttribList);
    CharAttribList::DbgCheckAttribs(pNextNode->maCharAttribList);
#endif
}

void ContentNode::CreateDefFont()
{
    // First use the information from the style ...
    SfxStyleSheet* pS = maContentAttribs.GetStyleSheet();
    if ( pS )
        CreateFont( GetCharAttribs().GetDefFont(), pS->GetItemSet() );

    // ... then iron out the hard paragraph formatting...
    CreateFont( GetCharAttribs().GetDefFont(),
        GetContentAttribs().GetItems(), pS == nullptr );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle )
{
    maContentAttribs.SetStyleSheet( pS );


    // First use the information from the style ...
    GetCharAttribs().GetDefFont() = rFontFromStyle;
    // ... then iron out the hard paragraph formatting...
    CreateFont( GetCharAttribs().GetDefFont(),
                GetContentAttribs().GetItems(), pS == nullptr );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, bool bRecalcFont )
{
    maContentAttribs.SetStyleSheet( pS );
    if ( bRecalcFont )
        CreateDefFont();
}

bool ContentNode::IsFeature( sal_Int32 nPos ) const
{
    return maString[nPos] == CH_FEATURE;
}

sal_Int32 ContentNode::Len() const
{
    return maString.getLength();
}

sal_Int32 ContentNode::GetExpandedLen() const
{
    sal_Int32 nLen = maString.getLength();

    // Fields can be longer than the placeholder in the Node
    const CharAttribList::AttribsType& rAttrs = GetCharAttribs().GetAttribs();
    for (sal_Int32 nAttr = rAttrs.size(); nAttr; )
    {
        const EditCharAttrib& rAttr = *rAttrs[--nAttr];
        if (rAttr.Which() == EE_FEATURE_FIELD)
        {
            nLen += static_cast<const EditCharAttribField&>(rAttr).GetFieldValue().getLength();
            --nLen; // Standalone, to avoid corner cases when previous getLength() returns 0
        }
    }

    return nLen;
}

OUString ContentNode::GetExpandedText(sal_Int32 nStartPos, sal_Int32 nEndPos) const
{
    if ( nEndPos < 0 || nEndPos > Len() )
        nEndPos = Len();

    DBG_ASSERT( nStartPos <= nEndPos, "Start and End reversed?" );

    sal_Int32 nIndex = nStartPos;
    OUStringBuffer aStr(256);
    const EditCharAttrib* pNextFeature = GetCharAttribs().FindFeature( nIndex );
    while ( nIndex < nEndPos )
    {
        sal_Int32 nEnd = nEndPos;
        if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
            nEnd = pNextFeature->GetStart();
        else
            pNextFeature = nullptr;   // Feature does not interest the below

        DBG_ASSERT( nEnd >= nIndex, "End in front of the index?" );
        //!! beware of sub string length  of -1
        if (nEnd > nIndex)
            aStr.append( GetString().subView(nIndex, nEnd - nIndex) );

        if ( pNextFeature )
        {
            switch ( pNextFeature->GetItem()->Which() )
            {
                case EE_FEATURE_TAB:    aStr.append( "\t" );
                break;
                case EE_FEATURE_LINEBR: aStr.append( "\x0A" );
                break;
                case EE_FEATURE_FIELD:
                    aStr.append( static_cast<const EditCharAttribField*>(pNextFeature)->GetFieldValue() );
                break;
                default:    OSL_FAIL( "What feature?" );
            }
            pNextFeature = GetCharAttribs().FindFeature( ++nEnd );
        }
        nIndex = nEnd;
    }
    return aStr.makeStringAndClear();
}

void ContentNode::UnExpandPosition( sal_Int32 &rPos, bool bBiasStart )
{
    sal_Int32 nOffset = 0;

    const CharAttribList::AttribsType& rAttrs = GetCharAttribs().GetAttribs();
    for (size_t nAttr = 0; nAttr < rAttrs.size(); ++nAttr )
    {
        const EditCharAttrib& rAttr = *rAttrs[nAttr];
        assert (!(nAttr < rAttrs.size() - 1) ||
                rAttrs[nAttr]->GetStart() <= rAttrs[nAttr + 1]->GetStart());

        nOffset = rAttr.GetStart();

        if (nOffset >= rPos) // happens after the position
            return;

        if (rAttr.Which() == EE_FEATURE_FIELD)
        {
            sal_Int32 nChunk = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue().getLength();
            nChunk--; // Character representing the field in the string

            if (nOffset + nChunk >= rPos) // we're inside the field
            {
                if (bBiasStart)
                    rPos = rAttr.GetStart();
                else
                    rPos = rAttr.GetEnd();
                return;
            }
            // Adjust for the position
            rPos -= nChunk;
        }
    }
    assert (rPos <= Len());
}

/*
 * Fields are represented by a single character in the underlying string
 * and/or selection, however, they can be expanded to the full value of
 * the field. When we're dealing with selection / offsets however we need
 * to deal in character positions inside the real (unexpanded) string.
 * This method maps us back to character offsets.
 */
void ContentNode::UnExpandPositions( sal_Int32 &rStartPos, sal_Int32 &rEndPos )
{
    UnExpandPosition( rStartPos, true );
    UnExpandPosition( rEndPos, false );
}

void ContentNode::SetChar(sal_Int32 nPos, sal_Unicode c)
{
    maString = maString.replaceAt(nPos, 1, rtl::OUStringChar(c));
}

void ContentNode::Insert(std::u16string_view rStr, sal_Int32 nPos)
{
    maString = maString.replaceAt(nPos, 0, rStr);
}

void ContentNode::Append(std::u16string_view rStr)
{
    maString += rStr;
}

void ContentNode::Erase(sal_Int32 nPos)
{
    maString = maString.copy(0, nPos);
}

void ContentNode::Erase(sal_Int32 nPos, sal_Int32 nCount)
{
    maString = maString.replaceAt(nPos, nCount, u"");
}

OUString ContentNode::Copy(sal_Int32 nPos) const
{
    return maString.copy(nPos);
}

OUString ContentNode::Copy(sal_Int32 nPos, sal_Int32 nCount) const
{
    return maString.copy(nPos, nCount);
}

sal_Unicode ContentNode::GetChar(sal_Int32 nPos) const
{
    return maString[nPos];
}

void ContentNode::EnsureWrongList()
{
    if (!mpWrongList)
        CreateWrongList();
}

WrongList* ContentNode::GetWrongList()
{
    return mpWrongList.get();
}

const WrongList* ContentNode::GetWrongList() const
{
    return mpWrongList.get();
}

void ContentNode::SetWrongList( WrongList* p )
{
    mpWrongList.reset(p);
}

void ContentNode::CreateWrongList()
{
    SAL_WARN_IF( mpWrongList && !mpWrongList->empty(), "editeng", "WrongList already exist!");
    if (!mpWrongList || !mpWrongList->empty())
        mpWrongList.reset(new WrongList);
}

void ContentNode::DestroyWrongList()
{
    mpWrongList.reset();
}

void ContentNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ContentNode"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maString"), BAD_CAST(maString.toUtf8().getStr()));
    maContentAttribs.dumpAsXml(pWriter);
    maCharAttribList.dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

void ContentNode::checkAndDeleteEmptyAttribs() const
{
    // Delete empty attributes, but only if paragraph is not empty!
    if (GetCharAttribs().HasEmptyAttribs() && Len())
    {
        const_cast<ContentNode*>(this)->GetCharAttribs().DeleteEmptyAttribs();
    }
}

ContentAttribs::ContentAttribs( SfxItemPool& rPool )
    : maAttribSet(rPool)
{
}

SvxTabStop ContentAttribs::FindTabStop( sal_Int32 nCurPos, sal_uInt16 nDefTab )
{
    const SvxTabStopItem& rTabs = GetItem( EE_PARA_TABS );
    for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
    {
        const SvxTabStop& rTab = rTabs[i];
        if ( rTab.GetTabPos() > nCurPos  )
            return rTab;
    }

    // if there's a default tab size defined for this item use that instead
    if (rTabs.GetDefaultDistance())
        nDefTab = rTabs.GetDefaultDistance();

    // Determine DefTab ...
    SvxTabStop aTabStop;
    const sal_Int32 x = nCurPos / nDefTab + 1;
    aTabStop.GetTabPos() = nDefTab * x;
    return aTabStop;
}

void ContentAttribs::SetStyleSheet( SfxStyleSheet* pS )
{
    bool bStyleChanged = ( mpStyle != pS );
    mpStyle = pS;
    // Only when other style sheet, not when current style sheet modified
    if ( !(mpStyle && bStyleChanged) )
        return;

    // Selectively remove the attributes from the paragraph formatting
    // which are specified in the style, so that the attributes of the
    // style can have an affect.
    const SfxItemSet& rStyleAttribs = mpStyle->GetItemSet();
    for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        // Don't change bullet on/off
        if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SfxItemState::SET ) )
            maAttribSet.ClearItem( nWhich );
    }
}

const SfxPoolItem& ContentAttribs::GetItem( sal_uInt16 nWhich ) const
{
    // Hard paragraph attributes take precedence!
    const SfxItemSet* pTakeFrom = &maAttribSet;
    if ( mpStyle && ( maAttribSet.GetItemState( nWhich, false ) != SfxItemState::SET  ) )
        pTakeFrom = &mpStyle->GetItemSet();

    return pTakeFrom->Get( nWhich );
}

bool ContentAttribs::HasItem( sal_uInt16 nWhich ) const
{
    bool bHasItem = false;
    if ( maAttribSet.GetItemState( nWhich, false ) == SfxItemState::SET  )
        bHasItem = true;
    else if ( mpStyle && mpStyle->GetItemSet().GetItemState( nWhich ) == SfxItemState::SET )
        bHasItem = true;

    return bHasItem;
}

void ContentAttribs::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ContentAttribs"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("style"), "%s", mpStyle->GetName().toUtf8().getStr());
    maAttribSet.dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

namespace {

struct LessByStart
{
    bool operator() (const std::unique_ptr<EditCharAttrib>& left, const std::unique_ptr<EditCharAttrib>& right) const
    {
        return left->GetStart() < right->GetStart();
    }
};

}

void CharAttribList::InsertAttrib( EditCharAttrib* pAttrib )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// optimize: binary search?    !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Maybe just simply iterate backwards:
    // The most common and critical case: Attributes are already sorted
    // (InsertTextObject!) binary search would not be optimal here.
    // => Would bring something!

    const sal_Int32 nStart = pAttrib->GetStart(); // may be better for Comp.Opt.

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif

    if ( pAttrib->IsEmpty() )
        mbHasEmptyAttribs = true;

    bool bInsert(true);
    for (sal_Int32 i = 0, n = maAttribs.size(); i < n; ++i)
    {
        const EditCharAttrib& rCurAttrib = *maAttribs[i];
        if (rCurAttrib.GetStart() > nStart)
        {
            maAttribs.insert(maAttribs.begin()+i, std::unique_ptr<EditCharAttrib>(pAttrib));
            bInsert = false;
            break;
        }
    }

    if (bInsert) maAttribs.push_back(std::unique_ptr<EditCharAttrib>(pAttrib));

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

void CharAttribList::ResortAttribs()
{
    std::sort(maAttribs.begin(), maAttribs.end(), LessByStart());

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

void CharAttribList::OptimizeRanges()
{
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(maAttribs.size()); ++i)
    {
        EditCharAttrib& rAttr = *maAttribs[i];
        for (sal_Int32 nNext = i+1; nNext < static_cast<sal_Int32>(maAttribs.size()); ++nNext)
        {
            EditCharAttrib& rNext = *maAttribs[nNext];
            if (!rAttr.IsFeature() && rNext.GetStart() == rAttr.GetEnd() && rNext.Which() == rAttr.Which())
            {
                if (*rNext.GetItem() == *rAttr.GetItem())
                {
                    rAttr.GetEnd() = rNext.GetEnd();
                    maAttribs.erase(maAttribs.begin()+nNext);
                }
                break;  // only 1 attr with same which can start here.
            }
            else if (rNext.GetStart() > rAttr.GetEnd())
            {
                break;
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

sal_Int32 CharAttribList::Count() const
{
    return maAttribs.size();
}

const EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = std::find_if(maAttribs.rbegin(), maAttribs.rend(),
        [&nWhich, &nPos](const AttribsType::value_type& rxAttr) {
            return rxAttr->Which() == nWhich && rxAttr->IsIn(nPos); });
    if (it != maAttribs.rend())
    {
        const EditCharAttrib& rAttr = **it;
        return &rAttr;
    }
    return nullptr;
}

EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::reverse_iterator it = std::find_if(maAttribs.rbegin(), maAttribs.rend(),
        [&nWhich, &nPos](AttribsType::value_type& rxAttr) {
            return rxAttr->Which() == nWhich && rxAttr->IsIn(nPos); });
    if (it != maAttribs.rend())
    {
        EditCharAttrib& rAttr = **it;
        return &rAttr;
    }
    return nullptr;
}

EditCharAttrib* CharAttribList::FindAttribRightOpen( sal_uInt16 nWhich, sal_Int32 nPos )
{
    AttribsType::reverse_iterator it = std::find_if(maAttribs.rbegin(), maAttribs.rend(),
        [&nWhich, &nPos](AttribsType::value_type& rxAttr) {
            return rxAttr->Which() == nWhich && rxAttr->IsInLeftClosedRightOpen(nPos); });
    if (it != maAttribs.rend())
    {
        EditCharAttrib& rAttr = **it;
        return &rAttr;
    }
    return nullptr;
}

const EditCharAttrib* CharAttribList::FindNextAttrib( sal_uInt16 nWhich, sal_Int32 nFromPos ) const
{
    assert(nWhich);
    for (auto const& attrib : maAttribs)
    {
        const EditCharAttrib& rAttr = *attrib;
        if (rAttr.GetStart() >= nFromPos && rAttr.Which() == nWhich)
            return &rAttr;
    }
    return nullptr;
}

bool CharAttribList::HasAttrib( sal_Int32 nStartPos, sal_Int32 nEndPos ) const
{
    return std::any_of(maAttribs.rbegin(), maAttribs.rend(),
        [&nStartPos, &nEndPos](const AttribsType::value_type& rxAttr) {
            return rxAttr->GetStart() < nEndPos && rxAttr->GetEnd() > nStartPos; });
}


namespace {

class FindByAddress
{
    const EditCharAttrib* mpAttr;
public:
    explicit FindByAddress(const EditCharAttrib* p) : mpAttr(p) {}
    bool operator() (const std::unique_ptr<EditCharAttrib>& r) const
    {
        return r.get() == mpAttr;
    }
};

}

void CharAttribList::Remove(const EditCharAttrib* p)
{
    AttribsType::iterator it = std::find_if(maAttribs.begin(), maAttribs.end(), FindByAddress(p));
    if (it != maAttribs.end())
        maAttribs.erase(it);
}

void CharAttribList::Remove(sal_Int32 nPos)
{
    if (nPos >= static_cast<sal_Int32>(maAttribs.size()))
        return;

    maAttribs.erase(maAttribs.begin()+nPos);
}

void CharAttribList::SetHasEmptyAttribs(bool b)
{
    mbHasEmptyAttribs = b;
}

bool CharAttribList::HasBoundingAttrib( sal_Int32 nBound ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = maAttribs.rbegin(), itEnd = maAttribs.rend();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = **it;
        if (rAttr.GetEnd() < nBound)
            return false;

        if (rAttr.GetStart() == nBound || rAttr.GetEnd() == nBound)
            return true;
    }
    return false;
}

EditCharAttrib* CharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    if ( !mbHasEmptyAttribs )
        return nullptr;

    for (const std::unique_ptr<EditCharAttrib>& rAttr : maAttribs)
    {
        if (rAttr->GetStart() == nPos && rAttr->GetEnd() == nPos && rAttr->Which() == nWhich)
            return rAttr.get();
    }
    return nullptr;
}

namespace
{

class FindByStartPos
{
    sal_Int32 mnPos;
public:
    explicit FindByStartPos(sal_Int32 nPos)
        : mnPos(nPos)
    {}

    bool operator() (std::unique_ptr<EditCharAttrib> const& pCharAttrib) const
    {
        return pCharAttrib->GetStart() >= mnPos;
    }
};

}

const EditCharAttrib* CharAttribList::FindFeature( sal_Int32 nPos ) const
{
    // First, find the first attribute that starts at or after specified position.
    AttribsType::const_iterator iterator =
        std::find_if(maAttribs.begin(), maAttribs.end(), FindByStartPos(nPos));

    if (iterator == maAttribs.end())
    {
        // All attributes are before the specified position.
        return nullptr;
    }

    // And find the first attribute with feature.
    iterator = std::find_if(iterator, maAttribs.end(), [](const std::unique_ptr<EditCharAttrib>& aAttrib) {
        return aAttrib->IsFeature();
    });

    if (iterator == maAttribs.end())
    {
        // Couldn't find the feature
        return nullptr;
    }

    // Found
    return iterator->get();
}

void CharAttribList::DeleteEmptyAttribs()
{
    std::erase_if(maAttribs, [](const std::unique_ptr<EditCharAttrib>& aAttrib) { return aAttrib->IsEmpty(); } );
    mbHasEmptyAttribs = false;
}

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
void CharAttribList::DbgCheckAttribs(CharAttribList const& rAttribs)
{
    std::set<std::pair<sal_Int32, sal_uInt16>> zero_set;
    for (const std::unique_ptr<EditCharAttrib>& rAttr : rAttribs.maAttribs)
    {
        assert(rAttr->GetStart() <= rAttr->GetEnd());
        assert(!rAttr->IsFeature() || rAttr->GetLen() == 1);
        if (0 == rAttr->GetLen())
        {
            // not sure if 0-length attributes allowed at all in non-empty para?
            assert(zero_set.insert(std::make_pair(rAttr->GetStart(), rAttr->Which())).second && "duplicate 0-length attribute detected");
        }
    }
    CheckOrderedList(rAttribs.GetAttribs());
}
#endif

void CharAttribList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("CharAttribList"));
    for (auto const & i : maAttribs) {
        i->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
