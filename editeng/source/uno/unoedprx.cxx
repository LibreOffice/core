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



//
// Global header
//


#include <limits.h>
#include <vector>
#include <algorithm>
#include <osl/mutex.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>


//
// Project-local header
//

#include "editeng/unoedprx.hxx"
#include <editeng/unotext.hxx>
#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/AccessibleStringWrap.hxx>
#include <editeng/outliner.hxx>

using namespace ::com::sun::star;


class SvxAccessibleTextIndex
{
public:
    SvxAccessibleTextIndex() :
        mnPara(0),
        mnIndex(0),
        mnEEIndex(0),
        mnFieldOffset(0),
        mnFieldLen(0),
        mbInField(sal_False),
        mnBulletOffset(0),
        mnBulletLen(0),
        mbInBullet(sal_False) {};
    ~SvxAccessibleTextIndex() {};

    // Get/Set current paragraph
    void SetParagraph( sal_Int32 nPara )
    {
        mnPara = nPara;
    }
    sal_Int32 GetParagraph() const { return mnPara; }

    /** Set the index in the UAA semantic

        @param nIndex
         The index from the UA API (fields and bullets are expanded)

        @param rTF
        The text forwarder to use in the calculations
     */
    void SetIndex( sal_Int32 nIndex, const SvxTextForwarder& rTF );
    void SetIndex( sal_Int32 nPara, sal_Int32 nIndex, const SvxTextForwarder& rTF ) { SetParagraph(nPara); SetIndex(nIndex, rTF); }
    sal_Int32 GetIndex() const { return mnIndex; }

    /** Set the index in the edit engine semantic

        Update the object state to reflect the given index position in
        EditEngine/Outliner index values

        @param nEEIndex
         The index from the edit engine (fields span exactly one index increment)

        @param rTF
        The text forwarder to use in the calculations
     */
    void SetEEIndex( sal_uInt16 nEEIndex, const SvxTextForwarder& rTF );
    void SetEEIndex( sal_Int32 nPara, sal_uInt16 nEEIndex, const SvxTextForwarder& rTF ) { SetParagraph(nPara); SetEEIndex(nEEIndex, rTF); }
    sal_uInt16 GetEEIndex() const;

    void SetFieldOffset( sal_Int32 nOffset, sal_Int32 nLen ) { mnFieldOffset = nOffset; mnFieldLen = nLen; }
    sal_Int32 GetFieldOffset() const { return mnFieldOffset; }
    sal_Int32 GetFieldLen() const { return mnFieldLen; }
    void AreInField( sal_Bool bInField = sal_True ) { mbInField = bInField; }
    sal_Bool InField() const { return mbInField; }

    void SetBulletOffset( sal_Int32 nOffset, sal_Int32 nLen ) { mnBulletOffset = nOffset; mnBulletLen = nLen; }
    sal_Int32 GetBulletOffset() const { return mnBulletOffset; }
    sal_Int32 GetBulletLen() const { return mnBulletLen; }
    void AreInBullet( sal_Bool bInBullet = sal_True ) { mbInBullet = bInBullet; }
    sal_Bool InBullet() const { return mbInBullet; }

    /// returns false if the given range is non-editable (e.g. contains bullets or _parts_ of fields)
    sal_Bool IsEditableRange( const SvxAccessibleTextIndex& rEnd ) const;

private:
    sal_Int32 mnPara;
    sal_Int32 mnIndex;
    sal_Int32 mnEEIndex;
    sal_Int32 mnFieldOffset;
    sal_Int32 mnFieldLen;
    sal_Bool  mbInField;
    sal_Int32 mnBulletOffset;
    sal_Int32 mnBulletLen;
    sal_Bool  mbInBullet;
};

ESelection MakeEESelection( const SvxAccessibleTextIndex& rStart, const SvxAccessibleTextIndex& rEnd )
{
    // deal with field special case: to really get a field contained
    // within a selection, the start index must be before or on the
    // field, the end index after it.

    // The SvxAccessibleTextIndex.GetEEIndex method gives the index on
    // the field, as long the input index is on the field. Thus,
    // correction necessary for the end index

    // Therefore, for _ranges_, if part of the field is touched, all
    // of the field must be selected
    if( rStart.GetParagraph() <= rEnd.GetParagraph() ||
        (rStart.GetParagraph() == rEnd.GetParagraph() &&
         rStart.GetEEIndex() <= rEnd.GetEEIndex()) )
    {
        if( rEnd.InField() && rEnd.GetFieldOffset() )
            return ESelection( rStart.GetParagraph(), rStart.GetEEIndex(),
                               rEnd.GetParagraph(), rEnd.GetEEIndex()+1 );
    }
    else if( rStart.GetParagraph() > rEnd.GetParagraph() ||
             (rStart.GetParagraph() == rEnd.GetParagraph() &&
              rStart.GetEEIndex() > rEnd.GetEEIndex()) )
    {
        if( rStart.InField() && rStart.GetFieldOffset()  )
            return ESelection( rStart.GetParagraph(), rStart.GetEEIndex()+1,
                               rEnd.GetParagraph(), rEnd.GetEEIndex() );
    }

    return ESelection( rStart.GetParagraph(), rStart.GetEEIndex(),
                       rEnd.GetParagraph(), rEnd.GetEEIndex() );
}

