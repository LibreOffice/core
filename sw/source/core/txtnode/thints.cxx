/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thints.cxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:09:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SOT_FACTORY_HXX
#include <sot/factory.hxx>
#endif
#ifndef _SVX_XMLCNITM_HXX
#include <svx/xmlcnitm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#include <svtools/stylepool.hxx>
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
// --> OD 2008-01-16 #newlistlevelattrs#
#include <svx/lrspitem.hxx>
// <--

#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#include <fmtautofmt.hxx>
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>           // fuer SwRegHistory
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#include <istyleaccess.hxx>
// OD 26.06.2003 #108784#
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#include <docsh.hxx>
#include <svtools/smplhint.hxx>
#include <algorithm>
#include <map>

#ifndef PRODUCT
#define CHECK    Check();
#else
#define CHECK
#endif

using namespace ::com::sun::star::i18n;

// This function takes care for the following text attribute:
// RES_TXTATR_CHARFMT, RES_TXTATR_INETFMT, RES_TXTATR_AUTOFMT, and
// RES_TXTATR_CJK_RUBY. These attributes have to be handled in a
// special way (Portion building).
// 1. New attribute is RES_TXTATR_AUTOFMT or RES_TXTATR_CHARFMT:
// The new attribute will be split by any existing RES_TXTATR_AUTOFMT,
// RES_TXTATR_CHARFMT or RES_TXTATR_INETFMT. The new attribute itself will
// split any existing RES_TXTATR_AUTOFMT or RES_TXTATR_CHARFMT.
// 2. New attribute is RES_TXTATR_INETFMT:
// The new attribute will split any existing RES_TXTATR_AUTOFMT or RES_TXTATR_CHARFMT.
// The new attribute will suppress any existing RES_TXTATR_INETFMT.
// 3. New attribute is RES_TXTATR_CJK_RUBY:
// The new attribute will suppress any existing RES_TXTATR_CJK_RUBY.

void SwpHints::BuildPortions( SwTxtNode& rNode, SwTxtAttr& rNewHint, USHORT nMode )
{
    ASSERT( RES_TXTATR_CHARFMT == rNewHint.Which() ||
            RES_TXTATR_INETFMT == rNewHint.Which() ||
            RES_TXTATR_AUTOFMT == rNewHint.Which() ||
            RES_TXTATR_CJK_RUBY == rNewHint.Which(),
            "Expecting CHARFMT or AUTOFMT or INETFMT or RUBY" )

    const USHORT nWhich = rNewHint.Which();

    const xub_StrLen nThisStart = *rNewHint.GetStart();
    const xub_StrLen nThisEnd =   *rNewHint.GetEnd();
    const bool bNoLengthAttribute = nThisStart == nThisEnd;

    std::vector<SwTxtAttr*> aInsDelHints;
    std::vector<SwTxtAttr*>::iterator aIter;

    //
    // 1. Some attributes are not allowed to overlap.
    // They should displace attributes of the same kind.
    // This is a special case for RES_TXTATR_INETFMT and
    // RES_TXTATR_CJK_RUBY.
    //
    if ( RES_TXTATR_INETFMT == nWhich || RES_TXTATR_CJK_RUBY == nWhich )
    {
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwTxtAttr* pOther = GetHt(i);

            if ( nWhich == pOther->Which() )
            {
                xub_StrLen nOtherStart = *pOther->GetStart();
                const xub_StrLen nOtherEnd = *pOther->GetEnd();

                // Check if start of new attribute overlaps with pOther.
                // Split pOther if necessary:
                if ( nOtherStart < nThisStart && nThisStart < nOtherEnd )
                {
                    SwTxtAttr* pNewAttr = rNode.MakeTxtAttr( pOther->GetAttr(), nOtherStart, nThisStart );
                    aInsDelHints.push_back( pNewAttr );

                    if( pHistory ) pHistory->Add( pOther );
                    *pOther->GetStart() = nThisStart;
                    if( pHistory ) pHistory->Add( pOther, TRUE );
                    nOtherStart = nThisStart;
                }

                // Check if end of new attribute overlaps with pOther:
                // Split pOther if necessary:
                if ( nOtherStart < nThisEnd && nThisEnd < nOtherEnd )
                {
                    SwTxtAttr* pNewAttr = rNode.MakeTxtAttr( pOther->GetAttr(), nOtherStart, nThisEnd );
                    aInsDelHints.push_back( pNewAttr );

                    if( pHistory ) pHistory->Add( pOther );
                    *pOther->GetStart() = nThisEnd;
                    if( pHistory ) pHistory->Add( pOther, TRUE );
                }
            }
        }

        // Insert the newly created attributes:
        const sal_uInt16 nCharFmtID = static_cast<sal_uInt16>( RES_TXTATR_INETFMT == nWhich ?
                                                               RES_POOLCHR_INET_NORMAL :
                                                               RES_POOLCHR_RUBYTEXT );
        SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool( nCharFmtID );

        for ( aIter = aInsDelHints.begin(); aIter != aInsDelHints.end(); ++aIter )
        {
            if ( RES_TXTATR_INETFMT == nWhich )
            {
                SwTxtINetFmt* pInetAttr = static_cast<SwTxtINetFmt*>(*aIter);
                pInetAttr->ChgTxtNode( &rNode );
                pFmt->Add( pInetAttr );

            }

            else
            {
                SwTxtRuby* pRubyAttr = static_cast<SwTxtRuby*>(*aIter);
                pRubyAttr->ChgTxtNode( &rNode );
                pFmt->Add( pRubyAttr );

            }

            SwpHintsArr::Insert( *aIter );
            if ( pHistory ) pHistory->Add( *aIter, TRUE );
        }

        aInsDelHints.clear();

        // Now delete all attributes of the same type as the new one
        // which are fully covered by the new attribute:
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwTxtAttr* pOther = GetHt(i);
            if ( nWhich == pOther->Which() )
            {
                const xub_StrLen nOtherStart = *pOther->GetStart();
                const xub_StrLen nOtherEnd = *pOther->GetEnd();

                if ( nOtherStart >= nThisStart && nOtherEnd <= nThisEnd )
                    aInsDelHints.push_back( pOther );
            }
        }
        for ( aIter = aInsDelHints.begin(); aIter != aInsDelHints.end(); ++aIter )
        {
            Delete( *aIter );
            rNode.DestroyAttr( *aIter );
        }

        SwpHintsArr::Insert( &rNewHint );
        return;
    }

    ASSERT( RES_TXTATR_CHARFMT == rNewHint.Which() ||
            RES_TXTATR_AUTOFMT == rNewHint.Which(),
            "Expecting CHARFMT or AUTOFMT" )

    //
    // 2. Find the hints which cover the start and end position
    // of the new hint. These hints have to be split into two portions:
    //
    if ( !bNoLengthAttribute ) // nothing to do for no length attributes
    {
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwTxtAttr* pOther = GetHt(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            xub_StrLen nOtherStart = *pOther->GetStart();
            const xub_StrLen nOtherEnd = *pOther->GetEnd();

            // Check if start of new attribute overlaps with pOther:
            // Split pOther if necessary:
            if ( nOtherStart < nThisStart && nThisStart < nOtherEnd )
            {
                SwTxtAttr* pNewAttr = rNode.MakeTxtAttr( pOther->GetAttr(), nOtherStart, nThisStart );
                if ( RES_TXTATR_CHARFMT == pOther->Which() )
                    static_cast<SwTxtCharFmt*>(pNewAttr)->SetSortNumber( static_cast<SwTxtCharFmt*>(pOther)->GetSortNumber() );
                aInsDelHints.push_back( pNewAttr );

                if( pHistory ) pHistory->Add( pOther );
                *pOther->GetStart() = nThisStart;
                if( pHistory ) pHistory->Add( pOther, TRUE );

                nOtherStart = nThisStart;
            }

            // Check if end of new attribute overlaps with pOther:
            // Split pOther if necessary:
            if ( nOtherStart < nThisEnd && nThisEnd < nOtherEnd )
            {
                SwTxtAttr* pNewAttr = rNode.MakeTxtAttr( pOther->GetAttr(), nOtherStart, nThisEnd );
                if ( RES_TXTATR_CHARFMT == pOther->Which() )
                    static_cast<SwTxtCharFmt*>(pNewAttr)->SetSortNumber( static_cast<SwTxtCharFmt*>(pOther)->GetSortNumber() );
                aInsDelHints.push_back( pNewAttr );

                if( pHistory ) pHistory->Add( pOther );
                *pOther->GetStart() = nThisEnd;
                if( pHistory ) pHistory->Add( pOther, TRUE );
            }
        }

        // Insert the newly created attributes:
        for ( aIter = aInsDelHints.begin(); aIter != aInsDelHints.end(); ++aIter )
        {
            SwpHintsArr::Insert( *aIter );
            if ( pHistory ) pHistory->Add( *aIter, TRUE );
        }
    }

#ifndef PRODUCT
    if( !rNode.GetDoc()->IsInReading() )
        CHECK;
