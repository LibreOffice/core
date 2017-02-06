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

#include <sal/config.h>

#include <hintids.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svl/whiter.hxx>
#include <svl/itemiter.hxx>
#include <svl/stylepool.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/lrspitem.hxx>
#include <txtinet.hxx>
#include <txtflcnt.hxx>
#include <fmtfld.hxx>
#include <fmtrfmrk.hxx>
#include <fmtanchr.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <fmtflcnt.hxx>
#include <fmtftn.hxx>
#include <txttxmrk.hxx>
#include <txtrfmrk.hxx>
#include <txtftn.hxx>
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <charatr.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <ftnidx.hxx>
#include <fmtruby.hxx>
#include <fmtmeta.hxx>
#include <breakit.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <rolbck.hxx>
#include <ddefld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <poolfmt.hxx>
#include <swfont.hxx>
#include <istyleaccess.hxx>
#include <dcontact.hxx>
#include <docsh.hxx>
#include <algorithm>
#include <map>
#include <memory>

#ifdef DBG_UTIL
#define CHECK           Check(true);
#define CHECK_NOTMERGED Check(false);
#else
#define CHECK_NOTMERGED
#endif

using namespace ::com::sun::star::i18n;

SwpHints::SwpHints()
    : m_pHistory(nullptr)
    , m_bInSplitNode(false)
    , m_bCalcHiddenParaField(false)
    , m_bHasHiddenParaField(false)
    , m_bFootnote(false)
    , m_bDDEFields(false)
{
}

static void TextAttrDelete( SwDoc & rDoc, SwTextAttr * const pAttr )
{
    if (RES_TXTATR_META == pAttr->Which() ||
        RES_TXTATR_METAFIELD == pAttr->Which())
    {
        static_txtattr_cast<SwTextMeta *>(pAttr)->ChgTextNode(nullptr); // prevents ASSERT
    }
    SwTextAttr::Destroy( pAttr, rDoc.GetAttrPool() );
}

static bool TextAttrContains(const sal_Int32 nPos, SwTextAttrEnd * const pAttr)
{
    return (pAttr->GetStart() < nPos) && (nPos < *pAttr->End());
}

// a:       |-----|
// b:
//    |---|               => valid: b before a
//    |-----|             => valid: start == end; b before a
//    |---------|         => invalid: overlap (1)
//    |-----------|       => valid: same end; b around a
//    |-----------------| => valid: b around a
//          |---|         => valid; same start; b within a
//          |-----|       => valid; same start and end; b around or within a?
//          |-----------| => valid: same start: b around a
//            |-|         => valid: b within a
//            |---|       => valid: same end; b within a
//            |---------| => invalid: overlap (2)
//                |-----| => valid: end == start; b after a
//                  |---| => valid: b after a
// ===> 2 invalid overlap cases
static
bool isOverlap(const sal_Int32 nStart1, const sal_Int32 nEnd1,
               const sal_Int32 nStart2, const sal_Int32 nEnd2)
{
    return
        ((nStart1 > nStart2) && (nStart1 < nEnd2) && (nEnd1 > nEnd2))  // (1)
     || ((nStart1 < nStart2) && (nStart2 < nEnd1) && (nEnd1 < nEnd2)); // (2)
}

/// #i106930#: now asymmetric: empty hint1 is _not_ nested, but empty hint2 is
static
bool isNestedAny(const sal_Int32 nStart1, const sal_Int32 nEnd1,
                 const sal_Int32 nStart2, const sal_Int32 nEnd2)
{
    return ((nStart1 == nStart2) || (nEnd1 == nEnd2))
        // same start/end: nested except if hint1 empty and hint2 not empty
        ? (nStart1 != nEnd1) || (nStart2 == nEnd2)
        : ((nStart1 < nStart2) ? (nEnd1 >= nEnd2) : (nEnd1 <= nEnd2));
}

static
bool isSelfNestable(const sal_uInt16 nWhich)
{
    if ((RES_TXTATR_INETFMT  == nWhich) ||
        (RES_TXTATR_CJK_RUBY == nWhich) ||
        (RES_TXTATR_INPUTFIELD == nWhich))
        return false;
    assert((RES_TXTATR_META  == nWhich) ||
           (RES_TXTATR_METAFIELD  == nWhich));
    return true;
}

static
bool isSplittable(const sal_uInt16 nWhich)
{
    if ((RES_TXTATR_INETFMT  == nWhich) ||
        (RES_TXTATR_CJK_RUBY == nWhich))
        return true;
    assert((RES_TXTATR_META  == nWhich) ||
           (RES_TXTATR_METAFIELD  == nWhich) ||
           (RES_TXTATR_INPUTFIELD  == nWhich));
    return false;
}

enum Split_t { FAIL, SPLIT_NEW, SPLIT_OTHER };
/**
  Calculate splitting policy for overlapping hints, based on what kind of
  hint is inserted, and what kind of existing hint overlaps.
  */
static Split_t
splitPolicy(const sal_uInt16 nWhichNew, const sal_uInt16 nWhichOther)
{
    if (!isSplittable(nWhichOther))
    {
        if (!isSplittable(nWhichNew))
            return FAIL;
        else
            return SPLIT_NEW;
    }
    else
    {
        if ( RES_TXTATR_INPUTFIELD == nWhichNew )
            return FAIL;
        else if ( (RES_TXTATR_INETFMT  == nWhichNew) &&
                  (RES_TXTATR_CJK_RUBY == nWhichOther) )
            return SPLIT_NEW;
        else
            return SPLIT_OTHER;
    }
}

void SwTextINetFormat::InitINetFormat(SwTextNode & rNode)
{
    ChgTextNode(&rNode);
    SwCharFormat * const pFormat(
         rNode.GetDoc()->getIDocumentStylePoolAccess().GetCharFormatFromPool(RES_POOLCHR_INET_NORMAL) );
    pFormat->Add( this );
}

void SwTextRuby::InitRuby(SwTextNode & rNode)
{
    ChgTextNode(&rNode);
    SwCharFormat * const pFormat(
        rNode.GetDoc()->getIDocumentStylePoolAccess().GetCharFormatFromPool(RES_POOLCHR_RUBYTEXT) );
    pFormat->Add( this );
}

/**
  Create a new nesting text hint.
 */
static SwTextAttrNesting *
MakeTextAttrNesting(SwTextNode & rNode, SwTextAttrNesting & rNesting,
        const sal_Int32 nStart, const sal_Int32 nEnd)
{
    SwTextAttr * const pNew( MakeTextAttr(
            *rNode.GetDoc(), rNesting.GetAttr(), nStart, nEnd ) );
    switch (pNew->Which())
    {
        case RES_TXTATR_INETFMT:
        {
            static_txtattr_cast<SwTextINetFormat*>(pNew)->InitINetFormat(rNode);
            break;
        }
        case RES_TXTATR_CJK_RUBY:
        {
            static_txtattr_cast<SwTextRuby*>(pNew)->InitRuby(rNode);
            break;
        }
        default:
            assert(!"MakeTextAttrNesting: what the hell is that?");
            break;
    }
    return static_txtattr_cast<SwTextAttrNesting*>(pNew);
}

typedef std::vector<SwTextAttrNesting *> NestList_t;

static void
lcl_DoSplitNew(NestList_t & rSplits, SwTextNode & rNode,
    const sal_Int32 nNewStart,
    const sal_Int32 nOtherStart, const sal_Int32 nOtherEnd, bool bOtherDummy)
{
    const bool bSplitAtStart(nNewStart < nOtherStart);
    const sal_Int32 nSplitPos( (bSplitAtStart) ? nOtherStart : nOtherEnd );
    // first find the portion that is split (not necessarily the last one!)
    NestList_t::iterator const iter(
        std::find_if( rSplits.begin(), rSplits.end(),
            [nSplitPos](SwTextAttrEnd * const pAttr) {
                return TextAttrContains(nSplitPos, pAttr);
            } ) );
    if (iter != rSplits.end()) // already split here?
    {
        const sal_Int32 nStartPos( // skip other's dummy character!
            (bSplitAtStart && bOtherDummy) ? nSplitPos + 1 : nSplitPos );
        SwTextAttrNesting * const pNew( MakeTextAttrNesting(
                rNode, **iter, nStartPos, *(*iter)->GetEnd() ) );
        *(*iter)->GetEnd() = nSplitPos;
        rSplits.insert(iter + 1, pNew);
    }
}

/**
  Insert nesting hint into the hints array. Also calls NoteInHistory.
  @param    rNewHint    the hint to be inserted (must not overlap existing!)
 */
void SwpHints::InsertNesting(SwTextAttrNesting & rNewHint)
{
    Insert(& rNewHint);
    NoteInHistory( & rNewHint, true );
}

/**

The following hints correspond to well-formed XML elements in ODF:
RES_TXTATR_INETFMT, RES_TXTATR_CJK_RUBY, RES_TXTATR_META, RES_TXTATR_METAFIELD

The writer core must ensure that these do not overlap; if they did,
the document would not be storable as ODF.

Also, a Hyperlink must not be nested within another Hyperlink,
and a Ruby must not be nested within another Ruby.

The ODF export in xmloff will only put a hyperlink into a ruby, never a ruby
into a hyperlink.

Unfortunately the UNO API for Hyperlink and Ruby consists of the properties
Hyperlink* and Ruby* of the css.text.CharacterProperties service.  In other
words, they are treated as formatting attributes, not as content entites.
Furthermore, for API users it is not possible to easily test whether a certain
range would be overlapping with other nested attributes, and most importantly,
<em>which ones</em>, so we can hardly refuse to insert these in cases of
overlap.

It is possible to split Hyperlink and Ruby into multiple portions, such that
the result is properly nested.

meta and meta-field must not be split, because they have xml:id.

These constraints result in the following design:

RES_TXTATR_INETFMT:
    always succeeds
    inserts n attributes split at RES_TXTATR_CJK_RUBY, RES_TXTATR_META,
        RES_TXTATR_METAFIELD
    may replace existing RES_TXTATR_INETFMT at overlap
RES_TXTATR_CJK_RUBY:
    always succeeds
    inserts n attributes split at RES_TXTATR_META, RES_TXTATR_METAFIELD
    may replace existing RES_TXTATR_CJK_RUBY at overlap
    may split existing overlapping RES_TXTATR_INETFMT
RES_TXTATR_META:
    may fail if overlapping existing RES_TXTATR_META/RES_TXTATR_METAFIELD
    may split existing overlapping RES_TXTATR_INETFMT or RES_TXTATR_CJK_RUBY
    inserts 1 attribute
RES_TXTATR_METAFIELD:
    may fail if overlapping existing RES_TXTATR_META/RES_TXTATR_METAFIELD
    may split existing overlapping RES_TXTATR_INETFMT or RES_TXTATR_CJK_RUBY
    inserts 1 attribute

The nesting is expressed by the position of the hints.
RES_TXTATR_META and RES_TXTATR_METAFIELD have a CH_TXTATR, and there can
only be one such hint starting and ending at a given position.
Only RES_TXTATR_INETFMT and RES_TXTATR_CJK_RUBY lack a CH_TXTATR.
The interpretation given is that RES_TXTATR_CJK_RUBY is always around
a RES_TXTATR_INETFMT at the same start and end position (which corresponds
with the UNO API).
Both of these are always around a nesting hint with CH_TXTATR at the same
start and end position (if they should be inside, then the start should be
after the CH_TXTATR).
It would probably be a bad idea to add another nesting hint without
CH_TXTATR; on the other hand, it would be difficult adding a CH_TXTATR to
RES_TXTATR_INETFMT and RES_TXTATR_CJK_RUBY, due to the overwriting and
splitting of exising hints that is necessary for backward compatibility.

    @param rNode    the text node
    @param rHint    the hint to be inserted
    @returns        true iff hint was successfully inserted
*/
bool
SwpHints::TryInsertNesting( SwTextNode & rNode, SwTextAttrNesting & rNewHint )
{
//    INVARIANT:  the nestable hints in the array are properly nested
    const sal_uInt16 nNewWhich( rNewHint.Which() );
    const sal_Int32 nNewStart( rNewHint.GetStart() );
    const sal_Int32 nNewEnd  ( *rNewHint.GetEnd()   );
    const bool bNewSelfNestable( isSelfNestable(nNewWhich) );

    assert( (RES_TXTATR_INETFMT   == nNewWhich) ||
            (RES_TXTATR_CJK_RUBY  == nNewWhich) ||
            (RES_TXTATR_META      == nNewWhich) ||
            (RES_TXTATR_METAFIELD == nNewWhich) ||
            (RES_TXTATR_INPUTFIELD == nNewWhich));

    NestList_t OverlappingExisting; // existing hints to be split
    NestList_t OverwrittenExisting; // existing hints to be replaced
    NestList_t SplitNew;            // new hints to be inserted

    SplitNew.push_back(& rNewHint);

    // pass 1: split the inserted hint into fragments if necessary
    for ( size_t i = 0; i < Count(); ++i )
    {
        SwTextAttr * const pOther = GetSortedByEnd(i);

        if (pOther->IsNesting())
        {
            const sal_uInt16 nOtherWhich( pOther->Which() );
            const sal_Int32 nOtherStart( pOther->GetStart() );
            const sal_Int32 nOtherEnd  ( *(pOther)->GetEnd()   );
            if (isOverlap(nNewStart, nNewEnd, nOtherStart, nOtherEnd ))
            {
                switch (splitPolicy(nNewWhich, nOtherWhich))
                {
                    case FAIL:
                        SAL_INFO("sw.core", "cannot insert hint: overlap");
                        for (const auto& aSplit : SplitNew)
                            TextAttrDelete(*rNode.GetDoc(), aSplit);
                        return false;
                    case SPLIT_NEW:
                        lcl_DoSplitNew(SplitNew, rNode, nNewStart,
                            nOtherStart, nOtherEnd, pOther->HasDummyChar());
                        break;
                    case SPLIT_OTHER:
                        OverlappingExisting.push_back(
                            static_txtattr_cast<SwTextAttrNesting*>(pOther));
                        break;
                    default:
                        assert(!"bad code monkey");
                        break;
                }
            }
            else if (isNestedAny(nNewStart, nNewEnd, nOtherStart, nOtherEnd))
            {
                if (!bNewSelfNestable && (nNewWhich == nOtherWhich))
                {
                // ruby and hyperlink: if there is nesting, _overwrite_
                OverwrittenExisting.push_back(
                    static_txtattr_cast<SwTextAttrNesting*>(pOther));
                }
                else if ((nNewStart == nOtherStart) && pOther->HasDummyChar())
                {
                    if (rNewHint.HasDummyChar())
                    {
                        assert(!"ERROR: inserting duplicate CH_TXTATR hint");
                        return false;
                    } else if (nNewEnd < nOtherEnd) {
                        // other has dummy char, new is inside other, but
                        // new contains the other's dummy char?
                        // should be corrected because it may lead to problems
                        // in SwXMeta::createEnumeration
                        // SplitNew is sorted, so this is the first split
                        sal_Int32& rStart(SplitNew.front()->GetStart());
                        assert(rStart == nNewStart);
                        rStart = nNewStart + 1;
                    }
                }
            }
        }
    }

    assert((isSplittable(nNewWhich) || SplitNew.size() == 1) &&
            "splitting the unsplittable ???");

    // pass 2: split existing hints that overlap/nest with new hint
    // do not iterate over hints array, but over remembered set of overlapping
    // hints, to keep things simple w.r.t. insertion/removal
    // N.B: if there is a hint that splits the inserted hint, then
    // that hint would also have already split any hint in OverlappingExisting
    // so any hint in OverlappingExisting can be split at most by one hint
    // in SplitNew, or even not at all (this is not true for existing hints
    // that go _around_ new hint, which is the reason d'^etre for pass 4)
    for (NestList_t::iterator itOther = OverlappingExisting.begin();
            itOther != OverlappingExisting.end(); ++itOther)
    {
        const sal_Int32 nOtherStart( (*itOther)->GetStart() );
        const sal_Int32 nOtherEnd  ( *(*itOther)->GetEnd()   );

        for (NestList_t::iterator itNew = SplitNew.begin();
                itNew != SplitNew.end(); ++itNew)
        {
            const sal_Int32 nSplitNewStart( (*itNew)->GetStart() );
            const sal_Int32 nSplitNewEnd  ( *(*itNew)->GetEnd()   );
            // 4 cases: within, around, overlap l, overlap r, (OTHER: no action)
            const bool bRemoveOverlap(
                !bNewSelfNestable && (nNewWhich == (*itOther)->Which()) );

            switch (ComparePosition(nSplitNewStart, nSplitNewEnd,
                                    nOtherStart,    nOtherEnd))
            {
                case POS_INSIDE:
                    {
                        assert(!bRemoveOverlap &&
                            "this one should be in OverwrittenExisting?");
                    }
                    break;
                case POS_OUTSIDE:
                case POS_EQUAL:
                    {
                        assert(!"existing hint inside new hint: why?");
                    }
                    break;
                case POS_OVERLAP_BEFORE:
                    {
                        Delete( *itOther ); // this also does NoteInHistory!
                        (*itOther)->GetStart() = nSplitNewEnd;
                        InsertNesting( **itOther );
                        if (!bRemoveOverlap)
                        {
                            if ( MAX_HINTS <= Count() )
                            {
                                SAL_INFO("sw.core", "hints array full :-(");
                                return false;
                            }
                            SwTextAttrNesting * const pOtherLeft(
                                MakeTextAttrNesting( rNode, **itOther,
                                    nOtherStart, nSplitNewEnd ) );
                            InsertNesting( *pOtherLeft );
                        }
                    }
                    break;
                case POS_OVERLAP_BEHIND:
                    {
                        Delete( *itOther ); // this also does NoteInHistory!
                        *(*itOther)->GetEnd() = nSplitNewStart;
                        InsertNesting( **itOther );
                        if (!bRemoveOverlap)
                        {
                            if ( MAX_HINTS <= Count() )
                            {
                                SAL_INFO("sw.core", "hints array full :-(");
                                return false;
                            }
                            SwTextAttrNesting * const pOtherRight(
                                MakeTextAttrNesting( rNode, **itOther,
                                    nSplitNewStart, nOtherEnd ) );
                            InsertNesting( *pOtherRight );
                        }
                    }
                    break;
                default:
                    break; // overlap resolved by splitting new: nothing to do
            }
        }
    }

    if ( MAX_HINTS <= Count() || MAX_HINTS - Count() <= SplitNew.size() )
    {
        SAL_INFO("sw.core", "hints array full :-(");
        return false;
    }

    // pass 3: insert new hints
    for (NestList_t::iterator iter = SplitNew.begin();
            iter != SplitNew.end(); ++iter)
    {
        InsertNesting(**iter);
    }

    // pass 4: handle overwritten hints
    // RES_TXTATR_INETFMT and RES_TXTATR_CJK_RUBY should displace attributes
    // of the same kind.
    for (NestList_t::iterator itOther = OverwrittenExisting.begin();
            itOther != OverwrittenExisting.end(); ++itOther)
    {
        const sal_Int32 nOtherStart( (*itOther)->GetStart() );
        const sal_Int32 nOtherEnd  ( *(*itOther)->GetEnd()   );

        // overwritten portion is given by start/end of inserted hint
        if ((nNewStart <= nOtherStart) && (nOtherEnd <= nNewEnd))
        {
            Delete(*itOther);
            rNode.DestroyAttr( *itOther );
        }
        else
        {
           assert((nOtherStart < nNewStart) || (nNewEnd < nOtherEnd));
        // scenario: there is a RUBY, and contained within that a META;
        // now a RUBY is inserted within the META => the exising RUBY is split:
        // here it is not possible to simply insert the left/right fragment
        // of the existing RUBY because they <em>overlap</em> with the META!
            Delete( *itOther ); // this also does NoteInHistory!
            if (nNewEnd < nOtherEnd)
            {
                SwTextAttrNesting * const pOtherRight(
                    MakeTextAttrNesting(
                        rNode, **itOther, nNewEnd, nOtherEnd ) );
                bool const bSuccess( TryInsertNesting(rNode, *pOtherRight) );
                SAL_WARN_IF(!bSuccess, "sw.core", "recursive call 1 failed?");
            }
            if (nOtherStart < nNewStart)
            {
                *(*itOther)->GetEnd() = nNewStart;
                bool const bSuccess( TryInsertNesting(rNode, **itOther) );
                SAL_WARN_IF(!bSuccess, "sw.core", "recursive call 2 failed?");
            }
            else
            {
                rNode.DestroyAttr(*itOther);
            }
        }
    }

    return true;
}