ESelection MakeEESelection( const SvxAccessibleTextIndex& rIndex )
{
    return ESelection( rIndex.GetParagraph(), rIndex.GetEEIndex(),
                       rIndex.GetParagraph(), rIndex.GetEEIndex() + 1 );
}

sal_uInt16 SvxAccessibleTextIndex::GetEEIndex() const
{
    DBG_ASSERT(mnEEIndex >= 0 && mnEEIndex <= USHRT_MAX,
               "SvxAccessibleTextIndex::GetEEIndex: index value overflow");

    return static_cast< sal_uInt16 > (mnEEIndex);
}

void SvxAccessibleTextIndex::SetEEIndex( sal_uInt16 nEEIndex, const SvxTextForwarder& rTF )
{
    // reset
    mnFieldOffset = 0;
    mbInField = sal_False;
    mnFieldLen = 0;
    mnBulletOffset = 0;
    mbInBullet = sal_False;
    mnBulletLen = 0;

    // set known values
    mnEEIndex = nEEIndex;

    // calculate unknowns
    sal_Int32 nCurrField, nFieldCount = rTF.GetFieldCount( GetParagraph() );

    mnIndex = nEEIndex;

    EBulletInfo aBulletInfo = rTF.GetBulletInfo( GetParagraph() );

    // any text bullets?
    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType != SVX_NUM_BITMAP )
    {
        mnIndex += aBulletInfo.aText.getLength();
    }

    for( nCurrField=0; nCurrField < nFieldCount; ++nCurrField )
    {
        EFieldInfo aFieldInfo( rTF.GetFieldInfo( GetParagraph(), nCurrField ) );

        if( aFieldInfo.aPosition.nIndex > nEEIndex )
            break;

        if( aFieldInfo.aPosition.nIndex == nEEIndex )
        {
            AreInField();
            break;
        }

        // #106010#
        mnIndex += ::std::max(aFieldInfo.aCurrentText.getLength()-1, (sal_Int32)0);
    }
}

void SvxAccessibleTextIndex::SetIndex( sal_Int32 nIndex, const SvxTextForwarder& rTF )
{
    // reset
    mnFieldOffset = 0;
    mbInField = sal_False;
    mnFieldLen = 0;
    mnBulletOffset = 0;
    mbInBullet = sal_False;
    mnBulletLen = 0;

    // set known values
    mnIndex = nIndex;

    // calculate unknowns
    sal_Int32 nCurrField, nFieldCount = rTF.GetFieldCount( GetParagraph() );

    DBG_ASSERT(nIndex >= 0 && nIndex <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");

    mnEEIndex = nIndex;

    EBulletInfo aBulletInfo = rTF.GetBulletInfo( GetParagraph() );

    // any text bullets?
    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType != SVX_NUM_BITMAP )
    {
        sal_Int32 nBulletLen = aBulletInfo.aText.getLength();

        if( nIndex < nBulletLen )
        {
            AreInBullet();
            SetBulletOffset( nIndex, nBulletLen );
            mnEEIndex = 0;
            return;
        }

        mnEEIndex = mnEEIndex - nBulletLen;
    }

    for( nCurrField=0; nCurrField < nFieldCount; ++nCurrField )
    {
        EFieldInfo aFieldInfo( rTF.GetFieldInfo( GetParagraph(), nCurrField ) );

        // we're before a field
        if( aFieldInfo.aPosition.nIndex > mnEEIndex )
            break;

        // #106010#
        mnEEIndex -= ::std::max(aFieldInfo.aCurrentText.getLength()-1, (sal_Int32)0);

        // we're within a field
        if( aFieldInfo.aPosition.nIndex >= mnEEIndex )
        {
            AreInField();
            SetFieldOffset( ::std::max(aFieldInfo.aCurrentText.getLength()-1, (sal_Int32)0) - (aFieldInfo.aPosition.nIndex - mnEEIndex),
                            aFieldInfo.aCurrentText.getLength() );
            mnEEIndex = aFieldInfo.aPosition.nIndex ;
            break;
        }
    }
}