#endif

    //
    // 4. Split rNewHint into 1 ... n new hints:
    //
    std::set<xub_StrLen> aBounds;
    aBounds.insert( nThisStart );
    aBounds.insert( nThisEnd );

    if ( !bNoLengthAttribute ) // nothing to do for no length attributes
    {
        for ( USHORT i = 0; i < Count(); ++i )
        {
            const SwTxtAttr* pOther = GetHt(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            const xub_StrLen nOtherStart = *pOther->GetStart();
            const xub_StrLen nOtherEnd = *pOther->GetEnd();

            aBounds.insert( nOtherStart );
            aBounds.insert( nOtherEnd );
        }
    }

    std::set<xub_StrLen>::iterator aStartIter = aBounds.lower_bound( nThisStart );
    std::set<xub_StrLen>::iterator aEndIter = aBounds.upper_bound( nThisEnd );
    xub_StrLen nPorStart = *aStartIter;
    ++aStartIter;
    bool bDestroyHint = true;

    //
    // Insert the 1...n new parts of the new attribute:
    //
    while ( aStartIter != aEndIter || bNoLengthAttribute )
    {
        ASSERT( bNoLengthAttribute || nPorStart < *aStartIter, "AUTOSTYLES: BuildPortion trouble" )

        const xub_StrLen nPorEnd = bNoLengthAttribute ? nPorStart : *aStartIter;
        aInsDelHints.clear();

        // Get all hints that are in [nPorStart, nPorEnd[:
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwTxtAttr *pOther = GetHt(i);

            if ( RES_TXTATR_CHARFMT != pOther->Which() &&
                 RES_TXTATR_AUTOFMT != pOther->Which() )
                continue;

            const xub_StrLen nOtherStart = *pOther->GetStart();

            if ( nOtherStart > nPorStart )
                break;

            if ( pOther->GetEnd() && *pOther->GetEnd() == nPorEnd && nOtherStart == nPorStart )
            {
                ASSERT( *pOther->GetEnd() == nPorEnd, "AUTOSTYLES: BuildPortion trouble" )
                aInsDelHints.push_back( pOther );
            }
        }

        SwTxtAttr* pNewAttr = 0;
        if ( RES_TXTATR_CHARFMT == nWhich )
        {
            // pNewHint can be inserted after calculating the sort value.
            // This should ensure, that pNewHint comes behind the already present
            // character style
            USHORT nCharStyleCount = 0;
            aIter = aInsDelHints.begin();
            while ( aIter != aInsDelHints.end() )
            {
                if ( RES_TXTATR_CHARFMT == (*aIter)->Which() )
                {
                    // --> FME 2007-02-16 #i74589#
                    const SwFmtCharFmt& rOtherCharFmt = (*aIter)->GetCharFmt();
                    const SwFmtCharFmt& rThisCharFmt = rNewHint.GetCharFmt();
                    const bool bSameCharFmt = rOtherCharFmt.GetCharFmt() == rThisCharFmt.GetCharFmt();
                    // <--

                    if ( !( nsSetAttrMode::SETATTR_DONTREPLACE & nMode ) || bNoLengthAttribute || bSameCharFmt )
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
                    // remove all attributes from auto styles, which are explicitely set in
                    // the new character format:
                    ASSERT( RES_TXTATR_AUTOFMT == (*aIter)->Which(), "AUTOSTYLES - Misc trouble" )
                    SwTxtAttr* pOther = *aIter;
                    boost::shared_ptr<SfxItemSet> pOldStyle = static_cast<const SwFmtAutoFmt&>(pOther->GetAttr()).GetStyleHandle();

                    // For each attribute in the automatic style check if it
                    // is also set the the new character style:
                    SfxItemSet aNewSet( *pOldStyle->GetPool(), RES_CHRATR_BEGIN, RES_CHRATR_END );
                    SfxItemIter aItemIter( *pOldStyle );
                    const SfxPoolItem* pItem = aItemIter.GetCurItem();
                    while( TRUE )
                    {
                        if ( !CharFmt::IsItemIncluded( pItem->Which(), &rNewHint ) )
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
                        pNewAttr = rNode.MakeTxtAttr( aNewSet, nPorStart, nPorEnd );
                        SwpHintsArr::Insert( pNewAttr );
                           if ( pHistory ) pHistory->Add( pNewAttr, TRUE );
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
                pNewAttr = rNode.MakeTxtAttr( rNewHint.GetAttr(), nPorStart, nPorEnd );
                static_cast<SwTxtCharFmt*>(pNewAttr)->SetSortNumber( nCharStyleCount );
            }
        }
        else
        {
            // Find the current autostyle. Mix attributes if necessary.
            SwTxtAttr* pCurrentAutoStyle = 0;
            SwTxtAttr* pCurrentCharFmt = 0;
            aIter = aInsDelHints.begin();
            while ( aIter != aInsDelHints.end() )
            {
                if ( RES_TXTATR_AUTOFMT == (*aIter)->Which() )
                    pCurrentAutoStyle = *aIter;
                else if ( RES_TXTATR_CHARFMT == (*aIter)->Which() )
                    pCurrentCharFmt = *aIter;
                ++aIter;
            }

            boost::shared_ptr<SfxItemSet> pNewStyle = static_cast<const SwFmtAutoFmt&>(rNewHint.GetAttr()).GetStyleHandle();
            if ( pCurrentAutoStyle )
            {
                boost::shared_ptr<SfxItemSet> pCurrentStyle = static_cast<const SwFmtAutoFmt&>(pCurrentAutoStyle->GetAttr()).GetStyleHandle();

                // Merge attributes
                SfxItemSet aNewSet( *pCurrentStyle );
                aNewSet.Put( *pNewStyle );

                // --> FME 2007-4-11 #i75750# Remove attributes already set at whole paragraph
                // --> FME 2007-09-24 #i81764# This should not be applied for no length attributes!!! <--
                if ( !bNoLengthAttribute && rNode.HasSwAttrSet() && aNewSet.Count() )
                {
                    SfxItemIter aIter2( aNewSet );
                    const SfxPoolItem* pItem = aIter2.GetCurItem();
                    const SfxItemSet& rWholeParaAttrSet = rNode.GetSwAttrSet();

                    do
                    {
                        const SfxPoolItem* pTmpItem = 0;
                        if ( SFX_ITEM_SET == rWholeParaAttrSet.GetItemState( pItem->Which(), FALSE, &pTmpItem ) &&
                             pTmpItem == pItem )
                        {
                            // Do not clear item if the attribute is set in a character format:
                            if ( !pCurrentCharFmt || 0 == CharFmt::GetItem( *pCurrentCharFmt, pItem->Which() ) )
                                aNewSet.ClearItem( pItem->Which() );
                        }
                    }
                    while (!aIter2.IsAtEnd() && 0 != (pItem = aIter2.NextItem()));
                }
                // <--

                // Remove old hint
                Delete( pCurrentAutoStyle );
                rNode.DestroyAttr( pCurrentAutoStyle );

                // Create new AutoStyle
                if ( aNewSet.Count() )
                    pNewAttr = rNode.MakeTxtAttr( aNewSet, nPorStart, nPorEnd );
            }
            else
            {
                // Remove any attributes which are already set at the whole paragraph:
                bool bOptimizeAllowed = true;

                SfxItemSet* pNewSet = 0;
                // --> FME 2007-4-11 #i75750# Remove attributes already set at whole paragraph
                // --> FME 2007-09-24 #i81764# This should not be applied for no length attributes!!! <--
                if ( !bNoLengthAttribute && rNode.HasSwAttrSet() && pNewStyle->Count() )
                {
                    SfxItemIter aIter2( *pNewStyle );
                    const SfxPoolItem* pItem = aIter2.GetCurItem();
                    const SfxItemSet& rWholeParaAttrSet = rNode.GetSwAttrSet();

                    do
                    {
                        const SfxPoolItem* pTmpItem = 0;
                        if ( SFX_ITEM_SET == rWholeParaAttrSet.GetItemState( pItem->Which(), FALSE, &pTmpItem ) &&
                             pTmpItem == pItem )
                        {
                            // Do not clear item if the attribute is set in a character format:
                            if ( !pCurrentCharFmt || 0 == CharFmt::GetItem( *pCurrentCharFmt, pItem->Which() ) )
                            {
                                if ( !pNewSet )
                                    pNewSet = pNewStyle->Clone( TRUE );
                                pNewSet->ClearItem( pItem->Which() );
                            }
                        }
                    }
                    while (!aIter2.IsAtEnd() && 0 != (pItem = aIter2.NextItem()));

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
                // <--

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
                    pNewAttr = rNode.MakeTxtAttr( *pNewStyle, nPorStart, nPorEnd );
                }
            }
        }

        if ( pNewAttr )
        {
            SwpHintsArr::Insert( pNewAttr );
            if ( pHistory /* && bDestroyHint*/ )
                pHistory->Add( pNewAttr, TRUE );
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

/*************************************************************************
 *                      SwTxtNode::MakeTxtAttr()
 *************************************************************************/

    // lege ein neues TextAttribut an und fuege es SwpHints-Array ein
SwTxtAttr* SwTxtNode::MakeTxtAttr( const SfxPoolItem& rAttr,
                                   xub_StrLen nStt, xub_StrLen nEnd, bool bRedlineAttr )
{
    SwDoc* pDoc = GetDoc();

    if ( !bRedlineAttr && RES_CHRATR_BEGIN <= rAttr.Which () && rAttr.Which() < RES_CHRATR_END )
    {
        // Somebody wants to build a SwTxtAttr for a character attribute (and
        // this attribute is not meant for redlining). Sorry, this is not allowed
        // any longer. You'll get a brand new autostyle attribute:
        SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END );
        aItemSet.Put( rAttr );
        return MakeTxtAttr( aItemSet, nStt, nEnd );
    }
    else if ( RES_TXTATR_AUTOFMT == rAttr.Which() &&
              static_cast<const SwFmtAutoFmt&>(rAttr).GetStyleHandle()->GetPool() != &pDoc->GetAttrPool() )
    {
        // If the attribut is an autostyle which referes to a pool that is different from
        // pDoc's pool, we have to correct this:
        const StylePool::SfxItemSet_Pointer_t pAutoStyle = static_cast<const SwFmtAutoFmt&>(rAttr).GetStyleHandle();
        const SfxItemSet* pNewSet = pAutoStyle->SfxItemSet::Clone( TRUE, &pDoc->GetAttrPool() );
        SwTxtAttr* pNew = MakeTxtAttr( *pNewSet, nStt, nEnd );
        delete pNewSet;
        return pNew;
    }

    // Put new attribute into pool unless we are asked to build a redline attribute
    const SfxPoolItem& rNew = !bRedlineAttr ? GetDoc()->GetAttrPool().Put( rAttr ) : rAttr;

    SwTxtAttr* pNew = 0;
    switch( rNew.Which() )
    {
    case RES_TXTATR_CHARFMT:
        {
            SwFmtCharFmt &rFmtCharFmt = (SwFmtCharFmt&) rNew;
            if( !rFmtCharFmt.GetCharFmt() )
                rFmtCharFmt.SetCharFmt( GetDoc()->GetDfltCharFmt() );

            pNew = new SwTxtCharFmt( rFmtCharFmt, nStt, nEnd );
        }
        break;
    case RES_TXTATR_INETFMT:
        pNew = new SwTxtINetFmt( (SwFmtINetFmt&)rNew, nStt, nEnd );
        break;
    case RES_TXTATR_FIELD:
        pNew = new SwTxtFld( (SwFmtFld&)rNew, nStt );
        break;
    case RES_TXTATR_FLYCNT:
        {
            // erst hier wird das Frame-Format kopiert (mit Inhalt) !!
            pNew = new SwTxtFlyCnt( (SwFmtFlyCnt&)rNew, nStt );
            // Kopie von einem Text-Attribut
            if( ((SwFmtFlyCnt&)rAttr).GetTxtFlyCnt() )
                // dann muss das Format Kopiert werden
                ((SwTxtFlyCnt*)pNew)->CopyFlyFmt( GetDoc() );
        }
        break;
    case RES_TXTATR_FTN:
        pNew = new SwTxtFtn( (SwFmtFtn&)rNew, nStt );
        // ggfs. SeqNo kopieren
        if( ((SwFmtFtn&)rAttr).GetTxtFtn() )
            ((SwTxtFtn*)pNew)->SetSeqNo( ((SwFmtFtn&)rAttr).GetTxtFtn()->GetSeqRefNo() );
        break;
    case RES_TXTATR_HARDBLANK:
        pNew = new SwTxtHardBlank( (SwFmtHardBlank&)rNew, nStt );
        break;
    case RES_CHRATR_TWO_LINES:
        pNew = new SwTxt2Lines( (SvxTwoLinesItem&)rNew, nStt, nEnd );
        break;
    case RES_TXTATR_REFMARK:
        pNew = nStt == nEnd
                ? new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt )
                : new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt, &nEnd );
        break;
    case RES_TXTATR_TOXMARK:
        pNew = new SwTxtTOXMark( (SwTOXMark&)rNew, nStt, &nEnd );
        break;
    case RES_UNKNOWNATR_CONTAINER:
    case RES_TXTATR_UNKNOWN_CONTAINER:
        pNew = new SwTxtXMLAttrContainer( (SvXMLAttrContainerItem&)rNew,
                                        nStt, nEnd );
        break;
    case RES_TXTATR_CJK_RUBY:
        pNew = new SwTxtRuby( (SwFmtRuby&)rNew, nStt, nEnd );
        break;
    default:
        pNew = new SwTxtAttrEnd( rNew, nStt, nEnd );
        break;
    }

    return pNew;
}

SwTxtAttr* SwTxtNode::MakeTxtAttr( const SfxItemSet& rSet, xub_StrLen nStt, xub_StrLen nEnd )
{
    IStyleAccess& rStyleAccess = getIDocumentStyleAccess();
    const StylePool::SfxItemSet_Pointer_t pAutoStyle = rStyleAccess.getAutomaticStyle( rSet, IStyleAccess::AUTO_STYLE_CHAR );
    SwFmtAutoFmt aNewAutoFmt;
    aNewAutoFmt.SetStyleHandle( pAutoStyle );
    SwTxtAttr* pNew = MakeTxtAttr( aNewAutoFmt, nStt, nEnd );
    return pNew;
}


// loesche das Text-Attribut (muss beim Pool abgemeldet werden!)
void SwTxtNode::DestroyAttr( SwTxtAttr* pAttr )
{
    if( pAttr )
    {
        // einige Sachen muessen vorm Loeschen der "Format-Attribute" erfolgen
        SwDoc* pDoc = GetDoc();
        USHORT nDelMsg = 0;
        switch( pAttr->Which() )
        {
        case RES_TXTATR_FLYCNT:
            {
                // siehe auch die Anmerkung "Loeschen von Formaten
                // zeichengebundener Frames" in fesh.cxx, SwFEShell::DelFmt()
                SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                if( pFmt )      // vom Undo auf 0 gesetzt ??
                    pDoc->DelLayoutFmt( (SwFlyFrmFmt*)pFmt );
            }
            break;

        case RES_CHRATR_HIDDEN:
            SetCalcHiddenCharFlags();
            break;

        case RES_TXTATR_FTN:
            ((SwTxtFtn*)pAttr)->SetStartNode( 0 );
            nDelMsg = RES_FOOTNOTE_DELETED;
            break;

        case RES_TXTATR_FIELD:
            if( !pDoc->IsInDtor() )
            {
                // Wenn wir ein HiddenParaField sind, dann muessen wir
                // ggf. fuer eine Neuberechnung des Visible-Flags sorgen.
                const SwField* pFld = pAttr->GetFld().GetFld();

                //JP 06-08-95: DDE-Felder bilden eine Ausnahme
                ASSERT( RES_DDEFLD == pFld->GetTyp()->Which() ||
                        this == ((SwTxtFld*)pAttr)->GetpTxtNode(),
                        "Wo steht denn dieses Feld?" )

                // bestimmte Felder mussen am Doc das Calculations-Flag updaten
                switch( pFld->GetTyp()->Which() )
                {
                case RES_HIDDENPARAFLD:
                    SetCalcHiddenParaField();
                    // kein break !
                case RES_DBSETNUMBERFLD:
                case RES_GETEXPFLD:
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    if( !pDoc->IsNewFldLst() && GetNodes().IsDocNodes() )
                        pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pAttr );
                    break;
                case RES_DDEFLD:
                    if( GetNodes().IsDocNodes() &&
                        ((SwTxtFld*)pAttr)->GetpTxtNode() )
                        ((SwDDEFieldType*)pFld->GetTyp())->DecRefCnt();
                    break;
                case RES_POSTITFLD:
                    {
                        const_cast<SwFmtFld&>(pAttr->GetFld()).Broadcast( SwFmtFldHint( &((SwTxtFld*)pAttr)->GetFld(), SWFMTFLD_REMOVED ) );
                        break;
                    }
                }
            }
            nDelMsg = RES_FIELD_DELETED;
            break;

        case RES_TXTATR_TOXMARK:
            nDelMsg = RES_TOXMARK_DELETED;
            break;

        case RES_TXTATR_REFMARK:
            nDelMsg = RES_REFMARK_DELETED;
            break;
        }

        if( nDelMsg && !pDoc->IsInDtor() && GetNodes().IsDocNodes() )
        {
            SwPtrMsgPoolItem aMsgHint( nDelMsg, (void*)&pAttr->GetAttr() );
            pDoc->GetUnoCallBack()->Modify( &aMsgHint, &aMsgHint );
        }

        pAttr->RemoveFromPool( pDoc->GetAttrPool() );
        delete pAttr;
    }
}