// This function takes care for the following text attribute:
// RES_TXTATR_CHARFMT, RES_TXTATR_AUTOFMT
// These attributes have to be handled in a special way (Portion building).

// The new attribute will be split by any existing RES_TXTATR_AUTOFMT or
// RES_TXTATR_CHARFMT. The new attribute itself will
// split any existing RES_TXTATR_AUTOFMT or RES_TXTATR_CHARFMT.

void SwpHints::BuildPortions( SwTextNode& rNode, SwTextAttr& rNewHint,
        const SetAttrMode nMode )
{
    const sal_uInt16 nWhich = rNewHint.Which();

    const sal_Int32 nThisStart = rNewHint.GetStart();
    const sal_Int32 nThisEnd =   *rNewHint.GetEnd();
    const bool bNoLengthAttribute = nThisStart == nThisEnd;

    std::vector<SwTextAttr*> aInsDelHints;
    std::vector<SwTextAttr*>::iterator aIter;

    assert( RES_TXTATR_CHARFMT == rNewHint.Which() ||
            RES_TXTATR_AUTOFMT == rNewHint.Which() );

    // 2. Find the hints which cover the start and end position
    // of the new hint. These hints have to be split into two portions:

    if ( !bNoLengthAttribute ) // nothing to do for no length attributes
    {
        for ( size_t i = 0; i < Count(); ++i )
        {
            SwTextAttr* pOther = Get(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            sal_Int32 nOtherStart = pOther->GetStart();
            const sal_Int32 nOtherEnd = *pOther->GetEnd();

            // Check if start of new attribute overlaps with pOther:
            // Split pOther if necessary:
            if ( nOtherStart < nThisStart && nThisStart < nOtherEnd )
            {
                SwTextAttr* pNewAttr = MakeTextAttr( *rNode.GetDoc(),
                        pOther->GetAttr(), nOtherStart, nThisStart );
                if ( RES_TXTATR_CHARFMT == pOther->Which() )
                {
                    static_txtattr_cast<SwTextCharFormat*>(pNewAttr)->SetSortNumber(
                        static_txtattr_cast<SwTextCharFormat*>(pOther)->GetSortNumber() );
                }
                aInsDelHints.push_back( pNewAttr );

                NoteInHistory( pOther );
                pOther->GetStart() = nThisStart;
                NoteInHistory( pOther, true );

                nOtherStart = nThisStart;
            }

            // Check if end of new attribute overlaps with pOther:
            // Split pOther if necessary:
            if ( nOtherStart < nThisEnd && nThisEnd < nOtherEnd )
            {
                SwTextAttr* pNewAttr = MakeTextAttr( *rNode.GetDoc(),
                        pOther->GetAttr(), nOtherStart, nThisEnd );
                if ( RES_TXTATR_CHARFMT == pOther->Which() )
                {
                    static_txtattr_cast<SwTextCharFormat*>(pNewAttr)->SetSortNumber(
                        static_txtattr_cast<SwTextCharFormat*>(pOther)->GetSortNumber());
                }
                aInsDelHints.push_back( pNewAttr );

                NoteInHistory( pOther );
                pOther->GetStart() = nThisEnd;
                NoteInHistory( pOther, true );
            }
        }

        // Insert the newly created attributes:
        for ( aIter = aInsDelHints.begin(); aIter != aInsDelHints.end(); ++aIter )
        {
            Insert( *aIter );
            NoteInHistory( *aIter, true );
        }
    }

#ifdef DBG_UTIL
    if( !rNode.GetDoc()->IsInReading() )
        CHECK_NOTMERGED; // ignore flags not set properly yet, don't check them
#endif

    // 4. Split rNewHint into 1 ... n new hints:

    std::set<sal_Int32> aBounds;
    aBounds.insert( nThisStart );
    aBounds.insert( nThisEnd );

    if ( !bNoLengthAttribute ) // nothing to do for no length attributes
    {
        for ( size_t i = 0; i < Count(); ++i )
        {
            const SwTextAttr* pOther = Get(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            const sal_Int32 nOtherStart = pOther->GetStart();
            const sal_Int32 nOtherEnd = *pOther->End();

            aBounds.insert( nOtherStart );
            aBounds.insert( nOtherEnd );
        }
    }

    std::set<sal_Int32>::iterator aStartIter = aBounds.lower_bound( nThisStart );
    std::set<sal_Int32>::iterator aEndIter = aBounds.upper_bound( nThisEnd );
    sal_Int32 nPorStart = *aStartIter;
    ++aStartIter;
    bool bDestroyHint = true;

    // Insert the 1...n new parts of the new attribute:

    while ( aStartIter != aEndIter || bNoLengthAttribute )
    {
        OSL_ENSURE( bNoLengthAttribute || nPorStart < *aStartIter, "AUTOSTYLES: BuildPortion trouble" );

        const sal_Int32 nPorEnd = bNoLengthAttribute ? nPorStart : *aStartIter;
        aInsDelHints.clear();

        // Get all hints that are in [nPorStart, nPorEnd[:
        for ( size_t i = 0; i < Count(); ++i )
        {
            SwTextAttr *pOther = Get(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            const sal_Int32 nOtherStart = pOther->GetStart();

            if ( nOtherStart > nPorStart )
                break;

            if ( pOther->GetEnd() && *pOther->GetEnd() == nPorEnd && nOtherStart == nPorStart )
            {
                OSL_ENSURE( *pOther->GetEnd() == nPorEnd, "AUTOSTYLES: BuildPortion trouble" );
                aInsDelHints.push_back( pOther );
            }
        }

        SwTextAttr* pNewAttr = nullptr;
        if ( RES_TXTATR_CHARFMT == nWhich )
        {
            // pNewHint can be inserted after calculating the sort value.
            // This should ensure, that pNewHint comes behind the already present
            // character style
            sal_uInt16 nCharStyleCount = 0;
            aIter = aInsDelHints.begin();
            while ( aIter != aInsDelHints.end() )
            {
                if ( RES_TXTATR_CHARFMT == (*aIter)->Which() )
                {
                    // #i74589#
                    const SwFormatCharFormat& rOtherCharFormat = (*aIter)->GetCharFormat();
                    const SwFormatCharFormat& rThisCharFormat = rNewHint.GetCharFormat();
                    const bool bSameCharFormat = rOtherCharFormat.GetCharFormat() == rThisCharFormat.GetCharFormat();

                    // #i90311#
                    // Do not remove existing character format hint during XML import
                    if ( !rNode.GetDoc()->IsInXMLImport() &&
                         ( !( SetAttrMode::DONTREPLACE & nMode ) ||
                           bNoLengthAttribute ||
                           bSameCharFormat ) )
                    {
                        // Remove old hint
                        Delete( *aIter );
                        rNode.DestroyAttr( *aIter );
                    }
                    else
                        ++nCharStyleCount;
                }
                else
                {
                    // remove all attributes from auto styles, which are explicitly set in
                    // the new character format:
                    OSL_ENSURE( RES_TXTATR_AUTOFMT == (*aIter)->Which(), "AUTOSTYLES - Misc trouble" );
                    SwTextAttr* pOther = *aIter;
                    std::shared_ptr<SfxItemSet> pOldStyle = static_cast<const SwFormatAutoFormat&>(pOther->GetAttr()).GetStyleHandle();

                    // For each attribute in the automatic style check if it
                    // is also set the new character style:
                    SfxItemSet aNewSet( *pOldStyle->GetPool(),
                        aCharAutoFormatSetRange);
                    SfxItemIter aItemIter( *pOldStyle );
                    const SfxPoolItem* pItem = aItemIter.GetCurItem();
                    while( true )
                    {
                        if ( !CharFormat::IsItemIncluded( pItem->Which(), &rNewHint ) )
                        {
                            aNewSet.Put( *pItem );
                        }

                        if( aItemIter.IsAtEnd() )
                            break;

                        pItem = aItemIter.NextItem();
                    }

                    // Remove old hint
                    Delete( pOther );
                    rNode.DestroyAttr( pOther );

                    // Create new AutoStyle
                    if ( aNewSet.Count() )
                    {
                        pNewAttr = MakeTextAttr( *rNode.GetDoc(),
                                aNewSet, nPorStart, nPorEnd );
                        Insert( pNewAttr );
                        NoteInHistory( pNewAttr, true );
                    }
                }
                ++aIter;
            }

            // If there is no current hint and start and end of rNewHint
            // is ok, we do not need to create a new txtattr.
            if ( nPorStart == nThisStart &&
                 nPorEnd == nThisEnd &&
                 !nCharStyleCount )
            {
                pNewAttr = &rNewHint;
                bDestroyHint = false;
            }
            else
            {
                pNewAttr = MakeTextAttr( *rNode.GetDoc(), rNewHint.GetAttr(),
                        nPorStart, nPorEnd );
                static_txtattr_cast<SwTextCharFormat*>(pNewAttr)->SetSortNumber(nCharStyleCount);
            }
        }
        else
        {
            // Find the current autostyle. Mix attributes if necessary.
            SwTextAttr* pCurrentAutoStyle = nullptr;
            SwTextAttr* pCurrentCharFormat = nullptr;
            aIter = aInsDelHints.begin();
            while ( aIter != aInsDelHints.end() )
            {
                if ( RES_TXTATR_AUTOFMT == (*aIter)->Which() )
                    pCurrentAutoStyle = *aIter;
                else if ( RES_TXTATR_CHARFMT == (*aIter)->Which() )
                    pCurrentCharFormat = *aIter;
                ++aIter;
            }

            std::shared_ptr<SfxItemSet> pNewStyle = static_cast<const SwFormatAutoFormat&>(rNewHint.GetAttr()).GetStyleHandle();
            if ( pCurrentAutoStyle )
            {
                std::shared_ptr<SfxItemSet> pCurrentStyle = static_cast<const SwFormatAutoFormat&>(pCurrentAutoStyle->GetAttr()).GetStyleHandle();

                // Merge attributes
                SfxItemSet aNewSet( *pCurrentStyle );
                aNewSet.Put( *pNewStyle );

                // #i75750# Remove attributes already set at whole paragraph
                // #i81764# This should not be applied for no length attributes!!! <--
                if ( !bNoLengthAttribute && rNode.HasSwAttrSet() && aNewSet.Count() )
                {
                    SfxItemIter aIter2( aNewSet );
                    const SfxPoolItem* pItem = aIter2.GetCurItem();
                    const SfxItemSet& rWholeParaAttrSet = rNode.GetSwAttrSet();

                    do
                    {
                        const SfxPoolItem* pTmpItem = nullptr;
                        if ( SfxItemState::SET == rWholeParaAttrSet.GetItemState( pItem->Which(), false, &pTmpItem ) &&
                             pTmpItem == pItem )
                        {
                            // Do not clear item if the attribute is set in a character format:
                            if ( !pCurrentCharFormat || nullptr == CharFormat::GetItem( *pCurrentCharFormat, pItem->Which() ) )
                                aNewSet.ClearItem( pItem->Which() );
                        }
                    }
                    while (!aIter2.IsAtEnd() && nullptr != (pItem = aIter2.NextItem()));
                }

                // Remove old hint
                Delete( pCurrentAutoStyle );
                rNode.DestroyAttr( pCurrentAutoStyle );

                // Create new AutoStyle
                if ( aNewSet.Count() )
                    pNewAttr = MakeTextAttr( *rNode.GetDoc(), aNewSet,
                            nPorStart, nPorEnd );
            }
            else
            {
                // Remove any attributes which are already set at the whole paragraph:
                bool bOptimizeAllowed = true;

                // #i75750# Remove attributes already set at whole paragraph
                // #i81764# This should not be applied for no length attributes!!! <--
                if ( !bNoLengthAttribute && rNode.HasSwAttrSet() && pNewStyle->Count() )
                {
                    SfxItemSet* pNewSet = nullptr;

                    SfxItemIter aIter2( *pNewStyle );
                    const SfxPoolItem* pItem = aIter2.GetCurItem();
                    const SfxItemSet& rWholeParaAttrSet = rNode.GetSwAttrSet();

                    do
                    {
                        const SfxPoolItem* pTmpItem = nullptr;
                        if ( SfxItemState::SET == rWholeParaAttrSet.GetItemState( pItem->Which(), false, &pTmpItem ) &&
                             pTmpItem == pItem )
                        {
                            // Do not clear item if the attribute is set in a character format:
                            if ( !pCurrentCharFormat || nullptr == CharFormat::GetItem( *pCurrentCharFormat, pItem->Which() ) )
                            {
                                if ( !pNewSet )
                                    pNewSet = pNewStyle->Clone();
                                pNewSet->ClearItem( pItem->Which() );
                            }
                        }
                    }
                    while (!aIter2.IsAtEnd() && nullptr != (pItem = aIter2.NextItem()));

                    if ( pNewSet )
                    {
                        bOptimizeAllowed = false;
                        if ( pNewSet->Count() )
                            pNewStyle = rNode.getIDocumentStyleAccess().getAutomaticStyle( *pNewSet, IStyleAccess::AUTO_STYLE_CHAR );
                        else
                            pNewStyle.reset();

                        delete pNewSet;
                    }
                }

                // Create new AutoStyle
                // If there is no current hint and start and end of rNewHint
                // is ok, we do not need to create a new txtattr.
                if ( bOptimizeAllowed &&
                     nPorStart == nThisStart &&
                     nPorEnd == nThisEnd )
                {
                    pNewAttr = &rNewHint;
                    bDestroyHint = false;
                }
                else if ( pNewStyle.get() )
                {
                    pNewAttr = MakeTextAttr( *rNode.GetDoc(), *pNewStyle,
                            nPorStart, nPorEnd );
                }
            }
        }

        if ( pNewAttr )
        {
            Insert( pNewAttr );
//            if ( bDestroyHint )
                NoteInHistory( pNewAttr, true );
        }

        if ( !bNoLengthAttribute )
        {
            nPorStart = *aStartIter;
            ++aStartIter;
        }
        else
            break;
    }

    if ( bDestroyHint )
        rNode.DestroyAttr( &rNewHint );
}

SwTextAttr* MakeRedlineTextAttr( SwDoc & rDoc, SfxPoolItem & rAttr )
{
    // this is intended _only_ for special-purpose redline attributes!
    switch (rAttr.Which())
    {
        case RES_CHRATR_COLOR:
        case RES_CHRATR_WEIGHT:
        case RES_CHRATR_CJK_WEIGHT:
        case RES_CHRATR_CTL_WEIGHT:
        case RES_CHRATR_POSTURE:
        case RES_CHRATR_CJK_POSTURE:
        case RES_CHRATR_CTL_POSTURE:
        case RES_CHRATR_UNDERLINE:
        case RES_CHRATR_CROSSEDOUT:
        case RES_CHRATR_CASEMAP:
        case RES_CHRATR_BACKGROUND:
            break;
        default:
            assert(!"unsupported redline attribute");
            break;
    }

    // Put new attribute into pool
    // FIXME: this const_cast is evil!
    SfxPoolItem& rNew =
        const_cast<SfxPoolItem&>( rDoc.GetAttrPool().Put( rAttr ) );
    return new SwTextAttrEnd( rNew, 0, 0 );
}

// create new text attribute
SwTextAttr* MakeTextAttr(
    SwDoc & rDoc,
    SfxPoolItem& rAttr,
    sal_Int32 const nStt,
    sal_Int32 const nEnd,
    CopyOrNewType const bIsCopy,
    SwTextNode *const pTextNode )
{
    if ( isCHRATR(rAttr.Which()) )
    {
        // Somebody wants to build a SwTextAttr for a character attribute.
        // Sorry, this is not allowed any longer.
        // You'll get a brand new autostyle attribute:
        SfxItemSet aItemSet( rDoc.GetAttrPool(),
                RES_CHRATR_BEGIN, RES_CHRATR_END );
        aItemSet.Put( rAttr );
        return MakeTextAttr( rDoc, aItemSet, nStt, nEnd );
    }
    else if ( RES_TXTATR_AUTOFMT == rAttr.Which() &&
              static_cast<const SwFormatAutoFormat&>(rAttr).GetStyleHandle()->
                GetPool() != &rDoc.GetAttrPool() )
    {
        // If the attribute is an auto-style which refers to a pool that is
        // different from rDoc's pool, we have to correct this:
        const std::shared_ptr<SfxItemSet> pAutoStyle = static_cast<const SwFormatAutoFormat&>(rAttr).GetStyleHandle();
        std::unique_ptr<const SfxItemSet> pNewSet(
                pAutoStyle->SfxItemSet::Clone( true, &rDoc.GetAttrPool() ));
        SwTextAttr* pNew = MakeTextAttr( rDoc, *pNewSet, nStt, nEnd );
        return pNew;
    }

    // Put new attribute into pool
    // FIXME: this const_cast is evil!
    SfxPoolItem& rNew =
        const_cast<SfxPoolItem&>( rDoc.GetAttrPool().Put( rAttr ) );

    SwTextAttr* pNew = nullptr;
    switch( rNew.Which() )
    {
    case RES_TXTATR_CHARFMT:
        {
            SwFormatCharFormat &rFormatCharFormat = static_cast<SwFormatCharFormat&>(rNew);
            if( !rFormatCharFormat.GetCharFormat() )
            {
                rFormatCharFormat.SetCharFormat( rDoc.GetDfltCharFormat() );
            }

            pNew = new SwTextCharFormat( rFormatCharFormat, nStt, nEnd );
        }
        break;
    case RES_TXTATR_INETFMT:
        pNew = new SwTextINetFormat( static_cast<SwFormatINetFormat&>(rNew), nStt, nEnd );
        break;

    case RES_TXTATR_FIELD:
        pNew = new SwTextField( static_cast<SwFormatField &>(rNew), nStt,
                    rDoc.IsClipBoard() );
        break;

    case RES_TXTATR_ANNOTATION:
        {
            pNew = new SwTextAnnotationField( static_cast<SwFormatField &>(rNew), nStt, rDoc.IsClipBoard() );
            if (bIsCopy == CopyOrNewType::Copy)
            {
                // On copy of the annotation field do not keep the annotated text range by removing
                // the relation to its annotation mark (relation established via annotation field's name).
                // If the annotation mark is also copied, the relation and thus the annotated text range will be reestablished,
                // when the annotation mark is created and inserted into the document.
                const_cast<SwPostItField&>(dynamic_cast<const SwPostItField&>(*(pNew->GetFormatField().GetField()))).SetName(OUString());
            }
        }
        break;

    case RES_TXTATR_INPUTFIELD:
        pNew = new SwTextInputField( static_cast<SwFormatField &>(rNew), nStt, nEnd,
                    rDoc.IsClipBoard() );
        break;

    case RES_TXTATR_FLYCNT:
        {
            // finally, copy the frame format (with content)
            pNew = new SwTextFlyCnt( static_cast<SwFormatFlyCnt&>(rNew), nStt );
            if ( static_cast<const SwFormatFlyCnt &>(rAttr).GetTextFlyCnt() )
            {
                // if it has an existing attr then the format must be copied
                static_cast<SwTextFlyCnt *>(pNew)->CopyFlyFormat( &rDoc );
            }
        }
        break;
    case RES_TXTATR_FTN:
        pNew = new SwTextFootnote( static_cast<SwFormatFootnote&>(rNew), nStt );
        // copy note's SeqNo
        if( static_cast<SwFormatFootnote&>(rAttr).GetTextFootnote() )
            static_cast<SwTextFootnote*>(pNew)->SetSeqNo( static_cast<SwFormatFootnote&>(rAttr).GetTextFootnote()->GetSeqRefNo() );
        break;
    case RES_TXTATR_REFMARK:
        pNew = nStt == nEnd
                ? new SwTextRefMark( static_cast<SwFormatRefMark&>(rNew), nStt )
                : new SwTextRefMark( static_cast<SwFormatRefMark&>(rNew), nStt, &nEnd );
        break;
    case RES_TXTATR_TOXMARK:
        pNew = new SwTextTOXMark( static_cast<SwTOXMark&>(rNew), nStt, &nEnd );
        break;
    case RES_TXTATR_CJK_RUBY:
        pNew = new SwTextRuby( static_cast<SwFormatRuby&>(rNew), nStt, nEnd );
        break;
    case RES_TXTATR_META:
    case RES_TXTATR_METAFIELD:
        pNew = SwTextMeta::CreateTextMeta( rDoc.GetMetaFieldManager(), pTextNode,
                static_cast<SwFormatMeta&>(rNew), nStt, nEnd, bIsCopy == CopyOrNewType::Copy );
        break;
    default:
        assert(RES_TXTATR_AUTOFMT == rNew.Which());
        pNew = new SwTextAttrEnd( rNew, nStt, nEnd );
        break;
    }

    return pNew;
}

SwTextAttr* MakeTextAttr( SwDoc & rDoc, const SfxItemSet& rSet,
                        sal_Int32 nStt, sal_Int32 nEnd )
{
    IStyleAccess& rStyleAccess = rDoc.GetIStyleAccess();
    const std::shared_ptr<SfxItemSet> pAutoStyle = rStyleAccess.getAutomaticStyle( rSet, IStyleAccess::AUTO_STYLE_CHAR );
    SwFormatAutoFormat aNewAutoFormat;
    aNewAutoFormat.SetStyleHandle( pAutoStyle );
    SwTextAttr* pNew = MakeTextAttr( rDoc, aNewAutoFormat, nStt, nEnd );
    return pNew;
}

// delete the text attribute and unregister its item at the pool
void SwTextNode::DestroyAttr( SwTextAttr* pAttr )
{
    if( pAttr )
    {
        // some things need to be done before deleting the formatting attribute
        SwDoc* pDoc = GetDoc();
        switch( pAttr->Which() )
        {
        case RES_TXTATR_FLYCNT:
            {
                SwFrameFormat* pFormat = pAttr->GetFlyCnt().GetFrameFormat();
                if( pFormat )      // set to 0 by Undo?
                    pDoc->getIDocumentLayoutAccess().DelLayoutFormat( pFormat );
            }
            break;

        case RES_CHRATR_HIDDEN:
            SetCalcHiddenCharFlags();
            break;

        case RES_TXTATR_FTN:
            static_cast<SwTextFootnote*>(pAttr)->SetStartNode( nullptr );
            static_cast<SwFormatFootnote&>(pAttr->GetAttr()).InvalidateFootnote();
            break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
            if( !pDoc->IsInDtor() )
            {
                SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pAttr));
                const SwField* pField = pAttr->GetFormatField().GetField();

                //JP 06-08-95: DDE-fields are an exception
                assert(RES_DDEFLD == pField->GetTyp()->Which() ||
                       this == pTextField->GetpTextNode());

                // certain fields must update the SwDoc's calculation flags
                switch( pField->GetTyp()->Which() )
                {
                case RES_HIDDENPARAFLD:
                // HiddenParaField must trigger recalculation of visible flag
                    SetCalcHiddenParaField();
                    SAL_FALLTHROUGH;
                case RES_DBSETNUMBERFLD:
                case RES_GETEXPFLD:
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    if( !pDoc->getIDocumentFieldsAccess().IsNewFieldLst() && GetNodes().IsDocNodes() )
                        pDoc->getIDocumentFieldsAccess().InsDelFieldInFieldLst(false, *pTextField);
                    break;
                case RES_DDEFLD:
                    if (GetNodes().IsDocNodes() && pTextField->GetpTextNode())
                        static_cast<SwDDEFieldType*>(pField->GetTyp())->DecRefCnt();
                    break;
                case RES_POSTITFLD:
                    {
                        const_cast<SwFormatField&>(pAttr->GetFormatField()).Broadcast(
                            SwFormatFieldHint(&pTextField->GetFormatField(), SwFormatFieldHintWhich::REMOVED));
                        break;
                    }
                }
            }
            static_cast<SwFormatField&>(pAttr->GetAttr()).InvalidateField();
            break;

        case RES_TXTATR_TOXMARK:
            static_cast<SwTOXMark&>(pAttr->GetAttr()).InvalidateTOXMark();
            break;

        case RES_TXTATR_REFMARK:
            static_cast<SwFormatRefMark&>(pAttr->GetAttr()).InvalidateRefMark();
            break;

        case RES_TXTATR_META:
        case RES_TXTATR_METAFIELD:
            static_txtattr_cast<SwTextMeta*>(pAttr)->ChgTextNode(nullptr);
            break;

        default:
            break;
        }

        SwTextAttr::Destroy( pAttr, pDoc->GetAttrPool() );
    }
}

SwTextAttr* SwTextNode::InsertItem(
    SfxPoolItem& rAttr,
    const sal_Int32 nStart,
    const sal_Int32 nEnd,
    const SetAttrMode nMode )
{
    // character attributes will be inserted as automatic styles:
    assert( !isCHRATR(rAttr.Which()) && "AUTOSTYLES - "
        "SwTextNode::InsertItem should not be called with character attributes");

    SwTextAttr *const pNew =
        MakeTextAttr(
            *GetDoc(),
            rAttr,
            nStart,
            nEnd,
            (nMode & SetAttrMode::IS_COPY) ? CopyOrNewType::Copy : CopyOrNewType::New,
            this );

    if ( pNew )
    {
        const bool bSuccess( InsertHint( pNew, nMode ) );
        // N.B.: also check that the hint is actually in the hints array,
        // because hints of certain types may be merged after successful
        // insertion, and thus destroyed!
        if (!bSuccess || !m_pSwpHints->Contains( pNew ))
        {
            return nullptr;
        }
    }

    return pNew;
}

// take ownership of pAttr; if insertion fails, delete pAttr
bool SwTextNode::InsertHint( SwTextAttr * const pAttr, const SetAttrMode nMode )
{
    bool bHiddenPara = false;

    assert(pAttr && pAttr->GetStart() <= Len());
    assert(!pAttr->GetEnd() || (*pAttr->GetEnd() <= Len()));

    // translate from SetAttrMode to InsertMode (for hints with CH_TXTATR)
    const SwInsertFlags nInsertFlags =
        (nMode & SetAttrMode::FORCEHINTEXPAND)
        ? (SwInsertFlags::FORCEHINTEXPAND | SwInsertFlags::EMPTYEXPAND)
        : SwInsertFlags::EMPTYEXPAND;

    // need this after TryInsertHint, when pAttr may be deleted
    const sal_Int32 nStart( pAttr->GetStart() );
    const bool bDummyChar( pAttr->HasDummyChar() );
    if (bDummyChar)
    {
        SetAttrMode nInsMode = nMode;
        switch( pAttr->Which() )
        {
        case RES_TXTATR_FLYCNT:
            {
                SwTextFlyCnt *pFly = static_cast<SwTextFlyCnt *>(pAttr);
                SwFrameFormat* pFormat = pAttr->GetFlyCnt().GetFrameFormat();
                if( !(SetAttrMode::NOTXTATRCHR & nInsMode) )
                {
                    // Need to insert char first, because SetAnchor() reads
                    // GetStart().
                    //JP 11.05.98: if the anchor is already set correctly,
                    // fix it after inserting the char, so that clients don't
                    // have to worry about it.
                    const SwFormatAnchor* pAnchor = nullptr;
                    pFormat->GetItemState( RES_ANCHOR, false,
                        reinterpret_cast<const SfxPoolItem**>(&pAnchor) );

                    SwIndex aIdx( this, pAttr->GetStart() );
                    const OUString c(GetCharOfTextAttr(*pAttr));
                    OUString const ins( InsertText(c, aIdx, nInsertFlags) );
                    if (ins.isEmpty())
                    {
                        // do not record deletion of Format!
                        ::sw::UndoGuard const ug(
                                pFormat->GetDoc()->GetIDocumentUndoRedo());
                        DestroyAttr(pAttr);
                        return false; // text node full :(
                    }
                    nInsMode |= SetAttrMode::NOTXTATRCHR;

                    if (pAnchor &&
                        (FLY_AS_CHAR == pAnchor->GetAnchorId()) &&
                        pAnchor->GetContentAnchor() &&
                        pAnchor->GetContentAnchor()->nNode == *this &&
                        pAnchor->GetContentAnchor()->nContent == aIdx )
                    {
                        --const_cast<SwIndex&>(
                            pAnchor->GetContentAnchor()->nContent);
                    }
                }
                pFly->SetAnchor( this );

                // format pointer could have changed in SetAnchor,
                // when copying to other docs!
                pFormat = pAttr->GetFlyCnt().GetFrameFormat();
                SwDoc *pDoc = pFormat->GetDoc();

                // OD 26.06.2003 - allow drawing objects in header/footer.
                // But don't allow control objects in header/footer
                if( RES_DRAWFRMFMT == pFormat->Which() &&
                    pDoc->IsInHeaderFooter( pFormat->GetAnchor().GetContentAnchor()->nNode ) )
                {
                    bool bCheckControlLayer = false;
                    pFormat->CallSwClientNotify(sw::CheckDrawFrameFormatLayerHint(&bCheckControlLayer));
                    if( bCheckControlLayer )
                    {
                        // This should not be allowed, prevent it here.
                        // The dtor of the SwTextAttr does not delete the
                        // char, so delete it explicitly here.
                        if( SetAttrMode::NOTXTATRCHR & nInsMode )
                        {
                            // delete the char from the string
                            assert(CH_TXTATR_BREAKWORD == m_Text[pAttr->GetStart()]
                                || CH_TXTATR_INWORD == m_Text[pAttr->GetStart()]);
                            m_Text = m_Text.replaceAt(pAttr->GetStart(), 1, "");
                            // Update SwIndexes
                            SwIndex aTmpIdx( this, pAttr->GetStart() );
                            Update( aTmpIdx, 1, true );
                        }
                        // do not record deletion of Format!
                        ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                        DestroyAttr( pAttr );
                        return false;
                    }
                }
                break;
            }

        case RES_TXTATR_FTN :
            {
                // Footnotes: create text node and put it into Inserts-section
                SwDoc *pDoc = GetDoc();
                SwNodes &rNodes = pDoc->GetNodes();

                // check that footnote is inserted into body or redline section
                if( StartOfSectionIndex() < rNodes.GetEndOfAutotext().GetIndex() )
                {
                    // This should not be allowed, prevent it here.
                    // The dtor of the SwTextAttr does not delete the
                    // char, so delete it explicitly here.
                    if( SetAttrMode::NOTXTATRCHR & nInsMode )
                    {
                        // delete the char from the string
                        assert(CH_TXTATR_BREAKWORD == m_Text[pAttr->GetStart()]
                            || CH_TXTATR_INWORD == m_Text[pAttr->GetStart()]);
                        m_Text = m_Text.replaceAt(pAttr->GetStart(), 1, "");
                        // Update SwIndexes
                        SwIndex aTmpIdx( this, pAttr->GetStart() );
                        Update( aTmpIdx, 1, true );
                    }
                    DestroyAttr( pAttr );
                    return false;
                }

                // is a new footnote being inserted?
                bool bNewFootnote = nullptr == static_cast<SwTextFootnote*>(pAttr)->GetStartNode();
                if( bNewFootnote )
                {
                    static_cast<SwTextFootnote*>(pAttr)->MakeNewTextSection( GetNodes() );
                    SwRegHistory* pHist = GetpSwpHints()
                        ? GetpSwpHints()->GetHistory() : nullptr;
                    if( pHist )
                        pHist->ChangeNodeIndex( GetIndex() );
                }
                else if ( !GetpSwpHints() || !GetpSwpHints()->IsInSplitNode() )
                {
                    // existing footnote: delete all layout frames of its
                    // footnote section
                    sal_uLong nSttIdx =
                        static_cast<SwTextFootnote*>(pAttr)->GetStartNode()->GetIndex();
                    sal_uLong nEndIdx = rNodes[ nSttIdx++ ]->EndOfSectionIndex();
                    for( ; nSttIdx < nEndIdx; ++nSttIdx )
                    {
                        SwContentNode* pCNd = rNodes[ nSttIdx ]->GetContentNode();
                        if( nullptr != pCNd )
                            pCNd->DelFrames();
                    }
                }

                if( !(SetAttrMode::NOTXTATRCHR & nInsMode) )
                {
                    // must insert first, to prevent identical indexes
                    // that could later prevent insertion into SwDoc's
                    // footnote array
                    SwIndex aNdIdx( this, pAttr->GetStart() );
                    const OUString c(GetCharOfTextAttr(*pAttr));
                    OUString const ins( InsertText(c, aNdIdx, nInsertFlags) );
                    if (ins.isEmpty())
                    {
                        DestroyAttr(pAttr);
                        return false; // text node full :(
                    }
                    nInsMode |= SetAttrMode::NOTXTATRCHR;
                }

                // insert into SwDoc's footnote index array
                SwTextFootnote* pTextFootnote = nullptr;
                if( !bNewFootnote )
                {
                    // moving an existing footnote (e.g. SplitNode)
                    for( size_t n = 0; n < pDoc->GetFootnoteIdxs().size(); ++n )
                        if( pAttr == pDoc->GetFootnoteIdxs()[n] )
                        {
                            // assign new index by removing and re-inserting
                            pTextFootnote = pDoc->GetFootnoteIdxs()[n];
                            pDoc->GetFootnoteIdxs().erase( pDoc->GetFootnoteIdxs().begin() + n );
                            break;
                        }
                        // if the Undo set the StartNode, the Index isn't
                        // in the doc's array yet!
                }
                if( !pTextFootnote )
                    pTextFootnote = static_cast<SwTextFootnote*>(pAttr);

                // to update the numbers and for sorting, the Node must be set
                static_cast<SwTextFootnote*>(pAttr)->ChgTextNode( this );

                // do not insert footnote in redline section into footnote array
                if( StartOfSectionIndex() > rNodes.GetEndOfRedlines().GetIndex() )
                {
                    const bool bSuccess = pDoc->GetFootnoteIdxs().insert(pTextFootnote).second;
                    OSL_ENSURE( bSuccess, "FootnoteIdx not inserted." );
                    (void) bSuccess; // unused in non-debug
                }
                SwNodeIndex aTmpIndex( *this );
                pDoc->GetFootnoteIdxs().UpdateFootnote( aTmpIndex);
                static_cast<SwTextFootnote*>(pAttr)->SetSeqRefNo();
            }
            break;

            case RES_TXTATR_FIELD:
                {
                    // trigger notification for HiddenParaFields
                    if( RES_HIDDENPARAFLD == pAttr->GetFormatField().GetField()->GetTyp()->Which() )
                    {
                        bHiddenPara = true;
                    }
                }
                break;

        }
        // CH_TXTATR_* are inserted for SwTextHints without EndIndex
        // If the caller is SwTextNode::Copy, the char has already been copied,
        // and SETATTR_NOTXTATRCHR prevents inserting it again here.
        if( !(SetAttrMode::NOTXTATRCHR & nInsMode) )
        {
            SwIndex aIdx( this, pAttr->GetStart() );
            OUString const ins( InsertText(OUString(GetCharOfTextAttr(*pAttr)),
                        aIdx, nInsertFlags) );
            if (ins.isEmpty())
            {
                DestroyAttr(pAttr);
                return false; // text node full :(
            }

            // adjust end of hint to account for inserted CH_TXTATR
            sal_Int32 * const pEnd(pAttr->GetEnd());
            if (pEnd)
            {
                *pEnd = *pEnd + 1;
            }
        }
    }

    // handle attributes which provide content
    sal_Int32 nEnd = nStart;
    bool bInputFieldStartCharInserted = false;
    bool bInputFieldEndCharInserted = false;
    const bool bHasContent( pAttr->HasContent() );
    if ( bHasContent )
    {
        switch( pAttr->Which() )
        {
        case RES_TXTATR_INPUTFIELD:
            {
                SwTextInputField* pTextInputField = dynamic_cast<SwTextInputField*>(pAttr);
                if ( pTextInputField )
                {
                    if( !(SetAttrMode::NOTXTATRCHR & nMode) )
                    {
                        SwIndex aIdx( this, pAttr->GetStart() );
                        const OUString aContent = OUStringLiteral1(CH_TXT_ATR_INPUTFIELDSTART)
                            + pTextInputField->GetFieldContent() + OUStringLiteral1(CH_TXT_ATR_INPUTFIELDEND);
                        InsertText( aContent, aIdx, nInsertFlags );

                        sal_Int32* const pEnd(pAttr->GetEnd());
                        OSL_ENSURE( pEnd != nullptr, "<SwTextNode::InsertHint(..)> - missing end of RES_TXTATR_INPUTFIELD!" );
                        if ( pEnd != nullptr )
                        {
                            *pEnd = *pEnd + aContent.getLength();
                            nEnd = *pEnd;
                        }
                    }
                    else
                    {
                        // assure that CH_TXT_ATR_INPUTFIELDSTART and CH_TXT_ATR_INPUTFIELDEND are inserted.
                        if ( m_Text[ pAttr->GetStart() ] != CH_TXT_ATR_INPUTFIELDSTART )
                        {
                            SwIndex aIdx( this, pAttr->GetStart() );
                            InsertText( OUString(CH_TXT_ATR_INPUTFIELDSTART), aIdx, nInsertFlags );
                            bInputFieldStartCharInserted = true;
                            sal_Int32* const pEnd(pAttr->GetEnd());
                            OSL_ENSURE( pEnd != nullptr, "<SwTextNode::InsertHint(..)> - missing end of RES_TXTATR_INPUTFIELD!" );
                            if ( pEnd != nullptr )
                            {
                                *pEnd = *pEnd + 1;
                                nEnd = *pEnd;
                            }
                        }

                        sal_Int32* const pEnd(pAttr->GetEnd());
                        OSL_ENSURE( pEnd != nullptr, "<SwTextNode::InsertHint(..)> - missing end of RES_TXTATR_INPUTFIELD!" );
                        if ( pEnd != nullptr
                             && m_Text[ *(pEnd) - 1 ] != CH_TXT_ATR_INPUTFIELDEND )
                        {
                            SwIndex aIdx( this, *(pEnd) );
                            InsertText( OUString(CH_TXT_ATR_INPUTFIELDEND), aIdx, nInsertFlags );
                            bInputFieldEndCharInserted = true;
                            *pEnd = *pEnd + 1;
                            nEnd = *pEnd;
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    GetOrCreateSwpHints();

    // handle overlap with an existing InputField
    bool bInsertHint = true;
    {
        const SwTextInputField* pTextInputField = GetOverlappingInputField( *pAttr );
        if ( pTextInputField != nullptr )
        {
            if ( pAttr->End() == nullptr )
            {
                bInsertHint = false;
            }
            else
            {
                if ( pAttr->GetStart() > pTextInputField->GetStart() )
                {
                    pAttr->GetStart() = pTextInputField->GetStart();
                }
                if ( *(pAttr->End()) < *(pTextInputField->End()) )
                {
                    *(pAttr->GetEnd()) = *(pTextInputField->End());
                }
            }
        }
    }

    const bool bRet = bInsertHint
                      && m_pSwpHints->TryInsertHint( pAttr, *this, nMode );

    if ( !bRet )
    {
        if ( bDummyChar
             && !(SetAttrMode::NOTXTATRCHR & nMode) )
        {
            // undo insertion of dummy character
            // N.B. cannot insert the dummy character after inserting the hint,
            // because if the hint has no extent it will be moved in InsertText,
            // resulting in infinite recursion
            assert((CH_TXTATR_BREAKWORD == m_Text[nStart] ||
                    CH_TXTATR_INWORD    == m_Text[nStart] ));
            SwIndex aIdx( this, nStart );
            EraseText( aIdx, 1 );
        }

        if ( bHasContent )
        {
            if ( !(SetAttrMode::NOTXTATRCHR & nMode)
                 && (nEnd - nStart) > 0 )
            {
                SwIndex aIdx( this, nStart );
                EraseText( aIdx, (nEnd - nStart) );
            }
            else
            {
                if ( bInputFieldEndCharInserted
                     && (nEnd - nStart) > 0 )
                {
                    SwIndex aIdx( this, nEnd - 1 );
                    EraseText( aIdx, 1 );
                }

                if ( bInputFieldStartCharInserted )
                {
                    SwIndex aIdx( this, nStart );
                    EraseText( aIdx, 1 );
                }
            }
        }
    }

    if ( bHiddenPara )
    {
        SetCalcHiddenParaField();
    }

    return bRet;
}

void SwTextNode::DeleteAttribute( SwTextAttr * const pAttr )
{
    if ( !HasHints() )
    {
        OSL_FAIL("DeleteAttribute called, but text node without hints?");
        return;
    }

    if ( pAttr->HasDummyChar() )
    {
        // copy index!
        const SwIndex aIdx( this, pAttr->GetStart() );
        // erase the CH_TXTATR, which will also delete pAttr
        EraseText( aIdx, 1 );
    }
    else if ( pAttr->HasContent() )
    {
        const SwIndex aIdx( this, pAttr->GetStart() );
        OSL_ENSURE( pAttr->End() != nullptr, "<SwTextNode::DeleteAttribute(..)> - missing End() at <SwTextAttr> instance which has content" );
        EraseText( aIdx, *pAttr->End() - pAttr->GetStart() );
    }
    else
    {
        // create MsgHint before start/end become invalid
        SwUpdateAttr aHint(
            pAttr->GetStart(),
            *pAttr->GetEnd(),
            pAttr->Which());

        m_pSwpHints->Delete( pAttr );
        SwTextAttr::Destroy( pAttr, GetDoc()->GetAttrPool() );
        NotifyClients( nullptr, &aHint );

        TryDeleteSwpHints();
    }
}

//FIXME: this does NOT respect SORT NUMBER (for CHARFMT)!
void SwTextNode::DeleteAttributes(
    const sal_uInt16 nWhich,
    const sal_Int32 nStart,
    const sal_Int32 nEnd )
{
    if ( !HasHints() )
        return;

    for ( size_t nPos = 0; m_pSwpHints && nPos < m_pSwpHints->Count(); ++nPos )
    {
        SwTextAttr * const pTextHt = m_pSwpHints->Get( nPos );
        const sal_Int32 nHintStart = pTextHt->GetStart();
        if (nStart < nHintStart)
        {
            break; // sorted by start
        }
        else if ( (nStart == nHintStart) && (nWhich == pTextHt->Which()) )
        {
            if ( nWhich == RES_CHRATR_HIDDEN  )
            {
                assert(!"hey, that's a CHRATR! how did that get in?");
                SetCalcHiddenCharFlags();
            }
            else if ( nWhich == RES_TXTATR_CHARFMT )
            {
                // Check if character format contains hidden attribute:
                const SwCharFormat* pFormat = pTextHt->GetCharFormat().GetCharFormat();
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == pFormat->GetItemState( RES_CHRATR_HIDDEN, true, &pItem ) )
                    SetCalcHiddenCharFlags();
            }
            // #i75430# Recalc hidden flags if necessary
            else if ( nWhich == RES_TXTATR_AUTOFMT )
            {
                // Check if auto style contains hidden attribute:
                const SfxPoolItem* pHiddenItem = CharFormat::GetItem( *pTextHt, RES_CHRATR_HIDDEN );
                if ( pHiddenItem )
                    SetCalcHiddenCharFlags();
                // for auto styles DeleteAttributes is only called from Undo
                // so it shouldn't need to care about ignore start/end flags
            }

            sal_Int32 const * const pEndIdx = pTextHt->GetEnd();

            if ( pTextHt->HasDummyChar() )
            {
                // copy index!
                const SwIndex aIdx( this, nStart );
                // erase the CH_TXTATR, which will also delete pTextHt
                EraseText( aIdx, 1 );
            }
            else if ( pTextHt->HasContent() )
            {
                const SwIndex aIdx( this, nStart );
                OSL_ENSURE( pTextHt->End() != nullptr, "<SwTextNode::DeleteAttributes(..)> - missing End() at <SwTextAttr> instance which has content" );
                EraseText( aIdx, *pTextHt->End() - nStart );
            }
            else if( *pEndIdx == nEnd )
            {
                // Create MsgHint before Start and End are gone.
                // For HiddenParaFields it's not necessary to call
                // SetCalcHiddenParaField because the dtor does that.
                SwUpdateAttr aHint(
                    nStart,
                    *pEndIdx,
                    nWhich);

                m_pSwpHints->DeleteAtPos( nPos );
                SwTextAttr::Destroy( pTextHt, GetDoc()->GetAttrPool() );
                NotifyClients( nullptr, &aHint );
            }
        }
    }
    TryDeleteSwpHints();
}

void SwTextNode::DelSoftHyph( const sal_Int32 nStt, const sal_Int32 nEnd )
{
    sal_Int32 nFndPos = nStt;
    sal_Int32 nEndPos = nEnd;
    for (;;)
    {
        nFndPos = m_Text.indexOf(CHAR_SOFTHYPHEN, nFndPos);
        if (nFndPos<0 || nFndPos>=nEndPos )
        {
            break;
        }
        const SwIndex aIdx( this, nFndPos );
        EraseText( aIdx, 1 );
        --nEndPos;
    }
}

bool SwTextNode::IsIgnoredCharFormatForNumbering(const sal_uInt16 nWhich)
{
    return (nWhich ==  RES_CHRATR_UNDERLINE || nWhich == RES_CHRATR_COLOR || nWhich == RES_CHRATR_BACKGROUND || nWhich == RES_CHRATR_ESCAPEMENT);
}

//In MS Word, following properties of the paragraph end position won't affect the formatting of bullets, so we ignore them:
//Font underline;
//Font Italic of Western, CJK and CTL;
//Font Bold of Wertern, CJK and CTL;
bool lcl_IsIgnoredCharFormatForBullets(const sal_uInt16 nWhich)
{
    return (nWhich == RES_CHRATR_UNDERLINE || nWhich == RES_CHRATR_POSTURE || nWhich == RES_CHRATR_WEIGHT
        || nWhich == RES_CHRATR_CJK_POSTURE || nWhich == RES_CHRATR_CJK_WEIGHT
        || nWhich == RES_CHRATR_CTL_POSTURE || nWhich == RES_CHRATR_CTL_WEIGHT);
}

//Condition for expanding char set to character style of specified number rule level:
//The item inside the set should not conflict to any exist and non-default item inside paragraph properties set (SwContentNode::SwPAttrSet);
//The node should have applied a number rule;
//The node should be counted in a list, if not, make it to be;
//The item should not conflict to any exist and non-default item inside the character of specified number rule level;
//The item should not be ignored depend on the exact number rule type;
bool SwTextNode::TryCharSetExpandToNum(const SfxItemSet& aCharSet)
{
    bool bRet = false;
    SfxItemIter aIter( aCharSet );
    const SfxPoolItem* pItem = aIter.FirstItem();
    if (!pItem)
        return bRet;
    const sal_uInt16 nWhich = pItem->Which();

    const SfxPoolItem& rInnerItem = GetAttr(nWhich,false);

    if (!IsDefaultItem(&rInnerItem) &&  !IsInvalidItem(&rInnerItem))
        return bRet;

    if (!IsInList() && GetNumRule() && !GetListId().isEmpty())
    {
        return bRet;
    }

    SwNumRule* pCurrNum = GetNumRule(false);

    int nLevel = GetActualListLevel();

    if (nLevel != -1 && pCurrNum)
    {
        const SwNumFormat* pCurrNumFormat = pCurrNum->GetNumFormat(static_cast<sal_uInt16>(nLevel));
        if (pCurrNumFormat)
        {
            if (pCurrNumFormat->IsItemize() && lcl_IsIgnoredCharFormatForBullets(nWhich))
                return bRet;
            if (pCurrNumFormat->IsEnumeration() && SwTextNode::IsIgnoredCharFormatForNumbering(nWhich))
                return bRet;
            SwCharFormat* pCurrCharFormat =pCurrNumFormat->GetCharFormat();

            if (pCurrCharFormat && pCurrCharFormat->GetItemState(nWhich,false) != SfxItemState::SET)
            {
                pCurrCharFormat->SetFormatAttr(*pItem);
                SwNumFormat aNewNumFormat(*pCurrNumFormat);
                aNewNumFormat.SetCharFormat(pCurrCharFormat);
                pCurrNum->Set(nLevel,aNewNumFormat);
                bRet = true;
            }
        }
    }

    return bRet;
}

// Set these attributes on SwTextNode.  If they apply to the entire paragraph
// text, set them in the SwTextNode's item set (SwContentNode::SetAttr).
bool SwTextNode::SetAttr(
    const SfxItemSet& rSet,
    const sal_Int32 nStt,
    const sal_Int32 nEnd,
    const SetAttrMode nMode )
{
    if( !rSet.Count() )
        return false;

    // split sets (for selection in nodes)
    const SfxItemSet* pSet = &rSet;
    SfxItemSet aTextSet( *rSet.GetPool(), RES_TXTATR_BEGIN, RES_TXTATR_END-1 );

    // entire paragraph
    if ( !nStt && (nEnd == m_Text.getLength()) &&
         !(nMode & SetAttrMode::NOFORMATATTR ) )
    {
        // if the node already has CharFormat hints, the new attributes must
        // be set as hints too to override those.
        bool bHasCharFormats = false;
        if ( HasHints() )
        {
            for ( size_t n = 0; n < m_pSwpHints->Count(); ++n )
            {
                if ( m_pSwpHints->Get( n )->IsCharFormatAttr() )
                {
                    bHasCharFormats = true;
                    break;
                }
            }
        }

        if( !bHasCharFormats )
        {
            aTextSet.Put( rSet );
            // If there are any character attributes in rSet,
            // we want to set them at the paragraph:
            if( aTextSet.Count() != rSet.Count() )
            {
                const bool bRet = SetAttr( rSet );
                if( !aTextSet.Count() )
                    return bRet;
            }

            // check for auto style:
            const SfxPoolItem* pItem;
            const bool bAutoStyle = SfxItemState::SET == aTextSet.GetItemState( RES_TXTATR_AUTOFMT, false, &pItem );
            if ( bAutoStyle )
            {
                std::shared_ptr<SfxItemSet> pAutoStyleSet = static_cast<const SwFormatAutoFormat*>(pItem)->GetStyleHandle();
                const bool bRet = SetAttr( *pAutoStyleSet );
                if( 1 == aTextSet.Count() )
                    return bRet;
            }

            // Continue with the text attributes:
            pSet = &aTextSet;
        }
    }

    GetOrCreateSwpHints();

    SfxItemSet aCharSet( *rSet.GetPool(), aCharAutoFormatSetRange );

    size_t nCount = 0;
    SfxItemIter aIter( *pSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();

    do
    {
        if ( pItem && !IsInvalidItem(pItem) )
        {
            const sal_uInt16 nWhich = pItem->Which();
            OSL_ENSURE( isCHRATR(nWhich) || isTXTATR(nWhich),
                    "SwTextNode::SetAttr(): unknown attribute" );
            if ( isCHRATR(nWhich) || isTXTATR(nWhich) )
            {
                if ((RES_TXTATR_CHARFMT == nWhich) &&
                    (GetDoc()->GetDfltCharFormat() ==
                     static_cast<const SwFormatCharFormat*>(pItem)->GetCharFormat()))
                {
                    SwIndex aIndex( this, nStt );
                    RstTextAttr( aIndex, nEnd - nStt, RES_TXTATR_CHARFMT );
                    DontExpandFormat( aIndex );
                }
                else
                {
                    if (isCHRATR(nWhich) ||
                        (RES_TXTATR_UNKNOWN_CONTAINER == nWhich))
                    {
                        aCharSet.Put( *pItem );
                    }
                    else
                    {

                        SwTextAttr *const pNew = MakeTextAttr( *GetDoc(),
                                const_cast<SfxPoolItem&>(*pItem), nStt, nEnd );
                        if ( pNew )
                        {
                            if ( nEnd != nStt && !pNew->GetEnd() )
                            {
                                OSL_FAIL("Attribut without end, but area marked");
                                DestroyAttr( pNew ); // do not insert
                            }
                            else if ( InsertHint( pNew, nMode ) )
                            {
                                ++nCount;
                            }
                        }
                    }
                }
            }
        }
        if ( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    } while( true );

    if ( aCharSet.Count() )
    {
        SwTextAttr* pTmpNew = MakeTextAttr( *GetDoc(), aCharSet, nStt, nEnd );
        if ( InsertHint( pTmpNew, nMode ) )
        {
            ++nCount;
        }
    }

    TryDeleteSwpHints();

    return nCount != 0;
}

static void lcl_MergeAttr( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    if ( RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pCFSet = CharFormat::GetItemSet( rAttr );
        if ( !pCFSet )
            return;
        SfxWhichIter aIter( *pCFSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( ( nWhich < RES_CHRATR_END ||
                  RES_TXTATR_UNKNOWN_CONTAINER == nWhich ) &&
                ( SfxItemState::SET == pCFSet->GetItemState( nWhich ) ) )
                rSet.Put( pCFSet->Get( nWhich ) );
            nWhich = aIter.NextWhich();
        }
    }
    else
        rSet.Put( rAttr );
}

static void lcl_MergeAttr_ExpandChrFormat( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    if( RES_TXTATR_CHARFMT == rAttr.Which() ||
        RES_TXTATR_INETFMT == rAttr.Which() ||
        RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pCFSet = CharFormat::GetItemSet( rAttr );

        if ( pCFSet )
        {
            SfxWhichIter aIter( *pCFSet );
            sal_uInt16 nWhich = aIter.FirstWhich();
            while( nWhich )
            {
                if( ( nWhich < RES_CHRATR_END ||
                      ( RES_TXTATR_AUTOFMT == rAttr.Which() && RES_TXTATR_UNKNOWN_CONTAINER == nWhich ) ) &&
                    ( SfxItemState::SET == pCFSet->GetItemState( nWhich ) ) )
                    rSet.Put( pCFSet->Get( nWhich ) );
                nWhich = aIter.NextWhich();
            }
        }
    }

/* If multiple attributes overlap, the last one wins!
   Probably this can only happen between a RES_TXTATR_INETFMT and one of the
   other hints, because BuildPortions ensures that CHARFMT/AUTOFMT don't
   overlap.  But there may be multiple CHARFMT/AUTOFMT with exactly the same
   start/end, sorted by BuildPortions, in which case the same logic applies.

            1234567890123456789
              |------------|        Font1
                 |------|           Font2
                    ^  ^
                    |--|        query range: -> Font2
*/
    // merge into set
    rSet.Put( rAttr );
}

struct SwPoolItemEndPair
{
public:
    const SfxPoolItem* mpItem;
    sal_Int32 mnEndPos;

    SwPoolItemEndPair() : mpItem( nullptr ), mnEndPos( 0 ) {};
};

static void lcl_MergeListLevelIndentAsLRSpaceItem( const SwTextNode& rTextNode,
                                            SfxItemSet& rSet )
{
    if ( rTextNode.AreListLevelIndentsApplicable() )
    {
        const SwNumRule* pRule = rTextNode.GetNumRule();
        if ( pRule && rTextNode.GetActualListLevel() >= 0 )
        {
            const SwNumFormat& rFormat = pRule->Get(static_cast<sal_uInt16>(rTextNode.GetActualListLevel()));
            if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextLeft( rFormat.GetIndentAt() );
                aLR.SetTextFirstLineOfst( static_cast<short>(rFormat.GetFirstLineIndent()) );
                rSet.Put( aLR );
            }
        }
    }
}

// erfrage die Attribute vom TextNode ueber den Bereich
bool SwTextNode::GetAttr( SfxItemSet& rSet, sal_Int32 nStt, sal_Int32 nEnd,
                         const bool bOnlyTextAttr, const bool bGetFromChrFormat,
                         const bool bMergeIndentValuesOfNumRule ) const
{
    if( HasHints() )
    {
        // First, check which text attributes are valid in the range.
        // cases:
        // Ambiguous, if
        //  * the attribute is wholly contained in the range
        //  * the attribute end is in the range
        //  * the attribute start is in the range
        // Unambiguous (merge into set), if
        //  * the attribute wholly contains the range
        // Ignored, if
        //  * the attribute is wholly outside the range

        void (*fnMergeAttr)( SfxItemSet&, const SfxPoolItem& )
            = bGetFromChrFormat ? &lcl_MergeAttr_ExpandChrFormat
                             : &lcl_MergeAttr;

        // get the node's automatic attributes
        SfxItemSet aFormatSet( *rSet.GetPool(), rSet.GetRanges() );
        if( !bOnlyTextAttr )
        {
            SwContentNode::GetAttr( aFormatSet );
            if ( bMergeIndentValuesOfNumRule )
            {
                lcl_MergeListLevelIndentAsLRSpaceItem( *this, aFormatSet );
            }
        }

        const size_t nSize = m_pSwpHints->Count();

        if (nStt == nEnd)               // no range:
        {
            for (size_t n = 0; n < nSize; ++n)
            {
                const SwTextAttr* pHt = m_pSwpHints->Get(n);
                const sal_Int32 nAttrStart = pHt->GetStart();
                if (nAttrStart > nEnd)         // behind the range
                    break;

                const sal_Int32* pAttrEnd = pHt->End();
                if ( ! pAttrEnd ) // no attributes without end
                    continue;

                if( ( nAttrStart < nStt &&
                        ( pHt->DontExpand() ? nStt < *pAttrEnd
                                            : nStt <= *pAttrEnd )) ||
                    ( nStt == nAttrStart &&
                        ( nAttrStart == *pAttrEnd || !nStt )))
                    (*fnMergeAttr)( rSet, pHt->GetAttr() );
            }
        }
        else                            // a query range is defined
        {
            // #i75299#
            std::unique_ptr< std::vector< SwPoolItemEndPair > > pAttrArr;

            const size_t coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN;

            for (size_t n = 0; n < nSize; ++n)
            {
                const SwTextAttr* pHt = m_pSwpHints->Get(n);
                const sal_Int32 nAttrStart = pHt->GetStart();
                if (nAttrStart > nEnd)         // outside, behind
                    break;

                const sal_Int32* pAttrEnd = pHt->End();
                if ( ! pAttrEnd ) // no attributes without end
                    continue;

                bool bChkInvalid = false;
                if (nAttrStart <= nStt)        // before or exactly Start
                {
                    if (*pAttrEnd <= nStt)     // outside, before
                        continue;

                    if (nEnd <= *pAttrEnd)     // behind or exactly End
                        (*fnMergeAttr)( aFormatSet, pHt->GetAttr() );
                    else
//                  else if( pHt->GetAttr() != aFormatSet.Get( pHt->Which() ) )
                        // ambiguous
                        bChkInvalid = true;
                }
                else if (nAttrStart < nEnd     // starts in the range
)//                      && pHt->GetAttr() != aFormatSet.Get( pHt->Which() ) )
                    bChkInvalid = true;

                if( bChkInvalid )
                {
                    // ambiguous?
                    std::unique_ptr< SfxItemIter > pItemIter;
                    const SfxPoolItem* pItem = nullptr;

                    if ( RES_TXTATR_AUTOFMT == pHt->Which() )
                    {
                        const SfxItemSet* pAutoSet = CharFormat::GetItemSet( pHt->GetAttr() );
                        if ( pAutoSet )
                        {
                            pItemIter.reset( new SfxItemIter( *pAutoSet ) );
                            pItem = pItemIter->GetCurItem();
                        }
                    }
                    else
                        pItem = &pHt->GetAttr();

                    const sal_Int32 nHintEnd = *pAttrEnd;

                    while ( pItem )
                    {
                        const sal_uInt16 nHintWhich = pItem->Which();
                        OSL_ENSURE(!isUNKNOWNATR(nHintWhich),
                                "SwTextNode::GetAttr(): unknown attribute?");

                        if ( !pAttrArr.get() )
                        {
                            pAttrArr.reset(
                                new std::vector< SwPoolItemEndPair >(coArrSz));
                        }

                        std::vector< SwPoolItemEndPair >::iterator pPrev = pAttrArr->begin();
                        if (isCHRATR(nHintWhich) ||
                            isTXTATR_WITHEND(nHintWhich))
                        {
                            pPrev += nHintWhich - RES_CHRATR_BEGIN;
                        }
                        else
                        {
                            pPrev = pAttrArr->end();
                        }

                        if( pPrev != pAttrArr->end() )
                        {
                            if( !pPrev->mpItem )
                            {
                                if ( bOnlyTextAttr || *pItem != aFormatSet.Get( nHintWhich ) )
                                {
                                    if( nAttrStart > nStt )
                                    {
                                        rSet.InvalidateItem( nHintWhich );
                                        pPrev->mpItem = INVALID_POOL_ITEM;
                                    }
                                    else
                                    {
                                        pPrev->mpItem = pItem;
                                        pPrev->mnEndPos = nHintEnd;
                                    }
                                }
                            }
                            else if( !IsInvalidItem(pPrev->mpItem) )
                            {
                                if( pPrev->mnEndPos == nAttrStart &&
                                    *pPrev->mpItem == *pItem )
                                {
                                    pPrev->mpItem = pItem;
                                    pPrev->mnEndPos = nHintEnd;
                                }
                                else
                                {
                                    rSet.InvalidateItem( nHintWhich );
                                    pPrev->mpItem = INVALID_POOL_ITEM;
                                }
                            }
                        }

                        pItem = ( pItemIter.get() && !pItemIter->IsAtEnd() )
                                    ? pItemIter->NextItem() : nullptr;
                    } // end while
                }
            }

            if ( pAttrArr.get() )
            {
                for (size_t n = 0; n < coArrSz; ++n)
                {
                    const SwPoolItemEndPair& rItemPair = (*pAttrArr)[ n ];
                    if( rItemPair.mpItem && !IsInvalidItem(rItemPair.mpItem) )
                    {
                        const sal_uInt16 nWh =
                            static_cast<sal_uInt16>(n + RES_CHRATR_BEGIN);

                        if (nEnd <= rItemPair.mnEndPos) // behind or exactly end
                        {
                            if( *rItemPair.mpItem != aFormatSet.Get( nWh ) )
                                (*fnMergeAttr)( rSet, *rItemPair.mpItem );
                        }
                        else
                            // ambiguous
                            rSet.InvalidateItem( nWh );
                    }
                }
            }
        }
        if( aFormatSet.Count() )
        {
            // remove all from the format-set that are also set in the text-set
            aFormatSet.Differentiate( rSet );
            // now "merge" everything
            rSet.Put( aFormatSet );
        }
    }
    else if( !bOnlyTextAttr )
    {
        // get the node's automatic attributes
        SwContentNode::GetAttr( rSet );
        if ( bMergeIndentValuesOfNumRule )
        {
            lcl_MergeListLevelIndentAsLRSpaceItem( *this, rSet );
        }
    }

    return rSet.Count() != 0;
}

namespace
{

typedef std::pair<sal_Int32, sal_Int32> AttrSpan_t;
typedef std::multimap<AttrSpan_t, const SwTextAttr*> AttrSpanMap_t;

struct IsAutoStyle
{
    bool
    operator()(const AttrSpanMap_t::value_type& i_rAttrSpan)
    const
    {
        return i_rAttrSpan.second && i_rAttrSpan.second->Which() == RES_TXTATR_AUTOFMT;
    }
};

/** Removes from io_rAttrSet all items that are set by style on the
    given span.
  */
struct RemovePresentAttrs
{
    explicit RemovePresentAttrs(SfxItemSet& io_rAttrSet)
        : m_rAttrSet(io_rAttrSet)
    {
    }

    void
    operator()(const AttrSpanMap_t::value_type& i_rAttrSpan)
    const
    {
        if (!i_rAttrSpan.second)
        {
            return;
        }

        const SwTextAttr* const pAutoStyle(i_rAttrSpan.second);
        SfxItemIter aIter(m_rAttrSet);
        const SfxPoolItem* pItem(aIter.GetCurItem());
        while (pItem)
        {
            const sal_uInt16 nWhich(pItem->Which());
            if (CharFormat::IsItemIncluded(nWhich, pAutoStyle))
            {
                m_rAttrSet.ClearItem(nWhich);
            }

            if (aIter.IsAtEnd())
            {
                break;
            }
            pItem = aIter.NextItem();
        }
    }

private:
    SfxItemSet& m_rAttrSet;
};

/** Collects all style-covered spans from i_rHints to o_rSpanMap. In
    addition inserts dummy spans with pointer to format equal to 0 for
    all gaps (i.e. spans not covered by any style). This simplifies
    creation of autostyles for all needed spans, but it means all code
    that tries to access the pointer has to check if it's non-null!
 */
void
lcl_CollectHintSpans(const SwpHints& i_rHints, const sal_Int32 nLength,
        AttrSpanMap_t& o_rSpanMap)
{
    sal_Int32 nLastEnd(0);

    for (size_t i = 0; i < i_rHints.Count(); ++i)
    {
        const SwTextAttr* pHint = i_rHints.Get(i);
        const sal_uInt16 nWhich(pHint->Which());
        if (nWhich == RES_TXTATR_CHARFMT || nWhich == RES_TXTATR_AUTOFMT)
        {
            const AttrSpan_t aSpan(pHint->GetStart(), *pHint->End());
            o_rSpanMap.insert(AttrSpanMap_t::value_type(aSpan, pHint));

            // < not != because there may be multiple CHARFMT at same range
            if (nLastEnd < aSpan.first)
            {
                // insert dummy span covering the gap
                o_rSpanMap.insert(AttrSpanMap_t::value_type(
                    AttrSpan_t(nLastEnd, aSpan.first), nullptr));
            }

            nLastEnd = aSpan.second;
        }
    }

    // no hints at the end (special case: no hints at all in i_rHints)
    if (nLastEnd != nLength && nLength != 0)
    {
        o_rSpanMap.insert(
            AttrSpanMap_t::value_type(AttrSpan_t(nLastEnd, nLength), nullptr));
    }
}

void
lcl_FillWhichIds(const SfxItemSet& i_rAttrSet, std::vector<sal_uInt16>& o_rClearIds)
{
    o_rClearIds.reserve(i_rAttrSet.Count());
    SfxItemIter aIter(i_rAttrSet);
    const SfxPoolItem* pItem(aIter.GetCurItem());
    while (pItem)
    {
        o_rClearIds.push_back(pItem->Which());

        if (aIter.IsAtEnd())
        {
            break;
        }
        pItem = aIter.NextItem();
    }
}

struct SfxItemSetClearer
{
    SfxItemSet & m_rItemSet;
    explicit SfxItemSetClearer(SfxItemSet & rItemSet) : m_rItemSet(rItemSet) { }
    void operator()(sal_uInt16 const nWhich) { m_rItemSet.ClearItem(nWhich); }
};

}

/** Does the hard work of SwTextNode::FormatToTextAttr: the real conversion
    of items to automatic styles.
 */
void
SwTextNode::impl_FormatToTextAttr(const SfxItemSet& i_rAttrSet)
{
    typedef AttrSpanMap_t::iterator AttrSpanMap_iterator_t;
    AttrSpanMap_t aAttrSpanMap;

    if (i_rAttrSet.Count() == 0)
    {
        return;
    }

    // 1. Identify all spans in hints' array

    lcl_CollectHintSpans(*m_pSwpHints, m_Text.getLength(), aAttrSpanMap);

    // 2. Go through all spans and insert new attrs

    AttrSpanMap_iterator_t aCurRange(aAttrSpanMap.begin());
    const AttrSpanMap_iterator_t aEnd(aAttrSpanMap.end());
    while (aCurRange != aEnd)
    {
        typedef std::pair<AttrSpanMap_iterator_t, AttrSpanMap_iterator_t>
            AttrSpanMapRange_t;
        AttrSpanMapRange_t aRange(aAttrSpanMap.equal_range(aCurRange->first));

        // 2a. Collect attributes to insert

        SfxItemSet aCurSet(i_rAttrSet);
        std::for_each(aRange.first, aRange.second, RemovePresentAttrs(aCurSet));

        // 2b. Insert automatic style containing the collected attributes

        if (aCurSet.Count() != 0)
        {
            AttrSpanMap_iterator_t aAutoStyleIt(
                    std::find_if(aRange.first, aRange.second, IsAutoStyle()));
            if (aAutoStyleIt != aRange.second)
            {
                // there already is an automatic style on that span:
                // create new one and remove the original one
                SwTextAttr* const pAutoStyle(const_cast<SwTextAttr*>(aAutoStyleIt->second));
                const std::shared_ptr<SfxItemSet> pOldStyle(
                        static_cast<const SwFormatAutoFormat&>(
                            pAutoStyle->GetAttr()).GetStyleHandle());
                aCurSet.Put(*pOldStyle);

                // remove the old hint
                m_pSwpHints->Delete(pAutoStyle);
                DestroyAttr(pAutoStyle);
            }
            m_pSwpHints->Insert(
                    MakeTextAttr(*GetDoc(), aCurSet,
                        aCurRange->first.first, aCurRange->first.second));
        }

        aCurRange = aRange.second;
    }

    // hints were directly inserted, so need to fix the Ignore flags now
    m_pSwpHints->MergePortions(*this);

    // 3. Clear items from the node
    std::vector<sal_uInt16> aClearedIds;
    lcl_FillWhichIds(i_rAttrSet, aClearedIds);
    ClearItemsFromAttrSet(aClearedIds);
}

void SwTextNode::FormatToTextAttr( SwTextNode* pNd )
{
    SfxItemSet aThisSet( GetDoc()->GetAttrPool(), aCharFormatSetRange );
    if( HasSwAttrSet() && GetpSwAttrSet()->Count() )
        aThisSet.Put( *GetpSwAttrSet() );

    GetOrCreateSwpHints();

    if( pNd == this )
    {
        impl_FormatToTextAttr(aThisSet);
    }
    else
    {
        // There are five possible combinations of items from this and
        // pNd (pNd is the 'main' node):

        //  case    pNd     this     action

        //     1     -       -      do nothing
        //     2     -       a      convert item to attr of this
        //     3     a       -      convert item to attr of pNd
        //     4     a       a      clear item in this
        //     5     a       b      convert item to attr of this

        SfxItemSet aNdSet( pNd->GetDoc()->GetAttrPool(), aCharFormatSetRange );
        if( pNd->HasSwAttrSet() && pNd->GetpSwAttrSet()->Count() )
            aNdSet.Put( *pNd->GetpSwAttrSet() );

        pNd->GetOrCreateSwpHints();

        std::vector<sal_uInt16> aProcessedIds;

        if( aThisSet.Count() )
        {
            SfxItemIter aIter( aThisSet );
            const SfxPoolItem* pItem = aIter.GetCurItem(), *pNdItem = nullptr;
            SfxItemSet aConvertSet( GetDoc()->GetAttrPool(), aCharFormatSetRange );
            std::vector<sal_uInt16> aClearWhichIds;

            while( true )
            {
                if( SfxItemState::SET == aNdSet.GetItemState( pItem->Which(), false, &pNdItem ) )
                {
                    if (*pItem == *pNdItem) // 4
                    {
                        aClearWhichIds.push_back( pItem->Which() );
                    }
                    else    // 5
                    {
                        aConvertSet.Put(*pItem);
                    }
                    aProcessedIds.push_back(pItem->Which());
                }
                else    // 2
                {
                    aConvertSet.Put(*pItem);
                }

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }

            // 4/ clear items of this that are set with the same value on pNd
            ClearItemsFromAttrSet( aClearWhichIds );

            // 2, 5/ convert all other items to attrs
            impl_FormatToTextAttr(aConvertSet);
        }

        {
            std::for_each(aProcessedIds.begin(), aProcessedIds.end(),
                    SfxItemSetClearer(aNdSet));

            // 3/ convert items to attrs
            pNd->impl_FormatToTextAttr(aNdSet);

            if( aNdSet.Count() )
            {
                SwFormatChg aTmp1( pNd->GetFormatColl() );
                pNd->NotifyClients( &aTmp1, &aTmp1 );
            }
        }
    }

    SetCalcHiddenCharFlags();

    pNd->TryDeleteSwpHints();
}

void SwpHints::CalcFlags()
{
    m_bDDEFields = m_bFootnote = false;
    const size_t nSize = Count();
    for( size_t nPos = 0; nPos < nSize; ++nPos )
    {
        const SwTextAttr* pAttr = Get( nPos );
        switch( pAttr->Which() )
        {
        case RES_TXTATR_FTN:
            m_bFootnote = true;
            if ( m_bDDEFields )
                return;
            break;
        case RES_TXTATR_FIELD:
            {
                const SwField* pField = pAttr->GetFormatField().GetField();
                if( RES_DDEFLD == pField->GetTyp()->Which() )
                {
                    m_bDDEFields = true;
                    if ( m_bFootnote )
                        return;
                }
            }
            break;
        }
    }
}

bool SwpHints::CalcHiddenParaField()
{
    m_bCalcHiddenParaField = false;
    bool bOldHasHiddenParaField = m_bHasHiddenParaField;
    bool bNewHasHiddenParaField  = false;
    const size_t nSize = Count();
    const SwTextAttr *pTextHt;

    for( size_t nPos = 0; nPos < nSize; ++nPos )
    {
        pTextHt = Get(nPos);
        const sal_uInt16 nWhich = pTextHt->Which();

        if( RES_TXTATR_FIELD == nWhich )
        {
            const SwFormatField& rField = pTextHt->GetFormatField();
            if( RES_HIDDENPARAFLD == rField.GetField()->GetTyp()->Which() )
            {
                if( !static_cast<const SwHiddenParaField*>(rField.GetField())->IsHidden() )
                {
                    SetHiddenParaField(false);
                    return bOldHasHiddenParaField != bNewHasHiddenParaField;
                }
                else
                {
                    bNewHasHiddenParaField = true;
                }
            }
        }
    }
    SetHiddenParaField( bNewHasHiddenParaField );
    return bOldHasHiddenParaField != bNewHasHiddenParaField;
}

void SwpHints::NoteInHistory( SwTextAttr *pAttr, const bool bNew )
{
    if ( m_pHistory ) { m_pHistory->AddHint( pAttr, bNew ); }
}

bool SwpHints::MergePortions( SwTextNode& rNode )
{
    if ( !Count() )
        return false;

    // sort before merging
    Resort();

    bool bRet = false;
    typedef std::multimap< int, std::pair<SwTextAttr*, bool> > PortionMap;
    PortionMap aPortionMap;
    std::map<int, bool> RsidOnlyAutoFormatFlagMap;
    sal_Int32 nLastPorStart = COMPLETE_STRING;
    int nKey = 0;

    // get portions by start position:
    for ( size_t i = 0; i < Count(); ++i )
    {
        SwTextAttr *pHt = Get( i );
        if ( RES_TXTATR_CHARFMT != pHt->Which() &&
             RES_TXTATR_AUTOFMT != pHt->Which() )
             //&&
             //RES_TXTATR_INETFMT != pHt->Which() )
            continue;

        bool isRsidOnlyAutoFormat(false);
        // check for RSID-only AUTOFMT
        if (RES_TXTATR_AUTOFMT == pHt->Which())
        {
            std::shared_ptr<SfxItemSet> const pSet(
                    pHt->GetAutoFormat().GetStyleHandle());
            if ((pSet->Count() == 1) && pSet->GetItem(RES_CHRATR_RSID, false))
            {
                // fdo#70201: eliminate no-extent RSID-only AUTOFMT
                // could be produced by ReplaceText or (maybe?) RstAttr
                if (pHt->GetStart() == *pHt->GetEnd())
                {
                    DeleteAtPos(i); // kill it without History!
                    SwTextAttr::Destroy(pHt, rNode.GetDoc()->GetAttrPool());
                    --i;
                    continue;
                }
                // fdo#52028: this one has _only_ RSID => ignore it completely
                if (!pHt->IsFormatIgnoreStart() || !pHt->IsFormatIgnoreEnd())
                {
                    NoteInHistory(pHt);
                    pHt->SetFormatIgnoreStart(true);
                    pHt->SetFormatIgnoreEnd  (true);
                    NoteInHistory(pHt, true);
                }
                isRsidOnlyAutoFormat = true;
            }
        }

        if (pHt->GetStart() == *pHt->GetEnd())
        {
            // no-length hints are a disease. ignore them here.
            // the SwAttrIter::SeekFwd will not call Rst/Chg for them.
            continue;
        }

        const sal_Int32 nPorStart = pHt->GetStart();
        if (nPorStart != nLastPorStart)
            ++nKey;
        nLastPorStart = nPorStart;
        aPortionMap.insert(std::make_pair(nKey,
                            std::make_pair(pHt, isRsidOnlyAutoFormat)));
        RsidOnlyAutoFormatFlagMap[nKey] = isRsidOnlyAutoFormat;
    }

    // check if portion i can be merged with portion i+1:
    // note: need to include i=0 to set IgnoreStart and j=nKey+1 to reset
    // IgnoreEnd at first / last portion
    int i = 0;
    int j = i + 1;
    while ( i <= nKey )
    {
        std::pair< PortionMap::iterator, PortionMap::iterator > aRange1 = aPortionMap.equal_range( i );
        std::pair< PortionMap::iterator, PortionMap::iterator > aRange2 = aPortionMap.equal_range( j );
        PortionMap::iterator aIter1 = aRange1.first;
        PortionMap::iterator aIter2 = aRange2.first;

        enum { MATCH, DIFFER_ONLY_RSID, DIFFER } eMerge(MATCH);
        size_t const nAttributesInPor1 = std::distance(aRange1.first, aRange1.second);
        size_t const nAttributesInPor2 = std::distance(aRange2.first, aRange2.second);
        bool const isRsidOnlyAutoFormat1(RsidOnlyAutoFormatFlagMap[i]);
        bool const isRsidOnlyAutoFormat2(RsidOnlyAutoFormatFlagMap[j]);

        // if both have one they could be equal, but not if only one has it
        bool const bSkipRsidOnlyAutoFormat(nAttributesInPor1 != nAttributesInPor2);

        // this loop needs to handle the case where one has a CHARFMT and the
        // other CHARFMT + RSID-only AUTOFMT, so...
        // want to skip over RSID-only AUTOFMT here, hence the -1
        if ((nAttributesInPor1 - ((isRsidOnlyAutoFormat1) ? 1 : 0)) ==
            (nAttributesInPor2 - ((isRsidOnlyAutoFormat2) ? 1 : 0))
            && (nAttributesInPor1 != 0 || nAttributesInPor2 != 0))
        {
            // _if_ there is one element more either in aRange1 or aRange2
            // it _must_ be an RSID-only AUTOFMT, which can be ignored here...
            // But if both have RSID-only AUTOFMT they could be equal, no skip!
            while (aIter1 != aRange1.second || aIter2 != aRange2.second)
            {
                // first of all test if there's no gap (before skipping stuff!)
                if (aIter1 != aRange1.second && aIter2 != aRange2.second &&
                    *aIter1->second.first->End() < aIter2->second.first->GetStart())
                {
                    eMerge = DIFFER;
                    break;
                }
                // skip it - cannot be equal if bSkipRsidOnlyAutoFormat is set
                if (bSkipRsidOnlyAutoFormat
                    && aIter1 != aRange1.second && aIter1->second.second)
                {
                    assert(DIFFER != eMerge);
                    eMerge = DIFFER_ONLY_RSID;
                    ++aIter1;
                    continue;
                }
                if (bSkipRsidOnlyAutoFormat
                    && aIter2 != aRange2.second && aIter2->second.second)
                {
                    assert(DIFFER != eMerge);
                    eMerge = DIFFER_ONLY_RSID;
                    ++aIter2;
                    continue;
                }
                assert(aIter1 != aRange1.second && aIter2 != aRange2.second);
                SwTextAttr const*const p1 = aIter1->second.first;
                SwTextAttr const*const p2 = aIter2->second.first;
                if (p1->Which() != p2->Which())
                {
                    eMerge = DIFFER;
                    break;
                }
                if (!(*p1 == *p2))
                {
                    // fdo#52028: for auto styles, check if they differ only
                    // in the RSID, which should have no effect on text layout
                    if (RES_TXTATR_AUTOFMT == p1->Which())
                    {
                        SfxItemSet set1(*p1->GetAutoFormat().GetStyleHandle());
                        SfxItemSet set2(*p2->GetAutoFormat().GetStyleHandle());

                        set1.ClearItem(RES_CHRATR_RSID);
                        set2.ClearItem(RES_CHRATR_RSID);

                        // sadly SfxItemSet::operator== does not seem to work?
                        SfxItemIter iter1(set1);
                        SfxItemIter iter2(set2);
                        if (set1.Count() == set2.Count())
                        {
                            for (SfxPoolItem const* pItem1 = iter1.FirstItem(),
                                                  * pItem2 = iter2.FirstItem();
                                 pItem1 && pItem2;
                                 pItem1 = iter1.NextItem(),
                                 pItem2 = iter2.NextItem())
                            {
                                if (pItem1 != pItem2 ||
                                    pItem1->Which() != pItem2->Which() ||
                                    *pItem1 != *pItem2)
                                {
                                    eMerge = DIFFER;
                                    break;
                                }
                                if (iter1.IsAtEnd())
                                {
                                    assert(iter2.IsAtEnd());
                                    eMerge = DIFFER_ONLY_RSID;
                                }
                            }
                            if (DIFFER == eMerge)
                                break; // outer loop too
                        }
                        else
                        {
                            eMerge = DIFFER;
                            break;
                        }
                    }
                    else
                    {
                        eMerge = DIFFER;
                        break;
                    }
                }
                ++aIter1;
                ++aIter2;
            }
        }
        else
        {
            eMerge = DIFFER;
        }

        if (MATCH == eMerge)
        {
            // important: delete second range so any IgnoreStart on the first
            // range is still valid
            // erase all elements with key i + 1
            sal_Int32 nNewPortionEnd = 0;
            for ( aIter2 = aRange2.first; aIter2 != aRange2.second; ++aIter2 )
            {
                SwTextAttr *const p2 = aIter2->second.first;
                nNewPortionEnd = *p2->GetEnd();

                const size_t nCountBeforeDelete = Count();
                Delete( p2 );

                // robust: check if deletion actually took place before destroying attribute:
                if ( Count() < nCountBeforeDelete )
                    rNode.DestroyAttr( p2 );
            }
            aPortionMap.erase( aRange2.first, aRange2.second );
            ++j;

            // change all attributes with key i
            aRange1 = aPortionMap.equal_range( i );
            for ( aIter1 = aRange1.first; aIter1 != aRange1.second; ++aIter1 )
            {
                SwTextAttr *const p1 = aIter1->second.first;
                NoteInHistory( p1 );
                *p1->GetEnd() = nNewPortionEnd;
                NoteInHistory( p1, true );
                bRet = true;
            }

            if (bRet)
            {
                Resort();
            }
        }
        else
        {
            // when not merging the ignore flags need to be either set or reset
            // (reset too in case one of the autofmts was recently changed)
            bool const bSetIgnoreFlag(DIFFER_ONLY_RSID == eMerge);
            for (aIter1 = aRange1.first; aIter1 != aRange1.second; ++aIter1)
            {
                if (!aIter1->second.second) // already set above, don't change
                {
                    SwTextAttr *const pCurrent(aIter1->second.first);
                    if (pCurrent->IsFormatIgnoreEnd() != bSetIgnoreFlag)
                    {
                        NoteInHistory(pCurrent);
                        pCurrent->SetFormatIgnoreEnd(bSetIgnoreFlag);
                        NoteInHistory(pCurrent, true);
                    }
                }
            }
            for (aIter2 = aRange2.first; aIter2 != aRange2.second; ++aIter2)
            {
                if (!aIter2->second.second) // already set above, don't change
                {
                    SwTextAttr *const pCurrent(aIter2->second.first);
                    if (pCurrent->IsFormatIgnoreStart() != bSetIgnoreFlag)
                    {
                        NoteInHistory(pCurrent);
                        pCurrent->SetFormatIgnoreStart(bSetIgnoreFlag);
                        NoteInHistory(pCurrent, true);
                    }
                }
            }
            i = j; // ++i not enough: i + 1 may have been deleted (MATCH)!
            ++j;
        }
    }

    return bRet;
}

// check if there is already a character format and adjust the sort numbers
static void lcl_CheckSortNumber( const SwpHints& rHints, SwTextCharFormat& rNewCharFormat )
{
    const sal_Int32 nHtStart = rNewCharFormat.GetStart();
    const sal_Int32 nHtEnd   = *rNewCharFormat.GetEnd();
    sal_uInt16 nSortNumber = 0;

    for ( size_t i = 0; i < rHints.Count(); ++i )
    {
        const SwTextAttr* pOtherHt = rHints.Get(i);

        const sal_Int32 nOtherStart = pOtherHt->GetStart();

        if ( nOtherStart > nHtStart )
            break;

        if ( RES_TXTATR_CHARFMT == pOtherHt->Which() )
        {
            const sal_Int32 nOtherEnd = *pOtherHt->End();

            if ( nOtherStart == nHtStart && nOtherEnd == nHtEnd )
            {
                nSortNumber = static_txtattr_cast<const SwTextCharFormat*>(pOtherHt)->GetSortNumber() + 1;
            }
        }
    }

    if ( nSortNumber > 0 )
        rNewCharFormat.SetSortNumber( nSortNumber );
}

/*
 * Try to insert the new hint.
 * Depending on the type of the hint, this either always succeeds, or may fail.
 * Depending on the type of the hint, other hints may be deleted or
 * overwritten.
 * The return value indicates successful insertion.
 */
bool SwpHints::TryInsertHint(
    SwTextAttr* const pHint,
    SwTextNode &rNode,
    const SetAttrMode nMode )
{
    if ( MAX_HINTS <= Count() ) // we're sorry, this flight is overbooked...
    {
        OSL_FAIL("hints array full :-(");
        return false;
    }

    sal_Int32 *pHtEnd = pHint->GetEnd();
    const sal_uInt16 nWhich = pHint->Which();
    std::vector<sal_uInt16> aWhichSublist;

    switch( nWhich )
    {
    case RES_TXTATR_CHARFMT:
    {
        // Check if character format contains hidden attribute:
        const SwCharFormat* pFormat = pHint->GetCharFormat().GetCharFormat();
        const SfxPoolItem* pItem;
        if ( SfxItemState::SET == pFormat->GetItemState( RES_CHRATR_HIDDEN, true, &pItem ) )
            rNode.SetCalcHiddenCharFlags();

        static_txtattr_cast<SwTextCharFormat*>(pHint)->ChgTextNode( &rNode );
        break;
    }
    // #i75430# Recalc hidden flags if necessary
    case RES_TXTATR_AUTOFMT:
    {
        std::shared_ptr<SfxItemSet> const pSet( pHint->GetAutoFormat().GetStyleHandle() );
        if (pHint->GetStart() == *pHint->GetEnd())
        {
            if (pSet->Count() == 1 && pSet->GetItem(RES_CHRATR_RSID, false))
            {   // empty range RSID-only hints could cause trouble, there's no
                rNode.DestroyAttr(pHint); // need for them so don't insert
                return false;
            }
        }
        // Check if auto style contains hidden attribute:
        const SfxPoolItem* pHiddenItem = CharFormat::GetItem( *pHint, RES_CHRATR_HIDDEN );
        if ( pHiddenItem )
            rNode.SetCalcHiddenCharFlags();

        // fdo#71556: populate aWhichFormatAttr member of SwMsgPoolItem
        const sal_uInt16 *pRanges = pSet->GetRanges();
        while( (*pRanges) != 0 )
        {
            const sal_uInt16 nBeg = (*pRanges);
            ++pRanges;
            const sal_uInt16 nEnd = (*pRanges);
            ++pRanges;
            for( sal_uInt16 nSubElem = nBeg; nSubElem <= nEnd; ++nSubElem )
                if( pSet->HasItem( nSubElem ) )
                    aWhichSublist.push_back( nSubElem );
        }
        break;
    }
    case RES_TXTATR_INETFMT:
        static_txtattr_cast<SwTextINetFormat*>(pHint)->InitINetFormat(rNode);
        break;

    case RES_TXTATR_FIELD:
    case RES_TXTATR_ANNOTATION:
    case RES_TXTATR_INPUTFIELD:
        {
            SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pHint));
            bool bDelFirst = nullptr != pTextField->GetpTextNode();
            pTextField->ChgTextNode( &rNode );
            SwDoc* pDoc = rNode.GetDoc();
            const SwField* pField = pTextField->GetFormatField().GetField();

            if( !pDoc->getIDocumentFieldsAccess().IsNewFieldLst() )
            {
                // certain fields must update the SwDoc's calculation flags
                switch( pField->GetTyp()->Which() )
                {
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENPARAFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    {
                        if( bDelFirst )
                            pDoc->getIDocumentFieldsAccess().InsDelFieldInFieldLst(false, *pTextField);
                        if( rNode.GetNodes().IsDocNodes() )
                            pDoc->getIDocumentFieldsAccess().InsDelFieldInFieldLst(true, *pTextField);
                    }
                    break;
                case RES_DDEFLD:
                    if( rNode.GetNodes().IsDocNodes() )
                        static_cast<SwDDEFieldType*>(pField->GetTyp())->IncRefCnt();
                    break;
                }
            }

            // insert into real document's nodes-array?
            if( rNode.GetNodes().IsDocNodes() )
            {
                bool bInsFieldType = false;
                switch( pField->GetTyp()->Which() )
                {
                case RES_SETEXPFLD:
                    bInsFieldType = static_cast<SwSetExpFieldType*>(pField->GetTyp())->IsDeleted();
                    if( nsSwGetSetExpType::GSE_SEQ & static_cast<SwSetExpFieldType*>(pField->GetTyp())->GetType() )
                    {
                        // register the field at its FieldType before setting
                        // the sequence reference number!
                        SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(
                                    pDoc->getIDocumentFieldsAccess().InsertFieldType( *pField->GetTyp() ) );
                        if( pFieldType != pField->GetTyp() )
                        {
                            SwFormatField* pFormatField = const_cast<SwFormatField*>(&pTextField->GetFormatField());
                            pFormatField->RegisterToFieldType( *pFieldType );
                            pFormatField->GetField()->ChgTyp( pFieldType );
                        }
                        pFieldType->SetSeqRefNo( *const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField)) );
                    }
                    break;
                case RES_USERFLD:
                    bInsFieldType = static_cast<SwUserFieldType*>(pField->GetTyp())->IsDeleted();
                    break;

                case RES_DDEFLD:
                    if( pDoc->getIDocumentFieldsAccess().IsNewFieldLst() )
                        static_cast<SwDDEFieldType*>(pField->GetTyp())->IncRefCnt();
                    bInsFieldType = static_cast<SwDDEFieldType*>(pField->GetTyp())->IsDeleted();
                    break;

                case RES_POSTITFLD:
                    if ( pDoc->GetDocShell() )
                    {
                        pDoc->GetDocShell()->Broadcast( SwFormatFieldHint(
                            &pTextField->GetFormatField(), SwFormatFieldHintWhich::INSERTED));
                    }
                    break;
                }
                if( bInsFieldType )
                    pDoc->getIDocumentFieldsAccess().InsDeletedFieldType( *pField->GetTyp() );
            }
        }
        break;
    case RES_TXTATR_FTN :
        static_cast<SwTextFootnote*>(pHint)->ChgTextNode( &rNode );
        break;
    case RES_TXTATR_REFMARK:
        static_txtattr_cast<SwTextRefMark*>(pHint)->ChgTextNode( &rNode );
        if( rNode.GetNodes().IsDocNodes() )
        {
            // search for a reference with the same name
            SwTextAttr* pTmpHt;
            for( size_t n = 0, nEnd = Count(); n < nEnd; ++n )
            {
                sal_Int32 *pTmpHtEnd;
                sal_Int32 *pTmpHintEnd;
                if (RES_TXTATR_REFMARK == (pTmpHt = Get(n))->Which() &&
                    pHint->GetAttr() == pTmpHt->GetAttr() &&
                    nullptr != ( pTmpHtEnd = pTmpHt->GetEnd() ) &&
                    nullptr != ( pTmpHintEnd = pHint->GetEnd() ) )
                {
                    SwComparePosition eCmp = ::ComparePosition(
                            pTmpHt->GetStart(), *pTmpHtEnd,
                            pHint->GetStart(), *pTmpHintEnd );
                    bool bDelOld = true, bChgStart = false, bChgEnd = false;
                    switch( eCmp )
                    {
                    case POS_BEFORE:
                    case POS_BEHIND:    bDelOld = false; break;

                    case POS_OUTSIDE:   bChgStart = bChgEnd = true; break;

                    case POS_COLLIDE_END:
                    case POS_OVERLAP_BEFORE:    bChgStart = true; break;
                    case POS_COLLIDE_START:
                    case POS_OVERLAP_BEHIND:    bChgEnd = true; break;
                    default: break;
                    }

                    if( bChgStart )
                        pHint->GetStart() = pTmpHt->GetStart();
                    if( bChgEnd )
                        *pTmpHintEnd = *pTmpHtEnd;

                    if( bDelOld )
                    {
                        NoteInHistory( pTmpHt );
                        rNode.DestroyAttr( Cut( n-- ) );
                        --nEnd;
                    }
                }
            }
        }
        break;
    case RES_TXTATR_TOXMARK:
        static_txtattr_cast<SwTextTOXMark*>(pHint)->ChgTextNode( &rNode );
        break;

    case RES_TXTATR_CJK_RUBY:
        static_txtattr_cast<SwTextRuby*>(pHint)->InitRuby(rNode);
        break;

    case RES_TXTATR_META:
    case RES_TXTATR_METAFIELD:
        static_txtattr_cast<SwTextMeta *>(pHint)->ChgTextNode( &rNode );
        break;

    case RES_CHRATR_HIDDEN:
        rNode.SetCalcHiddenCharFlags();
        break;
    }

    if( SetAttrMode::DONTEXPAND & nMode )
        pHint->SetDontExpand( true );

    // special handling for SwTextAttrs without end:
    // 1) they cannot overlap
    // 2) if two fields are adjacent, they must not be merged into one
    // this is guaranteed by inserting a CH_TXTATR_* into the paragraph text!
    sal_Int32 nHtStart = pHint->GetStart();
    if( !pHtEnd )
    {
        Insert( pHint );
        NoteInHistory(pHint, true);
        CalcFlags();
#ifdef DBG_UTIL
        if( !rNode.GetDoc()->IsInReading() )
            CHECK;
#endif
        // ... and notify listeners
        if(rNode.HasWriterListeners())
        {
            SwUpdateAttr aHint(
                nHtStart,
                nHtStart,
                nWhich);

            rNode.ModifyNotification(nullptr,&aHint);
        }

        return true;
    }

    // from here on, pHint is known to have an end index!

    if( *pHtEnd < nHtStart )
    {
        assert(*pHtEnd >= nHtStart);

        // just swap the nonsense:
        pHint->GetStart() = *pHtEnd;
        *pHtEnd = nHtStart;
        nHtStart = pHint->GetStart();
    }

    // I need this value later on for notification but the pointer may become invalid
    const sal_Int32 nHintEnd = *pHtEnd;
    const bool bNoHintAdjustMode = bool(SetAttrMode::NOHINTADJUST & nMode);

    // handle nesting attributes: inserting may fail due to overlap!
    if (pHint->IsNesting())
    {
        const bool bRet(
            TryInsertNesting(rNode, *static_txtattr_cast<SwTextAttrNesting*>(pHint)));
        if (!bRet) return false;
    }
    // Currently REFMARK and TOXMARK have OverlapAllowed set to true.
    // These attributes may be inserted directly.
    // Also attributes without length may be inserted directly.
    // SETATTR_NOHINTADJUST is set e.g., during undo.
    // Portion building in not necessary during XML import.
    else if ( !bNoHintAdjustMode &&
         !pHint->IsOverlapAllowedAttr() &&
         !rNode.GetDoc()->IsInXMLImport() &&
         ( RES_TXTATR_AUTOFMT == nWhich ||
           RES_TXTATR_CHARFMT == nWhich ) )
    {
        assert( nWhich != RES_TXTATR_AUTOFMT ||
                static_cast<const SwFormatAutoFormat&>(pHint->GetAttr()).GetStyleHandle()->GetPool() ==
                &rNode.GetDoc()->GetAttrPool());

        BuildPortions( rNode, *pHint, nMode );

        if ( nHtStart < nHintEnd ) // skip merging for 0-length attributes
            MergePortions( rNode );
    }
    else
    {
        // There may be more than one character style at the current position.
        // Take care of the sort number.
        // Special case ruby portion: During import, the ruby attribute is set
        // multiple times
        // Special case hyperlink: During import, the ruby attribute is set
        // multiple times
        // FME 2007-11-08 #i82989# in NOHINTADJUST mode, we want to insert
        // character attributes directly
        if ( ( RES_TXTATR_CHARFMT  == nWhich && !bNoHintAdjustMode ) )
        {
            BuildPortions( rNode, *pHint, nMode );
        }
        else
        {
            // #i82989# Check sort numbers in NoHintAdjustMode
            if ( RES_TXTATR_CHARFMT == nWhich )
                lcl_CheckSortNumber(*this, *static_txtattr_cast<SwTextCharFormat*>(pHint));

            Insert( pHint );
            NoteInHistory( pHint, true );
        }
    }

    // ... and notify listeners
    if ( rNode.HasWriterListeners() )
    {
        SwUpdateAttr aHint(
            nHtStart,
            nHtStart == nHintEnd ? nHintEnd + 1 : nHintEnd,
            nWhich);

        rNode.ModifyNotification( nullptr, &aHint );
    }

#ifdef DBG_UTIL
    if( !bNoHintAdjustMode && !rNode.GetDoc()->IsInReading() )
        CHECK;
#endif

    return true;
}

void SwpHints::DeleteAtPos( const size_t nPos )
{
    SwTextAttr *pHint = Get(nPos);
    // ChainDelete( pHint );
    NoteInHistory( pHint );

    // optimization: nPos is the position in the Starts array
    SwTextAttr *pHt = m_HintsByStart[ nPos ];
    m_HintsByStart.erase( m_HintsByStart.begin() + nPos );

    Resort();

    bool const done = m_HintsByEnd.erase(pHt);
    assert(done);
    (void)done; // unused in NDEBUG

    if( pHint->Which() == RES_TXTATR_FIELD )
    {
        SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pHint));
        const SwFieldType* pFieldTyp = pTextField->GetFormatField().GetField()->GetTyp();
        if( RES_DDEFLD == pFieldTyp->Which() )
        {
            const SwTextNode* pNd = pTextField->GetpTextNode();
            if( pNd && pNd->GetNodes().IsDocNodes() )
                const_cast<SwDDEFieldType*>(static_cast<const SwDDEFieldType*>(pFieldTyp))->DecRefCnt();
            pTextField->ChgTextNode(nullptr);
        }
        else if ( m_bHasHiddenParaField &&
                 RES_HIDDENPARAFLD == pFieldTyp->Which() )
        {
            m_bCalcHiddenParaField = true;
        }
    }
    else if ( pHint->Which() == RES_TXTATR_ANNOTATION )
    {
        SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pHint));
        const_cast<SwFormatField&>(pTextField->GetFormatField()).Broadcast(
            SwFormatFieldHint(&pTextField->GetFormatField(), SwFormatFieldHintWhich::REMOVED));
    }

    CalcFlags();
    CHECK_NOTMERGED; // called from BuildPortions
}