sal_Bool SvxAccessibleTextIndex::IsEditableRange( const SvxAccessibleTextIndex& rEnd ) const
{
    if( GetIndex() > rEnd.GetIndex() )
        return rEnd.IsEditableRange( *this );

    if( InBullet() || rEnd.InBullet() )
        return sal_False;

    if( InField() && GetFieldOffset() )
        return sal_False; // within field

    if( rEnd.InField() && rEnd.GetFieldOffset() >= rEnd.GetFieldLen() - 1 )
        return sal_False; // within field

    return sal_True;
}



SvxEditSourceAdapter::SvxEditSourceAdapter() : mbEditSourceValid( sal_False )
{
}

SvxEditSourceAdapter::~SvxEditSourceAdapter()
{
}

SvxEditSource* SvxEditSourceAdapter::Clone() const
{
    if( mbEditSourceValid && mpAdaptee.get() )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< SvxEditSource > pClonedAdaptee( mpAdaptee->Clone() );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if( pClonedAdaptee.get() )
        {
            SvxEditSourceAdapter* pClone = new SvxEditSourceAdapter();

            if( pClone )
            {
                pClone->SetEditSource( pClonedAdaptee );
                return pClone;
            }
        }
    }

    return NULL;
}

SvxAccessibleTextAdapter* SvxEditSourceAdapter::GetTextForwarderAdapter()
{
    if( mbEditSourceValid && mpAdaptee.get() )
    {
        SvxTextForwarder* pTextForwarder = mpAdaptee->GetTextForwarder();

        if( pTextForwarder )
        {
            maTextAdapter.SetForwarder(*pTextForwarder);

            return &maTextAdapter;
        }
    }

    return NULL;
}

SvxTextForwarder* SvxEditSourceAdapter::GetTextForwarder()
{
    return GetTextForwarderAdapter();
}

SvxViewForwarder* SvxEditSourceAdapter::GetViewForwarder()
{
    if( mbEditSourceValid && mpAdaptee.get() )
        return mpAdaptee->GetViewForwarder();

    return NULL;
}

SvxAccessibleTextEditViewAdapter* SvxEditSourceAdapter::GetEditViewForwarderAdapter( sal_Bool bCreate )
{
    if( mbEditSourceValid && mpAdaptee.get() )
    {
        SvxEditViewForwarder* pEditViewForwarder = mpAdaptee->GetEditViewForwarder(bCreate);

        if( pEditViewForwarder )
        {
            SvxAccessibleTextAdapter* pTextAdapter = GetTextForwarderAdapter();

            if( pTextAdapter )
            {
                maEditViewAdapter.SetForwarder(*pEditViewForwarder, *pTextAdapter);

                return &maEditViewAdapter;
            }
        }
    }

    return NULL;
}

SvxEditViewForwarder* SvxEditSourceAdapter::GetEditViewForwarder( bool bCreate )
{
    return GetEditViewForwarderAdapter( bCreate );
}

void SvxEditSourceAdapter::UpdateData()
{
    if( mbEditSourceValid && mpAdaptee.get() )
        mpAdaptee->UpdateData();
}