/*************************************************************************
 *                      SwTxtNode::Insert()
 *************************************************************************/

// lege ein neues TextAttribut an und fuege es ins SwpHints-Array ein
SwTxtAttr* SwTxtNode::InsertItem( const SfxPoolItem& rAttr,
                                  xub_StrLen nStt, xub_StrLen nEnd, USHORT nMode )
{
   // character attributes will be inserted as automatic styles:
    ASSERT( rAttr.Which() < RES_CHRATR_BEGIN  || rAttr.Which() >= RES_CHRATR_END,
            "AUTOSTYLES - InsertItem should not be called with character attributes" )

    SwTxtAttr* pNew = MakeTxtAttr( rAttr, nStt, nEnd );

    if ( pNew )
       Insert( pNew, nMode );

    return pNew;
}

// uebernehme den Pointer auf das Text-Attribut
BOOL SwTxtNode::Insert( SwTxtAttr *pAttr, USHORT nMode )
{
    BOOL bHiddenPara = FALSE;

    ASSERT( pAttr && *pAttr->GetStart() <= Len(), "StartIdx hinter Len!" );

    if( !pAttr->GetEnd() )
    {
        USHORT nInsMode = nMode;
        switch( pAttr->Which() )
        {
            case RES_TXTATR_FLYCNT:
            {
                SwTxtFlyCnt *pFly = (SwTxtFlyCnt *)pAttr;
                SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                if( !(nsSetAttrMode::SETATTR_NOTXTATRCHR & nInsMode) )
                {
                    // Wir muessen zuerst einfuegen, da in SetAnchor()
                    // dem FlyFrm GetStart() uebermittelt wird.
                    //JP 11.05.98: falls das Anker-Attribut schon richtig
                    // gesetzt ist, dann korrigiere dieses nach dem Einfuegen
                    // des Zeichens. Sonst muesste das immer  ausserhalb
                    // erfolgen (Fehleranfaellig !)
                    const SwFmtAnchor* pAnchor = 0;
                    pFmt->GetItemState( RES_ANCHOR, FALSE,
                                            (const SfxPoolItem**)&pAnchor );

                    SwIndex aIdx( this, *pAttr->GetStart() );
                    Insert( GetCharOfTxtAttr(*pAttr), aIdx );
                    nInsMode |= nsSetAttrMode::SETATTR_NOTXTATRCHR;

                    if( pAnchor && FLY_IN_CNTNT == pAnchor->GetAnchorId() &&
                        pAnchor->GetCntntAnchor() &&
                        pAnchor->GetCntntAnchor()->nNode == *this &&
                        pAnchor->GetCntntAnchor()->nContent == aIdx )
                        ((SwIndex&)pAnchor->GetCntntAnchor()->nContent)--;
                }
                pFly->SetAnchor( this );

                // Format-Pointer kann sich im SetAnchor geaendert haben!
                // (Kopieren in andere Docs!)
                pFmt = pAttr->GetFlyCnt().GetFrmFmt();
                SwDoc *pDoc = pFmt->GetDoc();

                // OD 26.06.2003 #108784# - allow drawing objects in header/footer.
                // But don't allow control objects in header/footer
                if( RES_DRAWFRMFMT == pFmt->Which() &&
                    pDoc->IsInHeaderFooter( pFmt->GetAnchor().GetCntntAnchor()->nNode ) )
                {
                    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(pFmt->FindContactObj());
                    if ( pDrawContact &&
                         pDrawContact->GetMaster() &&
                         ::CheckControlLayer( pDrawContact->GetMaster() ) )
                    {
                        // das soll nicht meoglich sein; hier verhindern
                        // Der Dtor des TxtHints loescht nicht das Zeichen.
                        // Wenn ein CH_TXTATR_.. vorliegt, dann muss man
                        // dieses explizit loeschen
                        if( nsSetAttrMode::SETATTR_NOTXTATRCHR & nInsMode )
                        {
                            // loesche das Zeichen aus dem String !
                            ASSERT( ( CH_TXTATR_BREAKWORD ==
                                            aText.GetChar(*pAttr->GetStart() ) ||
                                      CH_TXTATR_INWORD ==
                                            aText.GetChar(*pAttr->GetStart())),
                                    "where is my attribu character" );
                            aText.Erase( *pAttr->GetStart(), 1 );
                            // Indizies Updaten
                            SwIndex aTmpIdx( this, *pAttr->GetStart() );
                            Update( aTmpIdx, 1, TRUE );
                        }
                        // Format loeschen nicht ins Undo aufnehmen!!
                        BOOL bUndo = pDoc->DoesUndo();
                        pDoc->DoUndo( FALSE );
                        DestroyAttr( pAttr );
                        pDoc->DoUndo( bUndo );
                        return FALSE;
                    }
                }
                break;
            }

            case RES_TXTATR_FTN :
            {
                // Fussnoten, man kommt an alles irgendwie heran.
                // CntntNode erzeugen und in die Inserts-Section stellen
                SwDoc *pDoc = GetDoc();
                SwNodes &rNodes = pDoc->GetNodes();

                // FussNote in nicht Content-/Redline-Bereich einfuegen ??
                if( StartOfSectionIndex() < rNodes.GetEndOfAutotext().GetIndex() )
                {
                    // das soll nicht meoglich sein; hier verhindern
                    // Der Dtor des TxtHints loescht nicht das Zeichen.
                    // Wenn ein CH_TXTATR_.. vorliegt, dann muss man
                    // dieses explizit loeschen
                    if( nsSetAttrMode::SETATTR_NOTXTATRCHR & nInsMode )
                    {
                        // loesche das Zeichen aus dem String !
                        ASSERT( ( CH_TXTATR_BREAKWORD ==
                                        aText.GetChar(*pAttr->GetStart() ) ||
                                  CH_TXTATR_INWORD ==
                                          aText.GetChar(*pAttr->GetStart())),
                                "where is my attribu character" );
                        aText.Erase( *pAttr->GetStart(), 1 );
                        // Indizies Updaten
                        SwIndex aTmpIdx( this, *pAttr->GetStart() );
                        Update( aTmpIdx, 1, TRUE );
                    }
                    DestroyAttr( pAttr );
                    return FALSE;
                }

                // wird eine neue Fussnote eingefuegt ??
                BOOL bNewFtn = 0 == ((SwTxtFtn*)pAttr)->GetStartNode();
                if( bNewFtn )
                {
                    ((SwTxtFtn*)pAttr)->MakeNewTextSection( GetNodes() );
                    SwRegHistory* pHist = GetpSwpHints() ? GetpSwpHints()->getHistory() : 0;
                    if( pHist )
                        pHist->ChangeNodeIndex( GetIndex() );
                }
                else if ( !GetpSwpHints() || !GetpSwpHints()->IsInSplitNode() )
                {
                    // loesche alle Frames der Section, auf die der StartNode zeigt
                    ULONG nSttIdx =
                        ((SwTxtFtn*)pAttr)->GetStartNode()->GetIndex();
                    ULONG nEndIdx = rNodes[ nSttIdx++ ]->EndOfSectionIndex();
                    SwCntntNode* pCNd;
                    for( ; nSttIdx < nEndIdx; ++nSttIdx )
                        if( 0 != ( pCNd = rNodes[ nSttIdx ]->GetCntntNode() ))
                            pCNd->DelFrms();
                }

                if( !(nsSetAttrMode::SETATTR_NOTXTATRCHR & nInsMode) )
                {
                    // Wir muessen zuerst einfuegen, da sonst gleiche Indizes
                    // entstehen koennen und das Attribut im _SortArr_ am
                    // Dokument nicht eingetrage wird.
                    SwIndex aNdIdx( this, *pAttr->GetStart() );
                    Insert( GetCharOfTxtAttr(*pAttr), aNdIdx );
                    nInsMode |= nsSetAttrMode::SETATTR_NOTXTATRCHR;
                }

                // Wir tragen uns am FtnIdx-Array des Docs ein ...
                SwTxtFtn* pTxtFtn = 0;
                if( !bNewFtn )
                {
                    // eine alte Ftn wird umgehaengt (z.B. SplitNode)
                    for( USHORT n = 0; n < pDoc->GetFtnIdxs().Count(); ++n )
                        if( pAttr == pDoc->GetFtnIdxs()[n] )
                        {
                            // neuen Index zuweisen, dafuer aus dem SortArray
                            // loeschen und neu eintragen
                            pTxtFtn = pDoc->GetFtnIdxs()[n];
                            pDoc->GetFtnIdxs().Remove( n );
                            break;
                        }
                    // wenn ueber Undo der StartNode gesetzt wurde, kann
                    // der Index noch gar nicht in der Verwaltung stehen !!
                }
                if( !pTxtFtn )
                    pTxtFtn = (SwTxtFtn*)pAttr;

                // fuers Update der Nummern und zum Sortieren
                // muss der Node gesetzt sein.
                ((SwTxtFtn*)pAttr)->ChgTxtNode( this );

                // FussNote im Redline-Bereich NICHT ins FtnArray einfuegen!
                if( StartOfSectionIndex() > rNodes.GetEndOfRedlines().GetIndex() )
                {
#ifndef PRODUCT
                    const BOOL bSuccess =
#endif
                        pDoc->GetFtnIdxs().Insert( pTxtFtn );
#ifndef PRODUCT
                    ASSERT( bSuccess, "FtnIdx nicht eingetragen." );
#endif
                }
                SwNodeIndex aTmpIndex( *this );
                pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
                ((SwTxtFtn*)pAttr)->SetSeqRefNo();
            }
            break;

            case RES_TXTATR_FIELD:
                {
                    // fuer HiddenParaFields Benachrichtigungsmechanismus
                    // anwerfen
                    if( RES_HIDDENPARAFLD ==
                        pAttr->GetFld().GetFld()->GetTyp()->Which() )
                    bHiddenPara = TRUE;
                }
                break;

        }
        // Fuer SwTxtHints ohne Endindex werden CH_TXTATR_..
        // eingefuegt, aStart muss danach um einen zurueckgesetzt werden.
        // Wenn wir im SwTxtNode::Copy stehen, so wurde das Zeichen bereits
        // mitkopiert. In solchem Fall ist SETATTR_NOTXTATRCHR angegeben worden.
        if( !(nsSetAttrMode::SETATTR_NOTXTATRCHR & nInsMode) )
        {
            SwIndex aIdx( this, *pAttr->GetStart() );
            Insert( GetCharOfTxtAttr(*pAttr), aIdx );
        }
    }
    else
        ASSERT( *pAttr->GetEnd() <= Len(), "EndIdx hinter Len!" );

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    // 4263: AttrInsert durch TextInsert => kein Adjust
    pSwpHints->Insert( pAttr, *this, nMode );

    // 47375: In pSwpHints->Insert wird u.a. Merge gerufen und das Hints-Array
    // von ueberfluessigen Hints befreit, dies kann u.U. sogar der frisch
    // eingefuegte Hint pAttr sein, der dann zerstoert wird!!
    if( USHRT_MAX == pSwpHints->GetPos( pAttr ) )
        return FALSE;

    if( bHiddenPara )
        SetCalcHiddenParaField();

    return TRUE;
}