/// delete the hint
/// precondition: pTextHt must be in this array
void SwpHints::Delete( SwTextAttr* pTextHt )
{
    const size_t nPos = GetIndexOf( pTextHt );
    assert(SAL_MAX_SIZE != nPos);
    if( SAL_MAX_SIZE != nPos )
        DeleteAtPos( nPos );
}

void SwTextNode::ClearSwpHintsArr( bool bDelFields )
{
    if ( HasHints() )
    {
        size_t nPos = 0;
        while ( nPos < m_pSwpHints->Count() )
        {
            SwTextAttr* pDel = m_pSwpHints->Get( nPos );
            bool bDel = false;

            switch( pDel->Which() )
            {
            case RES_TXTATR_FLYCNT:
            case RES_TXTATR_FTN:
                break;

            case RES_TXTATR_FIELD:
            case RES_TXTATR_ANNOTATION:
            case RES_TXTATR_INPUTFIELD:
                if( bDelFields )
                    bDel = true;
                break;
            default:
                bDel = true; break;
            }

            if( bDel )
            {
                m_pSwpHints->DeleteAtPos( nPos );
                DestroyAttr( pDel );
            }
            else
                ++nPos;
        }
    }
}

sal_uInt16 SwTextNode::GetLang( const sal_Int32 nBegin, const sal_Int32 nLen,
                           sal_uInt16 nScript ) const
{
    sal_uInt16 nRet = LANGUAGE_DONTKNOW;

    if ( ! nScript )
    {
        nScript = g_pBreakIt->GetRealScriptOfText( m_Text, nBegin );
    }

    // #i91465# Consider nScript if pSwpHints == 0
    const sal_uInt16 nWhichId = GetWhichOfScript( RES_CHRATR_LANGUAGE, nScript );

    if ( HasHints() )
    {
        const sal_Int32 nEnd = nBegin + nLen;
        const size_t nSize = m_pSwpHints->Count();
        for ( size_t i = 0; i < nSize; ++i )
        {
            const SwTextAttr *pHt = m_pSwpHints->Get(i);
            const sal_Int32 nAttrStart = pHt->GetStart();
            if( nEnd < nAttrStart )
                break;

            const sal_uInt16 nWhich = pHt->Which();

            if( nWhichId == nWhich ||
                    ( ( pHt->IsCharFormatAttr() || RES_TXTATR_AUTOFMT == nWhich ) && CharFormat::IsItemIncluded( nWhichId, pHt ) ) )
            {
                const sal_Int32 *pEndIdx = pHt->End();
                // do the attribute and the range overlap?
                if( !pEndIdx )
                    continue;
                if( nLen )
                {
                    if( nAttrStart >= nEnd || nBegin >= *pEndIdx )
                        continue;
                }
                else if( nBegin != nAttrStart || ( nAttrStart != *pEndIdx && nBegin ))
                {
                    if( nAttrStart >= nBegin )
                        continue;
                    if( pHt->DontExpand() ? nBegin >= *pEndIdx : nBegin > *pEndIdx)
                        continue;
                }
                const SfxPoolItem* pItem = CharFormat::GetItem( *pHt, nWhichId );
                const sal_uInt16 nLng = static_cast<const SvxLanguageItem*>(pItem)->GetLanguage();

                // does the attribute completely cover the range?
                if( nAttrStart <= nBegin && nEnd <= *pEndIdx )
                    nRet = nLng;
                else if( LANGUAGE_DONTKNOW == nRet )
                    nRet = nLng; // partial overlap, the first one wins
            }
        }
    }
    if( LANGUAGE_DONTKNOW == nRet )
    {
        nRet = static_cast<const SvxLanguageItem&>(GetSwAttrSet().Get( nWhichId )).GetLanguage();
        if( LANGUAGE_DONTKNOW == nRet )
            nRet = static_cast<sal_uInt16>(GetAppLanguage());
    }
    return nRet;
}

sal_Unicode GetCharOfTextAttr( const SwTextAttr& rAttr )
{
    sal_Unicode cRet = CH_TXTATR_BREAKWORD;
    switch ( rAttr.Which() )
    {
        case RES_TXTATR_REFMARK:
        case RES_TXTATR_TOXMARK:
        case RES_TXTATR_ANNOTATION:
            cRet = CH_TXTATR_INWORD;
        break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_FLYCNT:
        case RES_TXTATR_FTN:
        case RES_TXTATR_META:
        case RES_TXTATR_METAFIELD:
        {
            cRet = CH_TXTATR_BREAKWORD;
        }
        break;

        default:
            assert(!"GetCharOfTextAttr: unknown attr");
            break;
    }
    return cRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