SfxBroadcaster& SvxEditSourceAdapter::GetBroadcaster() const
{
    if( mbEditSourceValid && mpAdaptee.get() )
        return mpAdaptee->GetBroadcaster();

    return maDummyBroadcaster;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
void SvxEditSourceAdapter::SetEditSource( ::std::auto_ptr< SvxEditSource > pAdaptee )
{
    if( pAdaptee.get() )
    {
        mpAdaptee = pAdaptee;
        mbEditSourceValid = sal_True;
    }
    else
    {
        // do a lazy delete (prevents us from deleting the broadcaster
        // from within a broadcast in
        // AccessibleTextHelper_Impl::Notify)
        mbEditSourceValid = sal_False;
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP

sal_Bool SvxEditSourceAdapter::IsValid() const
{
    return mbEditSourceValid;
}




SvxAccessibleTextAdapter::SvxAccessibleTextAdapter() : mpTextForwarder( NULL )
{
}

SvxAccessibleTextAdapter::~SvxAccessibleTextAdapter()
{
}

sal_Int32 SvxAccessibleTextAdapter::GetParagraphCount() const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetParagraphCount();
}

sal_Int32 SvxAccessibleTextAdapter::GetTextLen( sal_Int32 nParagraph ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;
    aIndex.SetEEIndex( nParagraph, mpTextForwarder->GetTextLen( nParagraph ), *this );

    return aIndex.GetIndex();
}

OUString SvxAccessibleTextAdapter::GetText( const ESelection& rSel ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    // normalize selection
    if( rSel.nStartPara > rSel.nEndPara ||
        (rSel.nStartPara == rSel.nEndPara && rSel.nStartPos > rSel.nEndPos) )
    {
        ::std::swap( aStartIndex, aEndIndex );
    }

    OUString sStr = mpTextForwarder->GetText( MakeEESelection(aStartIndex, aEndIndex) );

    // trim field text, if necessary
    if( aStartIndex.InField() )
    {
        DBG_ASSERT(aStartIndex.GetFieldOffset() >= 0 &&
                   aStartIndex.GetFieldOffset() <= USHRT_MAX,
                   "SvxAccessibleTextIndex::GetText: index value overflow");

        sStr = sStr.copy( aStartIndex.GetFieldOffset() );
    }
    if( aEndIndex.InField() && aEndIndex.GetFieldOffset() )
    {
        DBG_ASSERT(sStr.getLength() - (aEndIndex.GetFieldLen() - aEndIndex.GetFieldOffset()) >= 0 &&
                   sStr.getLength() - (aEndIndex.GetFieldLen() - aEndIndex.GetFieldOffset()) <= USHRT_MAX,
                   "SvxAccessibleTextIndex::GetText: index value overflow");

        sStr = sStr.copy(0, sStr.getLength() - (aEndIndex.GetFieldLen() - aEndIndex.GetFieldOffset()) );
    }

    EBulletInfo aBulletInfo1 = GetBulletInfo( aStartIndex.GetParagraph() );
    EBulletInfo aBulletInfo2 = GetBulletInfo( aEndIndex.GetParagraph() );

    if( aEndIndex.InBullet() )
    {
        // append trailing bullet
        sStr += aBulletInfo2.aText;

        DBG_ASSERT(sStr.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) >= 0 &&
                   sStr.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) <= USHRT_MAX,
                   "SvxAccessibleTextIndex::GetText: index value overflow");

        sStr = sStr.copy(0, sStr.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) );
    }
    else if( aStartIndex.GetParagraph() != aEndIndex.GetParagraph() &&
             HaveTextBullet( aEndIndex.GetParagraph() ) )
    {
        OUString sBullet = aBulletInfo2.aText;

        DBG_ASSERT(sBullet.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) >= 0 &&
                   sBullet.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) <= USHRT_MAX,
                   "SvxAccessibleTextIndex::GetText: index value overflow");

        sBullet = sBullet.copy(0, sBullet.getLength() - (aEndIndex.GetBulletLen() - aEndIndex.GetBulletOffset()) );

        // insert bullet
        sStr = sStr.replaceAt( GetTextLen(aStartIndex.GetParagraph()) - aStartIndex.GetIndex(), 0, sBullet );
    }

    return sStr;
}

SfxItemSet SvxAccessibleTextAdapter::GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    return mpTextForwarder->GetAttribs( MakeEESelection(aStartIndex, aEndIndex),
                                        bOnlyHardAttrib );
}

SfxItemSet SvxAccessibleTextAdapter::GetParaAttribs( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetParaAttribs( nPara );
}

void SvxAccessibleTextAdapter::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    mpTextForwarder->SetParaAttribs( nPara, rSet );
}

void SvxAccessibleTextAdapter::RemoveAttribs( const ESelection& , sal_Bool , sal_uInt16 )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");
}

void SvxAccessibleTextAdapter::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    mpTextForwarder->GetPortions( nPara, rList );
}

sal_uInt16 SvxAccessibleTextAdapter::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    return mpTextForwarder->GetItemState( MakeEESelection(aStartIndex, aEndIndex),
                                          nWhich );
}

sal_uInt16 SvxAccessibleTextAdapter::GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetItemState( nPara, nWhich );
}

void SvxAccessibleTextAdapter::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    mpTextForwarder->QuickInsertText( rText,
                                      MakeEESelection(aStartIndex, aEndIndex) );
}

void SvxAccessibleTextAdapter::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    mpTextForwarder->QuickInsertField( rFld,
                                       MakeEESelection(aStartIndex, aEndIndex) );
}

void SvxAccessibleTextAdapter::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    mpTextForwarder->QuickSetAttribs( rSet,
                                      MakeEESelection(aStartIndex, aEndIndex) );
}