/*************************************************************************
 *                      SwTxtNode::Delete()
 *************************************************************************/

void SwTxtNode::Delete( SwTxtAttr *pAttr, BOOL bThisOnly )
{
    if ( !pSwpHints )
        return;
    if( bThisOnly )
    {
        xub_StrLen* pEndIdx = pAttr->GetEnd();
        if( !pEndIdx )
        {
            // hat es kein Ende kann es nur das sein, was hier steht!
            // Unbedingt Copy-konstruieren!
            const SwIndex aIdx( this, *pAttr->GetStart() );
            Erase( aIdx, 1 );
        }
        else
        {
            // den MsgHint jetzt fuettern, weil gleich sind
            // Start und End weg.
            SwUpdateAttr aHint( *pAttr->GetStart(), *pEndIdx, pAttr->Which() );
            pSwpHints->Delete( pAttr );
            pAttr->RemoveFromPool( GetDoc()->GetAttrPool() );
            delete pAttr;
            SwModify::Modify( 0, &aHint );     // die Frames benachrichtigen

            if( pSwpHints && pSwpHints->CanBeDeleted() )
                DELETEZ( pSwpHints );
        }

        return;
    }
    Delete( pAttr->Which(), *pAttr->GetStart(), *pAttr->GetAnyEnd() );
}

/*************************************************************************
 *                      SwTxtNode::Delete()
 *************************************************************************/

void SwTxtNode::Delete( USHORT nTxtWhich, xub_StrLen nStt, xub_StrLen nEnd )
{
    if ( !pSwpHints )
        return;

    const xub_StrLen *pEndIdx;
    const xub_StrLen *pSttIdx;
    SwTxtAttr* pTxtHt;

    for( USHORT nPos = 0; pSwpHints && nPos < pSwpHints->Count(); nPos++ )
    {
        pTxtHt = pSwpHints->GetHt( nPos );
        const USHORT nWhich = pTxtHt->Which();
        if( nWhich == nTxtWhich &&
            *( pSttIdx = pTxtHt->GetStart()) == nStt )
        {
            if ( nWhich == RES_CHRATR_HIDDEN  )
                SetCalcHiddenCharFlags();
            else if ( nWhich == RES_TXTATR_CHARFMT )
            {
                // Check if character format contains hidden attribute:
                const SwCharFmt* pFmt = pTxtHt->GetCharFmt().GetCharFmt();
                const SfxPoolItem* pItem;
                if ( SFX_ITEM_SET == pFmt->GetItemState( RES_CHRATR_HIDDEN, TRUE, &pItem ) )
                    SetCalcHiddenCharFlags();
            }
            // --> FME 2007-03-16 #i75430# Recalc hidden flags if necessary
            else if ( nWhich == RES_TXTATR_AUTOFMT )
            {
                // Check if auto style contains hidden attribute:
                const SfxPoolItem* pHiddenItem = CharFmt::GetItem( *pTxtHt, RES_CHRATR_HIDDEN );
                if ( pHiddenItem )
                    SetCalcHiddenCharFlags();
            }
            // <--

            pEndIdx = pTxtHt->GetEnd();

            // Text-Attribute sind voellig dynamisch, so dass diese nur
            // mit ihrer Start-Position verglichen werden.
            if( !pEndIdx )
            {
                // Unbedingt Copy-konstruieren!
                const SwIndex aIdx( this, *pSttIdx );
                Erase( aIdx, 1 );
                break;
            }
            else if( *pEndIdx == nEnd )
            {
                // den MsgHint jetzt fuettern, weil gleich sind
                // Start und End weg.
                // Das CalcVisibleFlag bei HiddenParaFields entfaellt,
                // da dies das Feld im Dtor selbst erledigt.
                SwUpdateAttr aHint( *pSttIdx, *pEndIdx, nTxtWhich );
                pSwpHints->DeleteAtPos( nPos );    // gefunden, loeschen,
                pTxtHt->RemoveFromPool( GetDoc()->GetAttrPool() );
                delete pTxtHt;
                SwModify::Modify( 0, &aHint );     // die Frames benachrichtigen
                break;
            }
        }
    }
    if( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );
}

/*************************************************************************
 *                      SwTxtNode::DelSoftHyph()
 *************************************************************************/

void SwTxtNode::DelSoftHyph( const xub_StrLen nStt, const xub_StrLen nEnd )
{
    xub_StrLen nFndPos = nStt, nEndPos = nEnd;
    while( STRING_NOTFOUND !=
            ( nFndPos = aText.Search( CHAR_SOFTHYPHEN, nFndPos )) &&
            nFndPos < nEndPos )
    {
        const SwIndex aIdx( this, nFndPos );
        Erase( aIdx, 1 );
        --nEndPos;
    }
}