void SvxAccessibleTextAdapter::QuickInsertLineBreak( const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    mpTextForwarder->QuickInsertLineBreak( MakeEESelection(aStartIndex, aEndIndex) );
}

SfxItemPool* SvxAccessibleTextAdapter::GetPool() const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetPool();
}

OUString SvxAccessibleTextAdapter::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void SvxAccessibleTextAdapter::FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    mpTextForwarder->FieldClicked( rField, nPara, nPos );
}

sal_uInt16 SvxAccessibleTextAdapter::CalcEditEngineIndex( sal_Int32 nPara, sal_Int32 nLogicalIndex )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;
    aIndex.SetIndex(nPara, nLogicalIndex, *mpTextForwarder);
    return aIndex.GetEEIndex();
}



sal_Bool SvxAccessibleTextAdapter::IsValid() const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    if( mpTextForwarder )
        return mpTextForwarder->IsValid();
    else
        return sal_False;
}

LanguageType SvxAccessibleTextAdapter::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;

    aIndex.SetIndex( nPara, nPos, *this );

    return mpTextForwarder->GetLanguage( nPara, aIndex.GetEEIndex() );
}

sal_Int32 SvxAccessibleTextAdapter::GetFieldCount( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetFieldCount( nPara );
}

EFieldInfo SvxAccessibleTextAdapter::GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetFieldInfo( nPara, nField );
}

EBulletInfo SvxAccessibleTextAdapter::GetBulletInfo( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetBulletInfo( nPara );
}

void SvxAccessibleTextAdapter::SetUpdateModeForAcc(sal_Bool bUp)
{
    return mpTextForwarder->SetUpdateModeForAcc(bUp);
}

sal_Bool SvxAccessibleTextAdapter::GetUpdateModeForAcc( ) const
{
    return mpTextForwarder->GetUpdateModeForAcc();
}

Rectangle SvxAccessibleTextAdapter::GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;
    aIndex.SetIndex( nPara, nIndex, *this );

    // preset if anything goes wrong below
    // n-th char in GetParagraphIndex's paragraph
    Rectangle aRect = mpTextForwarder->GetCharBounds( nPara, aIndex.GetEEIndex() );

    if( aIndex.InBullet() )
    {
        EBulletInfo aBulletInfo = GetBulletInfo( nPara );

        OutputDevice* pOutDev = GetRefDevice();

        DBG_ASSERT(pOutDev!=NULL, "SvxAccessibleTextAdapter::GetCharBounds: No ref device");

        // preset if anything goes wrong below
        aRect = aBulletInfo.aBounds; // better than nothing
        if( pOutDev )
        {
            AccessibleStringWrap aStringWrap( *pOutDev, aBulletInfo.aFont, aBulletInfo.aText );

            aStringWrap.GetCharacterBounds( aIndex.GetBulletOffset(), aRect );
            aRect.Move( aBulletInfo.aBounds.Left(), aBulletInfo.aBounds.Top() );
        }
    }
    else
    {
        // handle field content manually
        if( aIndex.InField() )
        {
            OutputDevice* pOutDev = GetRefDevice();

            DBG_ASSERT(pOutDev!=NULL, "SvxAccessibleTextAdapter::GetCharBounds: No ref device");

            if( pOutDev )
            {
                ESelection aSel = MakeEESelection( aIndex );

                SvxFont aFont = EditEngine::CreateSvxFontFromItemSet( mpTextForwarder->GetAttribs( aSel ) );
                AccessibleStringWrap aStringWrap( *pOutDev,
                                                  aFont,
                                                  mpTextForwarder->GetText( aSel ) );

                Rectangle aStartRect = mpTextForwarder->GetCharBounds( nPara, aIndex.GetEEIndex() );

                aStringWrap.GetCharacterBounds( aIndex.GetFieldOffset(), aRect );
                aRect.Move( aStartRect.Left(), aStartRect.Top() );
            }
        }
    }

    return aRect;
}

Rectangle SvxAccessibleTextAdapter::GetParaBounds( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    EBulletInfo aBulletInfo = GetBulletInfo( nPara );

    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType != SVX_NUM_BITMAP )
    {
        // include bullet in para bounding box
        Rectangle aRect( mpTextForwarder->GetParaBounds( nPara ) );

        aRect.Union( aBulletInfo.aBounds );

        return aRect;
    }

    return mpTextForwarder->GetParaBounds( nPara );
}

MapMode SvxAccessibleTextAdapter::GetMapMode() const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetMapMode();
}

OutputDevice* SvxAccessibleTextAdapter::GetRefDevice() const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetRefDevice();
}

sal_Bool SvxAccessibleTextAdapter::GetIndexAtPoint( const Point& rPoint, sal_Int32& nPara, sal_Int32& nIndex ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    if( !mpTextForwarder->GetIndexAtPoint( rPoint, nPara, nIndex ) )
        return sal_False;

    SvxAccessibleTextIndex aIndex;
    aIndex.SetEEIndex(nPara, nIndex, *this);

    DBG_ASSERT(aIndex.GetIndex() >= 0 && aIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");

    nIndex = aIndex.GetIndex();

    EBulletInfo aBulletInfo = GetBulletInfo( nPara );

    // any text bullets?
    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType != SVX_NUM_BITMAP )
    {
        if( aBulletInfo.aBounds.IsInside( rPoint) )
        {
            OutputDevice* pOutDev = GetRefDevice();

            DBG_ASSERT(pOutDev!=NULL, "SvxAccessibleTextAdapter::GetIndexAtPoint: No ref device");

            if( !pOutDev )
                return sal_False;

            AccessibleStringWrap aStringWrap( *pOutDev, aBulletInfo.aFont, aBulletInfo.aText );

            Point aPoint = rPoint;
            aPoint.Move( -aBulletInfo.aBounds.Left(), -aBulletInfo.aBounds.Top() );

            DBG_ASSERT(aStringWrap.GetIndexAtPoint( aPoint ) >= 0 &&
                       aStringWrap.GetIndexAtPoint( aPoint ) <= USHRT_MAX,
                       "SvxAccessibleTextIndex::SetIndex: index value overflow");

            nIndex = aStringWrap.GetIndexAtPoint( aPoint );
            return sal_True;
        }
    }

    if( aIndex.InField() )
    {
        OutputDevice* pOutDev = GetRefDevice();

        DBG_ASSERT(pOutDev!=NULL, "SvxAccessibleTextAdapter::GetIndexAtPoint: No ref device");

        if( !pOutDev )
            return sal_False;

        ESelection aSelection = MakeEESelection( aIndex );
        SvxFont aFont = EditEngine::CreateSvxFontFromItemSet( mpTextForwarder->GetAttribs( aSelection ) );
        AccessibleStringWrap aStringWrap( *pOutDev,
                                          aFont,
                                          mpTextForwarder->GetText( aSelection ) );

        Rectangle aRect = mpTextForwarder->GetCharBounds( nPara, aIndex.GetEEIndex() );
        Point aPoint = rPoint;
        aPoint.Move( -aRect.Left(), -aRect.Top() );

        DBG_ASSERT(aIndex.GetIndex() + aStringWrap.GetIndexAtPoint( rPoint ) >= 0 &&
                   aIndex.GetIndex() + aStringWrap.GetIndexAtPoint( rPoint ) <= USHRT_MAX,
                   "SvxAccessibleTextIndex::SetIndex: index value overflow");

        nIndex = (aIndex.GetIndex() + aStringWrap.GetIndexAtPoint( aPoint ));
        return sal_True;
    }

    return sal_True;
}