// setze diese Attribute am TextNode. Wird der gesamte Bereich umspannt,
// dann setze sie nur im AutoAttrSet (SwCntntNode:: SetAttr)
BOOL SwTxtNode::SetAttr( const SfxItemSet& rSet, xub_StrLen nStt,
                         xub_StrLen nEnd, USHORT nMode )
{
    if( !rSet.Count() )
        return FALSE;

    // teil die Sets auf (fuer Selektion in Nodes)
    const SfxItemSet* pSet = &rSet;
    SfxItemSet aTxtSet( *rSet.GetPool(), RES_TXTATR_BEGIN, RES_TXTATR_END-1 );

    // gesamter Bereich
    if( !nStt && nEnd == aText.Len() && !(nMode & nsSetAttrMode::SETATTR_NOFORMATATTR ) )
    {
        // sind am Node schon Zeichenvorlagen gesetzt, muss man diese Attribute
        // (rSet) immer als TextAttribute setzen, damit sie angezeigt werden.
        int bHasCharFmts = FALSE;
        if( pSwpHints )
            for( USHORT n = 0; n < pSwpHints->Count(); ++n )
                if( (*pSwpHints)[ n ]->IsCharFmtAttr() )
                {
                    bHasCharFmts = TRUE;
                    break;
                }

        if( !bHasCharFmts )
        {
            aTxtSet.Put( rSet );
            // If there are any character attributes in rSet,
            // we want to set them at the paragraph:
            if( aTxtSet.Count() != rSet.Count() )
            {
                BOOL bRet = SwCntntNode::SetAttr( rSet );
                  if( !aTxtSet.Count() )
                    return bRet;
            }

            // check for auto style:
            const SfxPoolItem* pItem;
            const bool bAutoStyle = SFX_ITEM_SET == aTxtSet.GetItemState( RES_TXTATR_AUTOFMT, FALSE, &pItem );
            if ( bAutoStyle )
            {
                boost::shared_ptr<SfxItemSet> pAutoStyleSet = static_cast<const SwFmtAutoFmt*>(pItem)->GetStyleHandle();
                BOOL bRet = SwCntntNode::SetAttr( *pAutoStyleSet );
                  if( 1 == aTxtSet.Count() )
                    return bRet;
            }

            // Continue with the text attributes:
            pSet = &aTxtSet;
        }
    }

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    SfxItemSet aCharSet( *rSet.GetPool(), aCharAutoFmtSetRange );

    USHORT nWhich, nCount = 0;
    SwTxtAttr* pNew;
    SfxItemIter aIter( *pSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();

    do
    {
        if( pItem && (SfxPoolItem*)-1 != pItem &&
            (( RES_CHRATR_BEGIN <= ( nWhich = pItem->Which()) &&
              RES_CHRATR_END > nWhich ) ||
            ( RES_TXTATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich ) ||
            ( RES_UNKNOWNATR_BEGIN <= nWhich && RES_UNKNOWNATR_END > nWhich )) )
        {
            if( RES_TXTATR_CHARFMT == pItem->Which() &&
                GetDoc()->GetDfltCharFmt()==((SwFmtCharFmt*)pItem)->GetCharFmt())
            {
                SwIndex aIndex( this, nStt );
                RstAttr( aIndex, nEnd - nStt, RES_TXTATR_CHARFMT, 0 );
                DontExpandFmt( aIndex );
            }
            else
            {
                if ( ( RES_CHRATR_BEGIN <= nWhich && RES_CHRATR_END > nWhich ) ||
                       RES_TXTATR_UNKNOWN_CONTAINER == nWhich )
                {
                    aCharSet.Put( *pItem );
                }
                else
                {
                    pNew = MakeTxtAttr( *pItem, nStt, nEnd );
                    if( pNew )
                    {
                        // Attribut ohne Ende, aber Bereich markiert ?
                        if( nEnd != nStt && !pNew->GetEnd() )
                        {
                            ASSERT( !this, "Attribut ohne Ende aber Bereich vorgegeben" );
                            DestroyAttr( pNew );        // nicht einfuegen
                        }
                        else if( Insert( pNew, nMode ))
                            ++nCount;
                    }
                }
            }
        }
        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    } while( TRUE );

    if ( aCharSet.Count() )
    {
        SwTxtAttr* pTmpNew = MakeTxtAttr( aCharSet, nStt, nEnd );
        if ( Insert( pTmpNew, nMode ) )
            ++nCount;
    }

    if( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );

    return nCount ? TRUE : FALSE;
}

void lcl_MergeAttr( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    if ( RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pCFSet = CharFmt::GetItemSet( rAttr );
        if ( !pCFSet )
            return;
        SfxWhichIter aIter( *pCFSet );
        USHORT nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( ( nWhich < RES_CHRATR_END ||
                  RES_TXTATR_UNKNOWN_CONTAINER == nWhich ) &&
                ( SFX_ITEM_SET == pCFSet->GetItemState( nWhich, TRUE ) ) )
                rSet.Put( pCFSet->Get( nWhich ) );
            nWhich = aIter.NextWhich();
        }
    }
    else
        rSet.Put( rAttr );
}

void lcl_MergeAttr_ExpandChrFmt( SfxItemSet& rSet, const SfxPoolItem& rAttr )
{
    if( RES_TXTATR_CHARFMT == rAttr.Which() ||
        RES_TXTATR_INETFMT == rAttr.Which() ||
        RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pCFSet = CharFmt::GetItemSet( rAttr );

        if ( pCFSet )
        {
            SfxWhichIter aIter( *pCFSet );
            USHORT nWhich = aIter.FirstWhich();
            while( nWhich )
            {
                if( ( nWhich < RES_CHRATR_END ||
                      ( RES_TXTATR_AUTOFMT == rAttr.Which() && RES_TXTATR_UNKNOWN_CONTAINER == nWhich ) ) &&
                    ( SFX_ITEM_SET == pCFSet->GetItemState( nWhich, TRUE ) ) )
                    rSet.Put( pCFSet->Get( nWhich ) );
                nWhich = aIter.NextWhich();
            }
        }
    }

    // aufnehmen als MergeWert (falls noch nicht gesetzt neu setzen!)

/* wenn mehrere Attribute ueberlappen gewinnt der letze !!
 z.B
            1234567890123456789
              |------------|        Font1
                 |------|           Font2
                    ^  ^
                    |--|        Abfragebereich: -> Gueltig ist Font2
*/
    rSet.Put( rAttr );
}

struct SwPoolItemEndPair
{
public:
    const SfxPoolItem* mpItem;
    xub_StrLen mnEndPos;

    SwPoolItemEndPair() : mpItem( 0 ), mnEndPos( 0 ) {};
};

// --> OD 2008-01-16 #newlistlevelattrs#
void lcl_MergeListLevelIndentAsLRSpaceItem( const SwTxtNode& rTxtNode,
                                            SfxItemSet& rSet )
{
    if ( rTxtNode.AreListLevelIndentsApplicable() )
    {
        const SwNumRule* pRule = rTxtNode.GetNumRule();
        if ( pRule && rTxtNode.GetLevel() >= 0 )
        {
            const SwNumFmt& rFmt = pRule->Get(static_cast<USHORT>(rTxtNode.GetLevel()));
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtLeft( rFmt.GetIndentAt() );
                aLR.SetTxtFirstLineOfst( static_cast<short>(rFmt.GetFirstLineIndent()) );
                rSet.Put( aLR );
            }
        }
    }
}

// erfrage die Attribute vom TextNode ueber den Bereich
// --> OD 2008-01-16 #newlistlevelattrs#
BOOL SwTxtNode::GetAttr( SfxItemSet& rSet, xub_StrLen nStt, xub_StrLen nEnd,
                         BOOL bOnlyTxtAttr, BOOL bGetFromChrFmt,
                         const bool bMergeIndentValuesOfNumRule ) const
{
    if( pSwpHints )
    {
        /* stelle erstmal fest, welche Text-Attribut in dem Bereich gueltig
         * sind. Dabei gibt es folgende Faelle:
         *  UnEindeutig wenn: (wenn != Format-Attribut)
         *      - das Attribut liegt vollstaendig im Bereich
         *      - das Attributende liegt im Bereich
         *      - der Attributanfang liegt im Bereich:
         * Eindeutig (im Set mergen):
         *      - das Attrib umfasst den Bereich
         * nichts tun:
         *      das Attribut liegt ausserhalb des Bereiches
         */

        void (*fnMergeAttr)( SfxItemSet&, const SfxPoolItem& )
            = bGetFromChrFmt ? &lcl_MergeAttr_ExpandChrFmt
                             : &lcl_MergeAttr;

        // dann besorge mal die Auto-(Fmt)Attribute
        SfxItemSet aFmtSet( *rSet.GetPool(), rSet.GetRanges() );
        if( !bOnlyTxtAttr )
        {
            SwCntntNode::GetAttr( aFmtSet );
            // --> OD 2008-01-16 #newlistlevelattrs#
            if ( bMergeIndentValuesOfNumRule )
            {
                lcl_MergeListLevelIndentAsLRSpaceItem( *this, aFmtSet );
            }
            // <--
        }

        const USHORT nSize = pSwpHints->Count();
        USHORT n;
        xub_StrLen nAttrStart;
        const xub_StrLen* pAttrEnd;

        if( nStt == nEnd )             // kein Bereich:
        {
            for( n = 0; n < nSize; ++n )        //
            {
                const SwTxtAttr* pHt = (*pSwpHints)[n];
                nAttrStart = *pHt->GetStart();
                if( nAttrStart > nEnd )         // ueber den Bereich hinaus
                    break;

                if( 0 == ( pAttrEnd = pHt->GetEnd() ))      // nie Attribute ohne Ende
                    continue;

                if( ( nAttrStart < nStt &&
                        ( pHt->DontExpand() ? nStt < *pAttrEnd
                                            : nStt <= *pAttrEnd )) ||
                    ( nStt == nAttrStart &&
                        ( nAttrStart == *pAttrEnd || !nStt )))
                    (*fnMergeAttr)( rSet, pHt->GetAttr() );
            }
        }
        else                            // es ist ein Bereich definiert
        {
            // --> FME 2007-03-13 #i75299#
            std::vector< SwPoolItemEndPair >* pAttrArr = 0;
            // <--

            const USHORT coArrSz = static_cast<USHORT>(RES_TXTATR_WITHEND_END) -
                                   static_cast<USHORT>(RES_CHRATR_BEGIN) +
                                   static_cast<USHORT>(RES_UNKNOWNATR_END) -
                                   static_cast<USHORT>(RES_UNKNOWNATR_BEGIN);

            for( n = 0; n < nSize; ++n )
            {
                const SwTxtAttr* pHt = (*pSwpHints)[n];
                nAttrStart = *pHt->GetStart();
                if( nAttrStart > nEnd )         // ueber den Bereich hinaus
                    break;

                if( 0 == ( pAttrEnd = pHt->GetEnd() ))      // nie Attribute ohne Ende
                    continue;

                BOOL bChkInvalid = FALSE;
                if( nAttrStart <= nStt )       // vor oder genau Start
                {
                    if( *pAttrEnd <= nStt )    // liegt davor
                        continue;

                    if( nEnd <= *pAttrEnd )     // hinter oder genau Ende
                        (*fnMergeAttr)( aFmtSet, pHt->GetAttr() );
                    else
//                  else if( pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
                        // uneindeutig
                        bChkInvalid = TRUE;
                }
                else if( nAttrStart < nEnd      // reicht in den Bereich
)//                      && pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
                    bChkInvalid = TRUE;

                if( bChkInvalid )
                {
                    // uneindeutig ?
                    SfxItemIter* pItemIter = 0;
                    const SfxPoolItem* pItem = 0;

                    if ( RES_TXTATR_AUTOFMT == pHt->Which() )
                    {
                        const SfxItemSet* pAutoSet = CharFmt::GetItemSet( pHt->GetAttr() );
                        if ( pAutoSet )
                        {
                            pItemIter = new SfxItemIter( *pAutoSet );
                            pItem = pItemIter->GetCurItem();
                        }
                    }
                    else
                        pItem = &pHt->GetAttr();

                    const USHORT nHintEnd = *pAttrEnd;

                    while ( pItem )
                    {
                        const USHORT nHintWhich = pItem->Which();

                        if( !pAttrArr )
                            pAttrArr = new std::vector< SwPoolItemEndPair >( coArrSz );

                        std::vector< SwPoolItemEndPair >::iterator pPrev = pAttrArr->begin();
                        if( RES_CHRATR_BEGIN <= nHintWhich && nHintWhich < RES_TXTATR_WITHEND_END )
                            pPrev += nHintWhich - RES_CHRATR_BEGIN;
                        else if( RES_UNKNOWNATR_BEGIN <= nHintWhich && nHintWhich < RES_UNKNOWNATR_END )
                            pPrev += nHintWhich - RES_UNKNOWNATR_BEGIN + (
                                    static_cast< USHORT >(RES_TXTATR_WITHEND_END) - static_cast< USHORT >(RES_CHRATR_BEGIN) );
                        else
                            pPrev = pAttrArr->end();

#if OSL_DEBUG_LEVEL > 1
                        SwPoolItemEndPair aTmp = *pPrev;
#endif

                        if( pPrev != pAttrArr->end() )
                        {
                            if( !pPrev->mpItem )
                            {
                                if ( bOnlyTxtAttr || *pItem != aFmtSet.Get( nHintWhich ) )
                                {
                                    if( nAttrStart > nStart )
                                    {
                                        rSet.InvalidateItem( nHintWhich );
                                        pPrev->mpItem = (SfxPoolItem*)-1;
                                    }
                                    else
                                    {
                                        pPrev->mpItem = pItem;
                                        pPrev->mnEndPos = nHintEnd;
                                    }
                                }
                            }
                            else if( (SfxPoolItem*)-1 != pPrev->mpItem )
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
                                    pPrev->mpItem = (SfxPoolItem*)-1;
                                }
                            }
                        }

                        pItem = ( pItemIter && !pItemIter->IsAtEnd() ) ? pItemIter->NextItem() : 0;
                    } // end while

                    delete pItemIter;
                }
            }

            if( pAttrArr )
            {
                for( n = 0; n < coArrSz; ++n )
                {
                    const SwPoolItemEndPair& rItemPair = (*pAttrArr)[ n ];
                    if( (0 != rItemPair.mpItem) && ((SfxPoolItem*)-1 != rItemPair.mpItem) )
                    {
                        USHORT nWh;
                        if( n < static_cast<USHORT>( static_cast<USHORT>(RES_TXTATR_WITHEND_END) -
                                                     static_cast<USHORT>(RES_CHRATR_BEGIN) ) )
                            nWh = static_cast<USHORT>(n + RES_CHRATR_BEGIN);
                        else
                            nWh = n - static_cast<USHORT>( static_cast<USHORT>(RES_TXTATR_WITHEND_END) -
                                                           static_cast<USHORT>(RES_CHRATR_BEGIN) +
                                                           static_cast<USHORT>(RES_UNKNOWNATR_BEGIN) );

                        if( nEnd <= rItemPair.mnEndPos ) // hinter oder genau Ende
                        {
                            if( *rItemPair.mpItem != aFmtSet.Get( nWh ) )
                                (*fnMergeAttr)( rSet, *rItemPair.mpItem );
                        }
                        else
                            // uneindeutig
                            rSet.InvalidateItem( nWh );
                    }
                }

                delete pAttrArr;
            }
        }
        if( aFmtSet.Count() )
        {
            // aus dem Format-Set alle entfernen, die im TextSet auch gesetzt sind
            aFmtSet.Differentiate( rSet );
            // jetzt alle zusammen "mergen"
            rSet.Put( aFmtSet );
        }
    }
    else if( !bOnlyTxtAttr )
    {
        // dann besorge mal die Auto-(Fmt)Attribute
        SwCntntNode::GetAttr( rSet );
        // --> OD 2008-01-16 #newlistlevelattrs#
        if ( bMergeIndentValuesOfNumRule )
        {
            lcl_MergeListLevelIndentAsLRSpaceItem( *this, rSet );
        }
        // <--
    }

    return rSet.Count() ? TRUE : FALSE;
}