sal_Bool SvxAccessibleTextAdapter::GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;
    aIndex.SetIndex(nPara, nIndex, *this);
    nIndex = aIndex.GetEEIndex();

    if( aIndex.InBullet() )
    {
        DBG_ASSERT(aIndex.GetBulletLen() >= 0 &&
                   aIndex.GetBulletLen() <= USHRT_MAX,
                   "SvxAccessibleTextIndex::SetIndex: index value overflow");

        // always treat bullet as separate word
        nStart = 0;
        nEnd = aIndex.GetBulletLen();

        return sal_True;
    }

    if( aIndex.InField() )
    {
        DBG_ASSERT(aIndex.GetIndex() - aIndex.GetFieldOffset() >= 0 &&
                   aIndex.GetIndex() - aIndex.GetFieldOffset() <= USHRT_MAX &&
                   nStart + aIndex.GetFieldLen() >= 0 &&
                   nStart + aIndex.GetFieldLen() <= USHRT_MAX,
                   "SvxAccessibleTextIndex::SetIndex: index value overflow");

        // always treat field as separate word
        // TODO: to circumvent this, _we_ would have to do the break iterator stuff!
        nStart = aIndex.GetIndex() - aIndex.GetFieldOffset();
        nEnd = nStart + aIndex.GetFieldLen();

        return sal_True;
    }

    if( !mpTextForwarder->GetWordIndices( nPara, nIndex, nStart, nEnd ) )
        return sal_False;

    aIndex.SetEEIndex( nPara, nStart, *this );
    DBG_ASSERT(aIndex.GetIndex() >= 0 &&
               aIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");
    nStart = aIndex.GetIndex();

    aIndex.SetEEIndex( nPara, nEnd, *this );
    DBG_ASSERT(aIndex.GetIndex() >= 0 &&
               aIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");
    nEnd = aIndex.GetIndex();

    return sal_True;
}

sal_Bool SvxAccessibleTextAdapter::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, sal_Bool /* bInCell */ ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aIndex;
    aIndex.SetIndex(nPara, nIndex, *this);
    nIndex = aIndex.GetEEIndex();

    if( aIndex.InBullet() )
    {
        DBG_ASSERT(aIndex.GetBulletLen() >= 0 &&
                   aIndex.GetBulletLen() <= USHRT_MAX,
                   "SvxAccessibleTextIndex::SetIndex: index value overflow");

        // always treat bullet as distinct attribute
        nStartIndex = 0;
        nEndIndex = aIndex.GetBulletLen();

        return sal_True;
    }

    if( aIndex.InField() )
    {
        DBG_ASSERT(aIndex.GetIndex() - aIndex.GetFieldOffset() >= 0 &&
                   aIndex.GetIndex() - aIndex.GetFieldOffset() <= USHRT_MAX,
                   "SvxAccessibleTextIndex::SetIndex: index value overflow");

        // always treat field as distinct attribute
        nStartIndex = aIndex.GetIndex() - aIndex.GetFieldOffset();
        nEndIndex = nStartIndex + aIndex.GetFieldLen();

        return sal_True;
    }

    if( !mpTextForwarder->GetAttributeRun( nStartIndex, nEndIndex, nPara, nIndex ) )
        return sal_False;

    aIndex.SetEEIndex( nPara, nStartIndex, *this );
    DBG_ASSERT(aIndex.GetIndex() >= 0 &&
               aIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");
    nStartIndex = aIndex.GetIndex();

    aIndex.SetEEIndex( nPara, nEndIndex, *this );
    DBG_ASSERT(aIndex.GetIndex() >= 0 &&
               aIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextIndex::SetIndex: index value overflow");
    nEndIndex = aIndex.GetIndex();

    return sal_True;
}

sal_Int32 SvxAccessibleTextAdapter::GetLineCount( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetLineCount( nPara );
}

sal_Int32 SvxAccessibleTextAdapter::GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;
    sal_Int32 nCurrLine;
    sal_Int32 nCurrIndex, nLastIndex;
    for( nCurrLine=0, nCurrIndex=0, nLastIndex=0; nCurrLine<=nLine; ++nCurrLine )
    {
        nLastIndex = nCurrIndex;
        nCurrIndex =
            nCurrIndex + mpTextForwarder->GetLineLen( nPara, nCurrLine );
    }

    aEndIndex.SetEEIndex( nPara, nCurrIndex, *this );
    if( nLine > 0 )
    {
        aStartIndex.SetEEIndex( nPara, nLastIndex, *this );

        return aEndIndex.GetIndex() - aStartIndex.GetIndex();
    }
    else
        return aEndIndex.GetIndex();
}

void SvxAccessibleTextAdapter::GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    mpTextForwarder->GetLineBoundaries( rStart, rEnd, nParagraph, nLine );
}

sal_Int32 SvxAccessibleTextAdapter::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return mpTextForwarder->GetLineNumberAtIndex( nPara, nIndex );
}

sal_Bool SvxAccessibleTextAdapter::Delete( const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    return mpTextForwarder->Delete( MakeEESelection(aStartIndex, aEndIndex ) );
}

sal_Bool SvxAccessibleTextAdapter::InsertText( const OUString& rStr, const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    return mpTextForwarder->InsertText( rStr, MakeEESelection(aStartIndex, aEndIndex) );
}

sal_Bool SvxAccessibleTextAdapter::QuickFormatDoc( sal_Bool bFull )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->QuickFormatDoc( bFull );
}

sal_Int16 SvxAccessibleTextAdapter::GetDepth( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->GetDepth( nPara );
}

sal_Bool SvxAccessibleTextAdapter::SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    return mpTextForwarder->SetDepth( nPara, nNewDepth );
}

void SvxAccessibleTextAdapter::SetForwarder( SvxTextForwarder& rForwarder )
{
    mpTextForwarder = &rForwarder;
}

sal_Bool SvxAccessibleTextAdapter::HaveImageBullet( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    EBulletInfo aBulletInfo = GetBulletInfo( nPara );

    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType == SVX_NUM_BITMAP )
    {
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

sal_Bool SvxAccessibleTextAdapter::HaveTextBullet( sal_Int32 nPara ) const
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    EBulletInfo aBulletInfo = GetBulletInfo( nPara );

    if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
        aBulletInfo.bVisible &&
        aBulletInfo.nType != SVX_NUM_BITMAP )
    {
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

sal_Bool SvxAccessibleTextAdapter::IsEditable( const ESelection& rSel )
{
    DBG_ASSERT(mpTextForwarder, "SvxAccessibleTextAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *this );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *this );

    // normalize selection
    if( rSel.nStartPara > rSel.nEndPara ||
        (rSel.nStartPara == rSel.nEndPara && rSel.nStartPos > rSel.nEndPos) )
    {
        ::std::swap( aStartIndex, aEndIndex );
    }

    return aStartIndex.IsEditableRange( aEndIndex );
}

const SfxItemSet * SvxAccessibleTextAdapter::GetEmptyItemSetPtr()
{
    OSL_FAIL( "not implemented" );
    return 0;
}

void SvxAccessibleTextAdapter::AppendParagraph()
{
    OSL_FAIL( "not implemented" );
}

sal_Int32 SvxAccessibleTextAdapter::AppendTextPortion( sal_Int32, const OUString &, const SfxItemSet & )
{
    OSL_FAIL( "not implemented" );
    return 0;
}
void        SvxAccessibleTextAdapter::CopyText(const SvxTextForwarder&)
{
    OSL_FAIL( "not implemented" );
}





SvxAccessibleTextEditViewAdapter::SvxAccessibleTextEditViewAdapter()
    : mpViewForwarder(NULL)
    , mpTextForwarder(NULL)
{
}

SvxAccessibleTextEditViewAdapter::~SvxAccessibleTextEditViewAdapter()
{
}

sal_Bool SvxAccessibleTextEditViewAdapter::IsValid() const
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    if( mpViewForwarder )
        return mpViewForwarder->IsValid();
    else
        return sal_False;
}

Rectangle SvxAccessibleTextEditViewAdapter::GetVisArea() const
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->GetVisArea();
}

Point SvxAccessibleTextEditViewAdapter::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->LogicToPixel(rPoint, rMapMode);
}

Point SvxAccessibleTextEditViewAdapter::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->PixelToLogic(rPoint, rMapMode);
}

sal_Bool SvxAccessibleTextEditViewAdapter::GetSelection( ESelection& rSel ) const
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    ESelection aSelection;

    if( !mpViewForwarder->GetSelection( aSelection ) )
        return sal_False;

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetEEIndex( aSelection.nStartPara, aSelection.nStartPos, *mpTextForwarder );
    aEndIndex.SetEEIndex( aSelection.nEndPara, aSelection.nEndPos, *mpTextForwarder );

    DBG_ASSERT(aStartIndex.GetIndex() >= 0 && aStartIndex.GetIndex() <= USHRT_MAX &&
               aEndIndex.GetIndex() >= 0 && aEndIndex.GetIndex() <= USHRT_MAX,
               "SvxAccessibleTextEditViewAdapter::GetSelection: index value overflow");

    rSel = ESelection( aStartIndex.GetParagraph(), aStartIndex.GetIndex(),
                       aEndIndex.GetParagraph(), aEndIndex.GetIndex() );

    return sal_True;
}

sal_Bool SvxAccessibleTextEditViewAdapter::SetSelection( const ESelection& rSel )
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    SvxAccessibleTextIndex aStartIndex;
    SvxAccessibleTextIndex aEndIndex;

    aStartIndex.SetIndex( rSel.nStartPara, rSel.nStartPos, *mpTextForwarder );
    aEndIndex.SetIndex( rSel.nEndPara, rSel.nEndPos, *mpTextForwarder );

    return mpViewForwarder->SetSelection( MakeEESelection(aStartIndex, aEndIndex) );
}

sal_Bool SvxAccessibleTextEditViewAdapter::Copy()
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->Copy();
}

sal_Bool SvxAccessibleTextEditViewAdapter::Cut()
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->Cut();
}

sal_Bool SvxAccessibleTextEditViewAdapter::Paste()
{
    DBG_ASSERT(mpViewForwarder, "SvxAccessibleTextEditViewAdapter: no forwarder");

    return mpViewForwarder->Paste();
}

void SvxAccessibleTextEditViewAdapter::SetForwarder( SvxEditViewForwarder&      rForwarder,
                                                     SvxAccessibleTextAdapter&  rTextForwarder )
{
    mpViewForwarder = &rForwarder;
    mpTextForwarder = &rTextForwarder;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