int lcl_IsNewAttrInSet( const SwpHints& rHints, const SfxPoolItem& rItem,
                        const xub_StrLen nEnd )
{
    int bIns = TRUE;
    for( USHORT i = 0; i < rHints.Count(); ++i )
    {
        const SwTxtAttr *pOther = rHints[ i ];
        if( *pOther->GetStart() )
            break;

        if( pOther->GetEnd() &&
            *pOther->GetEnd() == nEnd &&
            ( pOther->IsCharFmtAttr() || pOther->Which() == rItem.Which() ) )
        {
            bIns = FALSE;
            break;
        }
    }
    return bIns;
}

void SwTxtNode::FmtToTxtAttr( SwTxtNode* pNd )
{
    SfxItemSet aThisSet( GetDoc()->GetAttrPool(), aCharFmtSetRange );
    if( HasSwAttrSet() && GetpSwAttrSet()->Count() )
        aThisSet.Put( *GetpSwAttrSet() );

    if ( !pSwpHints )
        pSwpHints = new SwpHints();

    if( pNd == this )
    {
        if( aThisSet.Count() )
        {
            SfxItemIter aIter( aThisSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            std::vector<USHORT> aClearWhichIds;

            while( TRUE )
            {
                if( lcl_IsNewAttrInSet( *pSwpHints, *pItem, GetTxt().Len() ) )
                {
                    pSwpHints->SwpHintsArr::Insert(
                            MakeTxtAttr( *pItem, 0, GetTxt().Len() ) );
                    aClearWhichIds.push_back( pItem->Which() );
                }

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }

            ClearItemsFromAttrSet( aClearWhichIds );
        }
    }
    else
    {
        SfxItemSet aNdSet( pNd->GetDoc()->GetAttrPool(), aCharFmtSetRange );
        if( pNd->HasSwAttrSet() && pNd->GetpSwAttrSet()->Count() )
            aNdSet.Put( *pNd->GetpSwAttrSet() );

        if ( !pNd->pSwpHints )
            pNd->pSwpHints = new SwpHints();

        if( aThisSet.Count() )
        {
            SfxItemIter aIter( aThisSet );
            const SfxPoolItem* pItem = aIter.GetCurItem(), *pNdItem;
            std::vector<USHORT> aClearWhichIds;

            while( TRUE )
            {
                if( ( SFX_ITEM_SET != aNdSet.GetItemState( pItem->Which(), FALSE,
                    &pNdItem ) || *pItem != *pNdItem ) &&
                    lcl_IsNewAttrInSet( *pSwpHints, *pItem, GetTxt().Len() ) )
                {
                    pSwpHints->SwpHintsArr::Insert(
                            MakeTxtAttr( *pItem, 0, GetTxt().Len() ) );
                    aClearWhichIds.push_back( pItem->Which() );
                }
                aNdSet.ClearItem( pItem->Which() );

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
            ClearItemsFromAttrSet( aClearWhichIds );
        }

        if( aNdSet.Count() )
        {
            SfxItemIter aIter( aNdSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            std::vector<USHORT> aClearWhichIds;

            while( TRUE )
            {
                if( lcl_IsNewAttrInSet( *pNd->pSwpHints, *pItem, pNd->GetTxt().Len() ) )
                    pNd->pSwpHints->SwpHintsArr::Insert(
                            pNd->MakeTxtAttr( *pItem, 0, pNd->GetTxt().Len() ) );
                aClearWhichIds.push_back( pItem->Which() );

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }

            pNd->ClearItemsFromAttrSet( aClearWhichIds );
            SwFmtChg aTmp1( pNd->GetFmtColl() );
            pNd->SwModify::Modify( &aTmp1, &aTmp1 );
        }
    }

    SetCalcHiddenCharFlags();

    if( pNd->pSwpHints->CanBeDeleted() )
        DELETEZ( pNd->pSwpHints );
}

/*************************************************************************
 *                      SwpHints::CalcFlags()
 *************************************************************************/

void SwpHints::CalcFlags()
{
    bDDEFlds = bFtn = FALSE;
    const USHORT nSize = Count();
    const SwTxtAttr* pAttr;
    for( USHORT nPos = 0; nPos < nSize; ++nPos )
        switch( ( pAttr = (*this)[ nPos ])->Which() )
        {
        case RES_TXTATR_FTN:
            bFtn = TRUE;
            if( bDDEFlds )
                return;
            break;
        case RES_TXTATR_FIELD:
            {
                const SwField* pFld = pAttr->GetFld().GetFld();
                if( RES_DDEFLD == pFld->GetTyp()->Which() )
                {
                    bDDEFlds = TRUE;
                    if( bFtn )
                        return;
                }
            }
            break;
        }
}

/*************************************************************************
 *                      SwpHints::CalcVisibleFlag()
 *************************************************************************/

BOOL SwpHints::CalcHiddenParaField()
{
    bCalcHiddenParaField = FALSE;
    BOOL bOldHasHiddenParaField = bHasHiddenParaField;
    BOOL bNewHasHiddenParaField  = FALSE;
    const USHORT    nSize = Count();
    const SwTxtAttr *pTxtHt;

    for( USHORT nPos = 0; nPos < nSize; ++nPos )
    {
        pTxtHt = (*this)[ nPos ];
        const USHORT nWhich = pTxtHt->Which();

        if( RES_TXTATR_FIELD == nWhich )
        {
            const SwFmtFld& rFld = pTxtHt->GetFld();
            if( RES_HIDDENPARAFLD == rFld.GetFld()->GetTyp()->Which() )
            {
                if( !((SwHiddenParaField*)rFld.GetFld())->IsHidden() )
                {
                    SetHiddenParaField(FALSE);
                    return bOldHasHiddenParaField != bNewHasHiddenParaField;
                }
                else
                    bNewHasHiddenParaField = TRUE;
            }
        }
    }
    SetHiddenParaField( bNewHasHiddenParaField );
    return bOldHasHiddenParaField != bNewHasHiddenParaField;
}


/*************************************************************************
 *                      SwpHints::NoteInHistory()
 *************************************************************************/

void SwpHints::NoteInHistory( SwTxtAttr *pAttr, const BOOL bNew )
{
    if ( pHistory )
        pHistory->Add( pAttr, bNew );
}

/*************************************************************************
 *                      SwpHints::MergePortions( )
 *************************************************************************/

bool SwpHints::MergePortions( SwTxtNode& rNode )
{
    if ( !Count() )
        return false;

    // sort before merging
    SwpHintsArr::Resort();

    bool bRet = false;
    typedef std::multimap< int, SwTxtAttr* > PortionMap;
    PortionMap aPortionMap;
    xub_StrLen nLastPorStart = STRING_LEN;
    USHORT i = 0;
    int nKey = 0;

    // get portions by start position:
    for ( i = 0; i < Count(); ++i )
    {
        SwTxtAttr *pHt = GetHt( i );
        if ( RES_TXTATR_CHARFMT != pHt->Which() &&
             RES_TXTATR_AUTOFMT != pHt->Which() )
             //&&
             //RES_TXTATR_INETFMT != pHt->Which() )
            continue;

        const xub_StrLen nPorStart = *pHt->GetStart();
        if ( nPorStart != nLastPorStart && nLastPorStart != STRING_LEN )
            ++nKey;
        nLastPorStart = nPorStart;
        aPortionMap.insert( std::pair< const int, SwTxtAttr* >( nKey, pHt ) );
    }

    // check if portion i can be merged with portion i+1:
    i = 0;
    int j = i + 1;
    while ( i <= nKey )
    {
        std::pair< PortionMap::iterator, PortionMap::iterator > aRange1 = aPortionMap.equal_range( i );
        std::pair< PortionMap::iterator, PortionMap::iterator > aRange2 = aPortionMap.equal_range( j );
        PortionMap::iterator aIter1 = aRange1.first;
        PortionMap::iterator aIter2 = aRange2.first;

        bool bMerge = true;
        const USHORT nAttributesInPor1  = static_cast<USHORT>(std::distance( aRange1.first, aRange1.second ));
        const USHORT nAttributesInPor2  = static_cast<USHORT>(std::distance( aRange2.first, aRange2.second ));

        if ( nAttributesInPor1 == nAttributesInPor2 && nAttributesInPor1 != 0 )
        {
            while ( aIter1 != aRange1.second )
            {
                const SwTxtAttr* p1 = (*aIter1).second;
                const SwTxtAttr* p2 = (*aIter2).second;
                if ( *p1->GetEnd() < *p2->GetStart() || p1->Which() != p2->Which() || !(*p1 == *p2) )
                {
                    bMerge = false;
                    break;
                }
                ++aIter1;
                ++aIter2;
            }
        }
        else
        {
            bMerge = false;
        }

        if ( bMerge )
        {
            // erase all elements with key i + 1
            xub_StrLen nNewPortionEnd = 0;
            for ( aIter2 = aRange2.first; aIter2 != aRange2.second; ++aIter2 )
            {
                SwTxtAttr* p2 = (*aIter2).second;
                nNewPortionEnd = *p2->GetEnd();

                const USHORT nCountBeforeDelete = Count();
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
                SwTxtAttr* p1 = (*aIter1).second;
                if( pHistory ) pHistory->Add( p1 );
                *p1->GetEnd() = nNewPortionEnd;
                if( pHistory ) pHistory->Add( p1, TRUE );
                bRet = true;
            }
        }
        else
        {
            ++i;
            j = i + 1;
        }
    }

    if ( bRet )
        SwpHintsArr::Resort();

    return bRet;
}

// check if there is already a character format and adjust the sort numbers
void lcl_CheckSortNumber( const SwpHints& rHints, SwTxtCharFmt& rNewCharFmt )
{
    const xub_StrLen nHtStart = *rNewCharFmt.GetStart();
    const xub_StrLen nHtEnd   = *rNewCharFmt.GetEnd();
    USHORT nSortNumber = 0;

    for ( USHORT i = 0; i < rHints.Count(); ++i )
    {
        const SwTxtAttr* pOtherHt = rHints[i];

        const xub_StrLen nOtherStart = *pOtherHt->GetStart();

        if ( nOtherStart > nHtStart )
            break;

        if ( RES_TXTATR_CHARFMT == pOtherHt->Which() )
        {
            const xub_StrLen nOtherEnd = *pOtherHt->GetEnd();

            if ( nOtherStart == nHtStart && nOtherEnd == nHtEnd )
            {
                const USHORT nOtherSortNum = static_cast<const SwTxtCharFmt*>(pOtherHt)->GetSortNumber();
                nSortNumber = nOtherSortNum + 1;
            }
        }
    }

    if ( nSortNumber > 0 )
        rNewCharFmt.SetSortNumber( nSortNumber );
}

/*************************************************************************
 *                      SwpHints::Insert()
 *************************************************************************/

/*
 * Insert: Der neue Hint wird immer eingefuegt. Wenn dabei ein
 * ueberlappender oder gleicher Hintbereich mit gleichem Attribut
 * und Wert gefunden, wird der neue Hint entsprechend veraendert
 * und der alte herausgenommen (und zerstoert:
 * SwpHints::Destroy()).
 */

void SwpHints::Insert( SwTxtAttr* pHint, SwTxtNode &rNode, USHORT nMode )
{
    // Irgendwann ist immer Schluss
    if( USHRT_MAX == Count() )
        return;

    // Felder bilden eine Ausnahme:
    // 1) Sie koennen nie ueberlappen
    // 2) Wenn zwei Felder genau aneinander liegen,
    //    sollen sie nicht zu einem verschmolzen werden.
    // Wir koennen also auf die while-Schleife verzichten

    xub_StrLen *pHtEnd = pHint->GetEnd();
    USHORT nWhich = pHint->Which();

    switch( nWhich )
    {
    case RES_TXTATR_CHARFMT:
    {
        // Check if character format contains hidden attribute:
        const SwCharFmt* pFmt = pHint->GetCharFmt().GetCharFmt();
        const SfxPoolItem* pItem;
        if ( SFX_ITEM_SET == pFmt->GetItemState( RES_CHRATR_HIDDEN, TRUE, &pItem ) )
            rNode.SetCalcHiddenCharFlags();

        ((SwTxtCharFmt*)pHint)->ChgTxtNode( &rNode );
        break;
    }
    // --> FME 2007-03-16 #i75430# Recalc hidden flags if necessary
    case RES_TXTATR_AUTOFMT:
    {
        // Check if auto style contains hidden attribute:
        const SfxPoolItem* pHiddenItem = CharFmt::GetItem( *pHint, RES_CHRATR_HIDDEN );
        if ( pHiddenItem )
            rNode.SetCalcHiddenCharFlags();
        break;
    }
    // <--
    case RES_TXTATR_INETFMT:
        {
            ((SwTxtINetFmt*)pHint)->ChgTxtNode( &rNode );
            SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
            pFmt->Add( (SwTxtINetFmt*)pHint );
        }
        break;
    case RES_TXTATR_FIELD:
        {
            BOOL bDelFirst = 0 != ((SwTxtFld*)pHint)->GetpTxtNode();
            ((SwTxtFld*)pHint)->ChgTxtNode( &rNode );
            SwDoc* pDoc = rNode.GetDoc();
            const SwField* pFld = ((SwTxtFld*)pHint)->GetFld().GetFld();

            if( !pDoc->IsNewFldLst() )
            {
                // was fuer ein Feld ist es denn ??
                // bestimmte Felder mussen am Doc das Calculations-Flag updaten
                switch( pFld->GetTyp()->Which() )
                {
                case RES_DBFLD:
                case RES_SETEXPFLD:
                case RES_HIDDENPARAFLD:
                case RES_HIDDENTXTFLD:
                case RES_DBNUMSETFLD:
                case RES_DBNEXTSETFLD:
                    {
                        if( bDelFirst )
                            pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pHint );
                        if( rNode.GetNodes().IsDocNodes() )
                            pDoc->InsDelFldInFldLst( TRUE, *(SwTxtFld*)pHint );
                    }
                    break;
                case RES_DDEFLD:
                    if( rNode.GetNodes().IsDocNodes() )
                        ((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
                    break;
                }
            }

            // gehts ins normale Nodes-Array?
            if( rNode.GetNodes().IsDocNodes() )
            {
                BOOL bInsFldType = FALSE;
                switch( pFld->GetTyp()->Which() )
                {
                case RES_SETEXPFLD:
                    bInsFldType = ((SwSetExpFieldType*)pFld->GetTyp())->IsDeleted();
                    if( nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType*)pFld->GetTyp())->GetType() )
                    {
                        // bevor die ReferenzNummer gesetzt wird, sollte
                        // das Feld am richtigen FeldTypen haengen!
                        SwSetExpFieldType* pFldType = (SwSetExpFieldType*)
                                    pDoc->InsertFldType( *pFld->GetTyp() );
                        if( pFldType != pFld->GetTyp() )
                        {
                            SwFmtFld* pFmtFld = (SwFmtFld*)&((SwTxtFld*)pHint)
                                                                ->GetFld();
                            pFldType->Add( pFmtFld );          // ummelden
                            pFmtFld->GetFld()->ChgTyp( pFldType );
                        }
                        pFldType->SetSeqRefNo( *(SwSetExpField*)pFld );
                    }
                    break;
                case RES_USERFLD:
                    bInsFldType = ((SwUserFieldType*)pFld->GetTyp())->IsDeleted();
                    break;

                case RES_DDEFLD:
                    if( pDoc->IsNewFldLst() )
                        ((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
                    bInsFldType = ((SwDDEFieldType*)pFld->GetTyp())->IsDeleted();
                    break;

                case RES_POSTITFLD:
                    if ( pDoc->GetDocShell() )
                        pDoc->GetDocShell()->Broadcast( SwFmtFldHint( &((SwTxtFld*)pHint)->GetFld(), SWFMTFLD_INSERTED ) );
                    break;
                }
                if( bInsFldType )
                    pDoc->InsDeletedFldType( *pFld->GetTyp() );
            }
        }
        break;
    case RES_TXTATR_FTN :
        ((SwTxtFtn*)pHint)->ChgTxtNode( &rNode );
        break;
    case RES_TXTATR_REFMARK:
        ((SwTxtRefMark*)pHint)->ChgTxtNode( &rNode );
        if( rNode.GetNodes().IsDocNodes() )
        {
            //search for a refernce with the same name
            SwTxtAttr* pTmpHt;
            xub_StrLen *pTmpHtEnd, *pTmpHintEnd;
            for( USHORT n = 0, nEnd = Count(); n < nEnd; ++n )
            {
                if( RES_TXTATR_REFMARK == (pTmpHt = GetHt( n ))->Which() &&
                    pHint->GetAttr() == pTmpHt->GetAttr() &&
                    0 != ( pTmpHtEnd = pTmpHt->GetEnd() ) &&
                    0 != ( pTmpHintEnd = pHint->GetEnd() ) )
                {
                    SwComparePosition eCmp = ::ComparePosition(
                            *pTmpHt->GetStart(), *pTmpHtEnd,
                            *pHint->GetStart(), *pTmpHintEnd );
                    BOOL bDelOld = TRUE, bChgStart = FALSE, bChgEnd = FALSE;
                    switch( eCmp )
                    {
                    case POS_BEFORE:
                    case POS_BEHIND:    bDelOld = FALSE; break;

                    case POS_OUTSIDE:   bChgStart = bChgEnd = TRUE; break;

                    case POS_COLLIDE_END:
                    case POS_OVERLAP_BEFORE:    bChgStart = TRUE; break;
                    case POS_COLLIDE_START:
                    case POS_OVERLAP_BEHIND:    bChgEnd = TRUE; break;
                    default: break;
                    }

                    if( bChgStart )
                        *pHint->GetStart() = *pTmpHt->GetStart();
                    if( bChgEnd )
                        *pTmpHintEnd = *pTmpHtEnd;

                    if( bDelOld )
                    {
                        if( pHistory )
                            pHistory->Add( pTmpHt );
                        rNode.DestroyAttr( Cut( n-- ) );
                        --nEnd;
                    }
                }
            }
        }
        break;
    case RES_TXTATR_TOXMARK:
        ((SwTxtTOXMark*)pHint)->ChgTxtNode( &rNode );
        break;

    case RES_TXTATR_CJK_RUBY:
        {
            ((SwTxtRuby*)pHint)->ChgTxtNode( &rNode );
            SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool(
                                                    RES_POOLCHR_RUBYTEXT );
            pFmt->Add( (SwTxtRuby*)pHint );
        }
        break;

    case RES_CHRATR_HIDDEN:
        rNode.SetCalcHiddenCharFlags();
        break;
    }

    if( nsSetAttrMode::SETATTR_DONTEXPAND & nMode )
        pHint->SetDontExpand( TRUE );

    // SwTxtAttrs ohne Ende werden sonderbehandelt:
    // Sie werden natuerlich in das Array insertet, aber sie werden nicht
    // in die pPrev/Next/On/Off-Verkettung aufgenommen.
    // Der Formatierer erkennt diese TxtHints an dem CH_TXTATR_.. im Text !
    xub_StrLen nHtStart = *pHint->GetStart();
    if( !pHtEnd )
    {
        SwpHintsArr::Insert( pHint );
        CalcFlags();
#ifndef PRODUCT
        if( !rNode.GetDoc()->IsInReading() )
            CHECK;
#endif
        // ... und die Abhaengigen benachrichtigen
        if ( rNode.GetDepends() )
        {
            SwUpdateAttr aHint( nHtStart, nHtStart, nWhich );
            rNode.Modify( 0, &aHint );
        }
        return;
    }

    // ----------------------------------------------------------------
    // Ab hier gibt es nur noch pHint mit einem EndIdx !!!

    if( *pHtEnd < nHtStart )
    {
        ASSERT( *pHtEnd >= nHtStart,
                    "+SwpHints::Insert: invalid hint, end < start" );

        // Wir drehen den Quatsch einfach um:
        *pHint->GetStart() = *pHtEnd;
        *pHtEnd = nHtStart;
        nHtStart = *pHint->GetStart();
    }

    // I need this value later on for notification but the pointer may become invalid
    const xub_StrLen nHintEnd = *pHtEnd;
    const bool bNoHintAdjustMode = (nsSetAttrMode::SETATTR_NOHINTADJUST & nMode);

    // Currently REFMARK and TOXMARK have OverlapAllowed set to true.
    // These attributes may be inserted directly.
    // Also attributes without length may be inserted directly.
    // SETATTR_NOHINTADJUST is set e.g., during undo.
    // Portion building in not necessary during XML import.
    if ( !bNoHintAdjustMode &&
         !pHint->IsOverlapAllowedAttr() &&
         !rNode.GetDoc()->IsInXMLImport() &&
         ( RES_TXTATR_AUTOFMT == nWhich ||
           RES_TXTATR_INETFMT == nWhich ||
           RES_TXTATR_CHARFMT == nWhich ||
           RES_TXTATR_CJK_RUBY == nWhich ) )
    {
        ASSERT( nWhich != RES_TXTATR_AUTOFMT ||
                static_cast<const SwFmtAutoFmt&>(pHint->GetAttr()).GetStyleHandle()->GetPool() ==
                &rNode.GetDoc()->GetAttrPool(),
                "AUTOSTYLES - Pool mismatch" )

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
        if ( ( RES_TXTATR_CHARFMT  == nWhich && !bNoHintAdjustMode ) ||
               RES_TXTATR_CJK_RUBY == nWhich ||
               RES_TXTATR_INETFMT  == nWhich )
            BuildPortions( rNode, *pHint, nMode );
        else
        {
            // --> FME 2007-11-08 #i82989# Check sort numbers in NoHintAdjustMode
            if ( RES_TXTATR_CHARFMT == nWhich )
                lcl_CheckSortNumber( *this, *static_cast<SwTxtCharFmt*>(pHint) );
            // <--

            SwpHintsArr::Insert( pHint );
            if ( pHistory )
                pHistory->Add( pHint, TRUE );
        }
    }

    // ... und die Abhaengigen benachrichtigen
    if ( rNode.GetDepends() )
    {
        SwUpdateAttr aHint( nHtStart, nHtStart == nHintEnd ? nHintEnd + 1 : nHintEnd, nWhich );
        rNode.Modify( 0, &aHint );
    }

#ifndef PRODUCT
    if( !bNoHintAdjustMode && !rNode.GetDoc()->IsInReading() )
        CHECK;
#endif

    return;
}

/*************************************************************************
 *                      SwpHints::DeleteAtPos()
 *************************************************************************/

void SwpHints::DeleteAtPos( const USHORT nPos )
{
    SwTxtAttr *pHint = GetHt(nPos);
    // ChainDelete( pHint );
    if( pHistory ) pHistory->Add( pHint );
    SwpHintsArr::DeleteAtPos( nPos );

    if( RES_TXTATR_FIELD == pHint->Which() )
    {
        SwFieldType* pFldTyp = ((SwTxtFld*)pHint)->GetFld().GetFld()->GetTyp();
        if( RES_DDEFLD == pFldTyp->Which() )
        {
            const SwTxtNode* pNd = ((SwTxtFld*)pHint)->GetpTxtNode();
            if( pNd && pNd->GetNodes().IsDocNodes() )
                ((SwDDEFieldType*)pFldTyp)->DecRefCnt();
            ((SwTxtFld*)pHint)->ChgTxtNode( 0 );
        }
        else if( RES_POSTITFLD == pFldTyp->Which() )
        {
            const_cast<SwFmtFld&>(((SwTxtFld*)pHint)->GetFld()).Broadcast( SwFmtFldHint( &((SwTxtFld*)pHint)->GetFld(), SWFMTFLD_REMOVED ) );
        }
        else if( bHasHiddenParaField && RES_HIDDENPARAFLD == pFldTyp->Which() )
            bCalcHiddenParaField = TRUE;
    }

    CalcFlags();
    CHECK;
}

// Ist der Hint schon bekannt, dann suche die Position und loesche ihn.
// Ist er nicht im Array, so gibt es ein ASSERT !!

void SwpHints::Delete( SwTxtAttr* pTxtHt )
{
    // Attr 2.0: SwpHintsArr::Delete( pTxtHt );
    const USHORT nPos = GetStartOf( pTxtHt );
    ASSERT( USHRT_MAX != nPos, "Attribut nicht im Attribut-Array!" );
    if( USHRT_MAX != nPos )
        DeleteAtPos( nPos );
}

void SwTxtNode::ClearSwpHintsArr( bool bDelFields )
{
    if( pSwpHints )
    {
        USHORT nPos = 0;
        while( nPos < pSwpHints->Count() )
        {
            SwTxtAttr* pDel = pSwpHints->GetHt( nPos );
            bool bDel = false;

            switch( pDel->Which() )
            {
            case RES_TXTATR_FLYCNT:
            case RES_TXTATR_FTN:
                break;

            case RES_TXTATR_FIELD:
            case RES_TXTATR_HARDBLANK:
                if( bDelFields )
                    bDel = true;
                break;
            default:
                bDel = true; break;
            }

            if( bDel )
            {
                pSwpHints->SwpHintsArr::DeleteAtPos( nPos );
                DestroyAttr( pDel );
            }
            else
                ++nPos;
        }
    }
}

USHORT SwTxtNode::GetLang( const xub_StrLen nBegin, const xub_StrLen nLen,
                           USHORT nScript ) const
{
    USHORT nWhichId = RES_CHRATR_LANGUAGE;
    USHORT nRet = LANGUAGE_DONTKNOW;
    if( pSwpHints )
    {
        if ( ! nScript )
            nScript = pBreakIt->GetRealScriptOfText( aText, nBegin );

        nWhichId = GetWhichOfScript( nWhichId, nScript );

        xub_StrLen nEnd = nBegin + nLen;
        for( USHORT i = 0, nSize = pSwpHints->Count(); i < nSize; ++i )
        {
            // ist der Attribut-Anfang schon groesser als der Idx ?
            const SwTxtAttr *pHt = pSwpHints->operator[](i);
            xub_StrLen nAttrStart = *pHt->GetStart();
            if( nEnd < nAttrStart )
                break;

            const USHORT nWhich = pHt->Which();

            if( nWhichId == nWhich ||
                    ( ( pHt->IsCharFmtAttr() || RES_TXTATR_AUTOFMT == nWhich ) && CharFmt::IsItemIncluded( nWhichId, pHt ) ) )
            {
                const xub_StrLen *pEndIdx = pHt->GetEnd();
                // Ueberlappt das Attribut den Bereich?

                if( pEndIdx &&
                    nLen ? ( nAttrStart < nEnd && nBegin < *pEndIdx )
                         : (( nAttrStart < nBegin &&
                                ( pHt->DontExpand() ? nBegin < *pEndIdx
                                                    : nBegin <= *pEndIdx )) ||
                            ( nBegin == nAttrStart &&
                                ( nAttrStart == *pEndIdx || !nBegin ))) )
                {
                    const SfxPoolItem* pItem = CharFmt::GetItem( *pHt, nWhichId );
                    USHORT nLng = ((SvxLanguageItem*)pItem)->GetLanguage();

                    // Umfasst das Attribut den Bereich komplett?
                    if( nAttrStart <= nBegin && nEnd <= *pEndIdx )
                        nRet = nLng;
                    else if( LANGUAGE_DONTKNOW == nRet )
                        nRet = nLng; // partielle Ueberlappung, der 1. gewinnt
                }
            }
        }
    }
    if( LANGUAGE_DONTKNOW == nRet )
    {
        if( !pSwpHints )
            nWhichId = GetWhichOfScript( RES_CHRATR_LANGUAGE,
                        pBreakIt->GetRealScriptOfText( aText, nBegin ));

        nRet = ((SvxLanguageItem&)GetSwAttrSet().Get( nWhichId )).GetLanguage();
        if( LANGUAGE_DONTKNOW == nRet )
            nRet = static_cast<USHORT>(GetAppLanguage());
    }
    return nRet;
}


sal_Unicode GetCharOfTxtAttr( const SwTxtAttr& rAttr )
{
    sal_Unicode cRet = CH_TXTATR_BREAKWORD;
    switch ( rAttr.Which() )
    {
    case RES_TXTATR_REFMARK:
    case RES_TXTATR_TOXMARK:

//  case RES_TXTATR_FIELD:          ??????
//  case RES_TXTATR_FLYCNT,                             // 29

    case RES_TXTATR_FTN:
        cRet = CH_TXTATR_INWORD;
        break;

        // depends on the character ??
//  case RES_TXTATR_HARDBLANK:
//      cRet = CH_TXTATR_INWORD;
//      break;
    }
    return cRet;
}


