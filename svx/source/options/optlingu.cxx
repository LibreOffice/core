/*************************************************************************
 *
 *  $RCSfile: optlingu.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-19 11:31:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
// include ---------------------------------------------------------------

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYLISTEVENTFLAGS_HPP_
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYLISTEVENT_HPP_
#include <com/sun/star/linguistic2/DictionaryListEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLISTEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XOTHERLINGU_HPP_
#include <com/sun/star/linguistic2/XOtherLingu.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include <comphelper/processfactory.hxx>

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#include <sfx2/viewfrm.hxx>

#define _SVX_OPTLINGU_CXX

#include "optlingu.hrc"

#define ITEMID_SPELLCHECK   0
#define ITEMID_HYPHENREGION 0

#include "optdict.hxx"
#include "optitems.hxx"
#include "optlingu.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "helpid.hrc"

#include <ucbhelper/content.hxx>

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define SVX_MAX_USERDICTS 20
// static ----------------------------------------------------------------

static const sal_uInt16 nNameLen = 8;

static sal_uInt16 pRanges[] =
{
    SID_ATTR_SPELL,
    SID_ATTR_SPELL,
    0
};

sal_Bool KillFile_Impl( const String& rURL )
{
    sal_Bool bRet = sal_True;
    try
    {
        Content aCnt( rURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "KillFile: CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "KillFile: Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}

/*-----------------02.06.97 07:38-------------------
    Ext. Linguistik auswaehlen
--------------------------------------------------*/

// class SvxLinguTabPage -------------------------------------------------

SvxLinguTabPage::SvxLinguTabPage( Window* pParent,
                                  const SfxItemSet& rSet ):

    SfxTabPage( pParent, ResId( RID_SFXPAGE_LINGU, DIALOG_MGR() ), rSet ),

    aCapsBtn            ( this, ResId( BTN_CAPS ) ),
    aNumsBtn            ( this, ResId( BTN_NUMS ) ),
    aUpLowBtn           ( this, ResId( BTN_UPLOW ) ),
    aSpellSpecialBtn    ( this, ResId( BTN_SPELL_SPECIAL ) ),
    aAllLangBtn         ( this, ResId( BTN_ALL_LANG ) ),
    aAuditBox           ( this, ResId( GB_AUDIT ) ),
    aDictsLB            ( this, ResId( LB_DICTS ) ),
    aDictsTbx           ( this, ResId( TB_DICTS ) ),
    aDictsBox           ( this, ResId( GB_DICTS ) ),
    aAutoCheckBtn       ( this, ResId( BTN_AUTO_SPELLING ) ),
    aMarkOffBtn         ( this, ResId( BTN_MARK_OFF ) ),
    aAutoSpellBox       ( this, ResId( GB_AUTO_SPELLING ) ),
    aGermanSpellBox     ( this, ResId( GB_GERMAN_SPELLING ) ),
    aGermanPreReformBtn ( this, ResId( BTN_GERMAN_PRE_REFORM) ),
    aPreBreakText       ( this, ResId( FT_PREBREAK ) ),
    aPreBreakEdit       ( this, ResId( ED_PREBREAK ) ),
    aAfterBreakText     ( this, ResId( FT_AFTERBREAK ) ),
    aAfterBreakEdit     ( this, ResId( ED_AFTERBREAK ) ),
    aAutoBtn            ( this, ResId( BTN_AUTO ) ),
    aSpecialBtn         ( this, ResId( BTN_SPECIAL ) ),
    aRulesBox           ( this, ResId( GB_RULES ) ),
    aLanguageText       ( this, ResId( FT_LANGUAGE ) ),
    aLanguageLB         ( this, ResId( LB_LANGUAGE ) ),
    aMaxWarning(ResId(STR_WARNING))

{
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
        aDics = xDicList->getDictionaries();
    aDictsTbx.SetSelectHdl( LINK( this, SvxLinguTabPage, BoxSelectHdl_Impl ) );
    Size aSiz = aDictsTbx.CalcWindowSizePixel();
    aDictsTbx.SetSizePixel( aSiz );

    const SfxSpellCheckItem* pItem = 0;
    SfxItemState eItemState = SFX_ITEM_UNKNOWN;

    eItemState = rSet.GetItemState( GetWhich( SID_ATTR_SPELL ),
                                    sal_False, (const SfxPoolItem**)&pItem );

    // handelt es sich um ein Default-Item?
    if ( eItemState == SFX_ITEM_DEFAULT )
        pItem = (const SfxSpellCheckItem*)&(rSet.Get( GetWhich( SID_ATTR_SPELL ) ) );
    else if ( eItemState == SFX_ITEM_DONTCARE )
        pItem = NULL;


    // fill box for DefaultLanguage (?) with possible languages
    const sal_uInt16 nLangCnt = SvxGetSelectableLanguages().getLength();
    const util::Language *pLang = SvxGetSelectableLanguages().getConstArray();
    // start with 1 to skip LANGUAGE_NONE
    for ( sal_uInt16 i = 1; i < nLangCnt; i++ )
    {
        sal_uInt16 nPos = aLanguageLB.InsertEntry( GetLanguageString( pLang[i] ) );
        aLanguageLB.SetEntryData( nPos, (void*)(sal_uInt32)i );
    }

    // disable button if IgnoreAllList has no elements

    aAutoCheckBtn.SetClickHdl( LINK( this, SvxLinguTabPage, CheckHdl_Impl ) );
    aDictsLB.SetCheckButtonHdl( LINK( this, SvxLinguTabPage, CheckDicHdl_Impl));
    aDictsLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectDicHdl_Impl));

    // disable controls that can't be used due to missing services
    if (!SvxGetLinguPropertySet().is())
        Enable( sal_False );    // disable everything
    else if (!xDicList.is())
    {   // disable dictionary and dictionary list controls
        aDictsLB .Enable( sal_False );
        aDictsTbx.Enable( sal_False );
        aDictsBox.Enable( sal_False );
    }

    FreeResource();
}

// -----------------------------------------------------------------------

SvxLinguTabPage::~SvxLinguTabPage()
{
}

//------------------------------------------------------------------------

sal_uInt16* SvxLinguTabPage::GetRanges()
{
    return pRanges;
}

//------------------------------------------------------------------------

SfxTabPage* SvxLinguTabPage::Create( Window* pParent,
                                     const SfxItemSet& rAttrSet )
{
    return ( new SvxLinguTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------
Any lcl_Bool2Any(BOOL bVal)
{
    Any aRet(&bVal, ::getBooleanCppuType());
    return aRet;
}
sal_Bool lcl_Bool2Any(Any& rVal)
{
    return *(sal_Bool*)rVal.getValue();
}
sal_Bool SvxLinguTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    sal_Bool bModified = sal_True; // !!!!

    if (!SvxGetLinguPropertySet().is())
        return sal_False;

    //
    // build list of active dictionaries
    // (There has to be at least one active positive dictionary
    // this usually is the "standard" dictionary. It will be created
    // if it does not already exist.)
    //

    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    const sal_uInt16 nCount = aDics.getLength();
    const sal_uInt16 nSize  = (sal_uInt16)aDictsLB.GetEntryCount();
    //
    sal_Bool bIsAnyPosDicActive = sal_False;
    for (sal_uInt16 j = 0;  j < nCount;  j++)
    {
        Reference< XDictionary1 >  xDic( pDic[j], UNO_QUERY );
        if (!xDic.is())
            continue;

        sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xDic->getName(),
                                             xDic->getLanguage(),
                                             bNegativ ) );

        sal_Bool bActivate = sal_False;
        for ( sal_uInt16 i = 0; i < nSize; ++i )
        {
            // Dictionary in der awt::Selection -> Aktivieren
            if ( aDictsLB.IsChecked(i) && aName == aDictsLB.GetText(i) )
            {
                // any dictionary is persistent not readonly and may hold
                // positive entries?
                Reference< frame::XStorable >  xStor( xDic, UNO_QUERY );
                if (xStor.is() && xStor->hasLocation() && !xStor->isReadonly())
                    bIsAnyPosDicActive =
                            xDic->getDictionaryType() != DictionaryType_NEGATIVE;

                bActivate = sal_True;
                break;
            }
        }
        if (xDic != SvxGetIgnoreAllList())
            xDic->setActive( bActivate );
    }

    // force new spelling and flushing of spell caches
    //! current implementation is a workaround until the correct
    //! interface is implemented.
    //TL:TODO: use XPropertyChangeListener mechanism to do this
    Reference< XDictionary1 >  xDic( SvxGetIgnoreAllList() );
    if (xDic.is())
    {
        OUString aTmp( C2U("v_7xj4") );
        sal_Bool bOk = xDic->add( aTmp, sal_False, OUString() );
        if (bOk)
            xDic->remove( aTmp );
    }
#ifdef NOT_YET_IMPLEMENTED
    //!!! functionality needs to be implemented via XPropertySet listeners!

    sal_Bool bSpellAllAgain   = sal_False;
    sal_Bool bSpellWrongAgain = sal_False;
    if( aNumsBtn.IsChecked() != aNumsBtn.GetSavedValue() )
    {
        if( aNumsBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aCapsBtn.IsChecked() != aCapsBtn.GetSavedValue() )
    {
        if( aCapsBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aUpLowBtn.IsChecked() != aUpLowBtn.GetSavedValue() )
    {
        if( aUpLowBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aAllLangBtn.IsChecked() != aAllLangBtn.GetSavedValue() )
    {
        if( aAllLangBtn.IsChecked() )
            bSpellWrongAgain = sal_True;
        else
            bSpellAllAgain = sal_True;
    }
    pSpell->SetSpellWrongAgain( bSpellWrongAgain );
    pSpell->SetSpellAllAgain( bSpellAllAgain );
#endif //NOT_YET_IMPLEMENTED

    Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        // set spellchecking properties
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_WITH_DIGITS), lcl_Bool2Any(aNumsBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_UPPER_CASE), lcl_Bool2Any(aCapsBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_CAPITALIZATION), lcl_Bool2Any(aUpLowBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES), lcl_Bool2Any(aAllLangBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_SPECIAL), lcl_Bool2Any(aSpellSpecialBtn.IsChecked()) );

        // set hyphenation properties
        xProp->setPropertyValue( C2U(UPN_IS_HYPH_AUTO), lcl_Bool2Any(aAutoBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_HYPH_SPECIAL), lcl_Bool2Any(aSpecialBtn.IsChecked()) );

        xProp->setPropertyValue( C2U(UPN_IS_GERMAN_PRE_REFORM), lcl_Bool2Any(aGermanPreReformBtn.IsChecked()) );
    }

    // erstmal immer putten!
    rCoreSet.Put( SfxBoolItem( SID_SPELL_MODIFIED, bModified ) );

    const String    &rPreStr    = aPreBreakEdit.GetText(),
                    &rAfterStr  = aAfterBreakEdit.GetText();

    const String    &rSavedPreStr   = aPreBreakEdit.GetSavedValue(),
                    &rSavedAfterStr = aAfterBreakEdit.GetSavedValue();

    if ( rPreStr != rSavedPreStr || rAfterStr != rSavedAfterStr )
    {
        SfxHyphenRegionItem aHyp( GetWhich( SID_ATTR_HYPHENREGION ) );
        aHyp.GetMinLead() =
            (sal_uInt8)aPreBreakEdit.Denormalize( aPreBreakEdit.GetValue() );
        aHyp.GetMinTrail() =
            (sal_uInt8)aAfterBreakEdit.Denormalize( aAfterBreakEdit.GetValue() );
        rCoreSet.Put( aHyp );
    }

    // language
    sal_uInt16 nPos = aLanguageLB.GetSelectEntryPos();
    sal_uInt16 nLang = (sal_uInt16)(sal_uInt32)aLanguageLB.GetEntryData( nPos );

    if ( nPos != aLanguageLB.GetSavedValue() &&
         nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        SfxUInt16Item aFmtLang( GetWhich( SID_ATTR_LANGUAGE ),
                                SvxGetSelectableLanguages().getConstArray()[ nLang ] );
        rCoreSet.Put( aFmtLang );
    }


    // Autom. Rechtschreibung
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, SID_AUTOSPELL_CHECK );

    if ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() !=
                    aAutoCheckBtn.IsChecked() )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_CHECK ),
                                   aAutoCheckBtn.IsChecked() ) );
        bModified |= sal_True;
    }
    pOld = GetItem( rCoreSet, SID_AUTOSPELL_MARKOFF );

    if ( aMarkOffBtn.IsEnabled() &&
         ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() !=
                      aMarkOffBtn.IsChecked() ) )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_MARKOFF ),
                                   aMarkOffBtn.IsChecked() ) );
        bModified |= sal_True;
    }

    return bModified;
}

//------------------------------------------------------------------------

void SvxLinguTabPage::Reset( const SfxItemSet& rSet )
{
    UpdateBox_Impl();

    // #44483#
    // form::component::CheckBox f"ur die Benutzerw"orterb"ucher
    // anhand der Vorgaben des SpellChecker setzen
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

    const sal_uInt16 nCount = aDics.getLength();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    const sal_uInt16 nSize  = (sal_uInt16)aDictsLB.GetEntryCount();
    //
    for ( sal_uInt16 i = 0; i < nSize; ++i )
    {
        for ( sal_uInt16 j = 0; j < nCount; ++j )
        {
            Reference< XDictionary1 >  xDic( pDic[j], UNO_QUERY );
            if (!xDic.is())
                continue;

            sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
            const String aName( ::GetDicInfoStr( xDic->getName(),
                                                 xDic->getLanguage(),
                                                 bNegativ ) );

            if ( xDic->isActive()  &&  aName == aDictsLB.GetText(i) )
                break;
        }
        aDictsLB.CheckEntryPos( i, j != nCount );
    }
    // #44483#

    Reference< XPropertySet >  xProp ( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        // Optionen aus dem Speller einstellen
        aCapsBtn        .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_UPPER_CASE) ).getValue() );
        aNumsBtn        .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_WITH_DIGITS) ).getValue() );
        aUpLowBtn       .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_CAPITALIZATION) ).getValue() );
        aAllLangBtn     .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES) ).getValue() );
        aSpellSpecialBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_SPECIAL) ).getValue() );

        aAutoBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_AUTO) ).getValue() );
        aSpecialBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_SPECIAL) ).getValue() );

        aGermanPreReformBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_GERMAN_PRE_REFORM) ).getValue() );
    }

    sal_uInt16 nWhich = GetWhich( SID_ATTR_HYPHENREGION );

    if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxHyphenRegionItem& rHyp =
            (const SfxHyphenRegionItem&)rSet.Get( nWhich );
        aPreBreakEdit.SetValue(
            aPreBreakEdit.Normalize( rHyp.GetMinLead() ) );
        aAfterBreakEdit.SetValue(
            aAfterBreakEdit.Normalize( rHyp.GetMinTrail() ) );
    }

    // Sprache
    nWhich = GetWhich( SID_ATTR_LANGUAGE );
    aLanguageLB.SetNoSelection();

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rSet.Get( nWhich );
        DBG_ASSERT( (LanguageType)rItem.GetValue() != LANGUAGE_SYSTEM,
                    "LANGUAGE_SYSTEM nicht erlaubt!" );

        if ( (LanguageType)rItem.GetValue() != LANGUAGE_DONTKNOW )
        {
            sal_uInt16 nLang = SvxGetLanguagePos( SvxGetSelectableLanguages(),
                                              rItem.GetValue() );

            for ( sal_uInt16 i = 0; i < aLanguageLB.GetEntryCount(); ++i )
            {
                if ( (sal_uInt16)(sal_uInt32)aLanguageLB.GetEntryData(i) == nLang )
                {
                    aLanguageLB.SelectEntryPos(i);
                    break;
                }
            }
        }
    }

    // Autom. Rechtschreibung
    const SfxPoolItem* pItem = GetItem( rSet, SID_AUTOSPELL_CHECK );

    if ( pItem )
        aAutoCheckBtn.Check( ( (SfxBoolItem*)pItem )->GetValue() );
    pItem = GetItem( rSet, SID_AUTOSPELL_MARKOFF );

    if ( pItem )
        aMarkOffBtn.Check( ( (SfxBoolItem*)pItem )->GetValue() );

    CheckHdl_Impl( 0 );

    aNumsBtn.SaveValue();
    aCapsBtn.SaveValue();
    aUpLowBtn.SaveValue();
    aAllLangBtn.SaveValue();
    aPreBreakEdit.SaveValue();
    aAfterBreakEdit.SaveValue();
    aLanguageLB.SaveValue();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, NewHdl_Impl, Button *, EMPTYARG )
{
    SvxNewDictionaryDialog* pDlg = new SvxNewDictionaryDialog( this, xSpell );

    Reference< XDictionary1 >  xNewDic;

    if ( pDlg->Execute() == RET_OK )
        xNewDic = pDlg->GetNewDictionary();
    delete pDlg;

    if ( xNewDic.is() )
    {
        // add new Dictionary to snapshot list
        sal_uInt16 nNewIdx = aDics.getLength();
        aDics.realloc( nNewIdx + 1 );
        aDics.getArray()[ nNewIdx ] = Reference< XDictionary > ( xNewDic, UNO_QUERY );
        const Reference< XDictionary >  *pDic = aDics.getConstArray();

        sal_Bool bNegativ = xNewDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xNewDic->getName(),
                                             xNewDic->getLanguage(),
                                             bNegativ ) );
        aDictsLB.InsertEntry( aName, nNewIdx );
        if ( aDictsLB.GetCheckedEntryCount() < SVX_MAX_USERDICTS )
            aDictsLB.CheckEntryPos( nNewIdx );
        aDictsLB.SelectEntryPos( nNewIdx );
        aDictsLB.MakeVisible( aDictsLB.GetEntry( nNewIdx ) );
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, EditHdl_Impl, Button *, EMPTYARG )
{
    String sName;
    sal_uInt16 nId = aDictsLB.GetSelectEntryPos();

    if ( nId != LISTBOX_ENTRY_NOTFOUND )
        sName = aDictsLB.GetText( nId );

    SvxEditDictionaryDialog* pDlg =
        new SvxEditDictionaryDialog( this, sName, xSpell );
    pDlg->Execute();

    UpdateBox_Impl();

    aDictsLB.MakeVisible(aDictsLB.GetEntry(pDlg->GetSelectedDict()));
    aDictsLB.SelectEntryPos(pDlg->GetSelectedDict());

    delete pDlg;
    return 1;
}

// -----------------------------------------------------------------------
void lcl_SequenceRemoveElementAt(
    Sequence<Reference<XDictionary > >& rEntries, int nPos )
{
    //TODO: helper for SequenceRemoveElementAt available?
    if(nPos >= rEntries.getLength())
        return;
    Sequence<Reference<XDictionary> > aTmp(rEntries.getLength() - 1);
    Reference<XDictionary >* pOrig = rEntries.getArray();
    Reference<XDictionary >* pTemp = aTmp.getArray();
    int nOffset = 0;
    for(int i = 0; i < aTmp.getLength(); i++)
    {
        if(nPos == i)
            nOffset++;
        pTemp[i] = pOrig[i + nOffset];
    }

    rEntries = aTmp;
}

IMPL_LINK( SvxLinguTabPage, DeleteHdl_Impl, Button *, EMPTYARG )
{
    if ( RET_NO ==
         QueryBox( this, ResId( RID_SFXQB_DELDICT, DIALOG_MGR() ) ).Execute() )
        return 0;

    //Eintrag aus der Listbox loeschen und am SpellCheck abmelden
    sal_uInt16 nId = aDictsLB.GetSelectEntryPos();

    if ( nId != LISTBOX_ENTRY_NOTFOUND )
    {
        String aDicName( aDics.getConstArray()[ nId ]->getName() );

        Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
        Reference< XDictionary >  xDic;
        if (xDicList.is())
        {
            xDic = xDicList->getDictionaryByName( aDicName);
        }

        if (xDic.is())
        {
            if (xDic == SvxGetIgnoreAllList())
            {
                xDic->clear();
            }
            else
            {
                xDicList->removeDictionary( xDic );

                // remove Dictionary from snapshot list
                lcl_SequenceRemoveElementAt( aDics, nId);

                Reference< frame::XStorable > xStor( xDic, UNO_QUERY );
                if ( xStor->hasLocation() && !xStor->isReadonly() )
                {
                    String sURL = xStor->getLocation();
                    INetURLObject aObj(sURL);
                    DBG_ASSERT( aObj.GetProtocol() == INET_PROT_FILE, "non-file URLs cannot be deleted" );
                    if ( aObj.GetProtocol() == INET_PROT_FILE )
                    {
                        KillFile_Impl( aObj.GetMainURL() );
                    }

//                  CntAnchorRef xAnchor =
//                          new CntAnchor( NULL, U2S( xStor->getLocation() ) );
//                  xAnchor->Put( SfxBoolItem(WID_DELETE, sal_False) );
                }
            }
        }


        if (xDic.is()  &&  xDic != SvxGetIgnoreAllList())
            aDictsLB.RemoveEntry( nId );
    }

    if ( aDictsLB.GetEntryCount() > 0 )
    {
        sal_uInt16 nNewId = Min( nId, (sal_uInt16)( aDictsLB.GetEntryCount() - 1 ) );
        aDictsLB.SelectEntryPos( nNewId );
    }
    else
    {
        aDictsTbx.EnableItem( TID_EDIT, sal_False );
        aDictsTbx.EnableItem( TID_DELETE, sal_False );
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxLinguTabPage, EraseHdl_Impl, Button *, EMPTYARG )
{
    if ( RET_YES ==
         QueryBox( this, SVX_RES( RID_SFXQB_DEL_IGNORELIST ) ).Execute() )
    {
        if (SvxGetIgnoreAllList().is())
            SvxGetIgnoreAllList()->clear();
    }
    return 1;
}
IMPL_LINK_INLINE_END( SvxLinguTabPage, EraseHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, CheckHdl_Impl, CheckBox *, EMPTYARG )
{
    if ( !aAutoCheckBtn.IsChecked() )
        aMarkOffBtn.Disable();
    else
        aMarkOffBtn.Enable();
    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvxLinguTabPage, CheckDicHdl_Impl, SvxCheckListBox *, EMPTYARG )
{
    sal_uInt16 nMax=0;
    sal_Bool bFlag=sal_False;

    if(aDictsLB.GetCheckedEntryCount()>SVX_MAX_USERDICTS)
    {
        InfoBox aInfoBox(this,aMaxWarning);
        aInfoBox.Execute();

        SvLBoxEntry* pEntry=aDictsLB.GetCurEntry();
        SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));

        if(pItem!=NULL)
        {
            pItem->SetStateHilighted(sal_False);

        }

        aDictsLB.EndSelection();
        aDictsLB.CheckEntryPos( aDictsLB.GetSelectEntryPos(),sal_False);
    }

    // let CheckBox for IgnoreAllList always be checked
    sal_uInt16 nSel = aDictsLB.GetSelectEntryPos();
    if (nSel != LISTBOX_ENTRY_NOTFOUND
        &&  aDictsLB.GetEntryData( nSel ) == (void *) 1)
    {
        aDictsLB.CheckEntryPos( nSel, sal_True );
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvxLinguTabPage, SelectDicHdl_Impl, SvxCheckListBox *, pBox )
{
    DBG_ASSERT( pBox == &aDictsLB, "svx : unexpected argument" );

    BOOL bEnableDelete = TRUE;

    sal_uInt16 nId = aDictsLB.GetSelectEntryPos();
    if ( nId != LISTBOX_ENTRY_NOTFOUND )
    {
        Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
        if (xDicList.is())
        {
            String aDicName( aDics.getConstArray()[ nId ]->getName() );

            Reference< frame::XStorable >  xStor(
                    xDicList->getDictionaryByName( aDicName ), UNO_QUERY );
            if (xStor.is() && xStor->isReadonly())
                bEnableDelete = FALSE;
        }
    }

    aDictsTbx.EnableItem( TID_DELETE, bEnableDelete);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, BoxSelectHdl_Impl, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();

    if ( TID_NEW == nEntry )
        NewHdl_Impl( 0 );
    else if ( TID_EDIT == nEntry )
        EditHdl_Impl( 0 );
    else if ( TID_DELETE == nEntry )
        DeleteHdl_Impl( 0 );
    return 1;
}
// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateBox_Impl()
{
    // gecheckte Eintr"age merken
    SvUShorts aArr;

    sal_uInt16 i;
    sal_uInt16 nOldEntryCount = (sal_uInt16)aDictsLB.GetEntryCount();
    for ( i = 0; i < nOldEntryCount; ++i )
        if ( aDictsLB.IsChecked(i) )
            aArr.Insert( i, aArr.Count() );

    const sal_Int32 nCount = aDics.getLength();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    aDictsLB.Clear();
    for ( i = 0; i < nCount; ++i )
    {
        Reference< XDictionary1 >  xDic( pDic[i], UNO_QUERY );
        if (!xDic.is())
            continue;

        sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xDic->getName(),
                                             xDic->getLanguage(),
                                             bNegativ ) );
        aDictsLB.InsertEntry( aName );

        // Neue Eintr"age evtl. checken
        if ( nOldEntryCount && i >= nOldEntryCount
            && xDic->isActive())
            aDictsLB.CheckEntryPos( (sal_uInt16)aDictsLB.GetEntryCount() - 1  );

        // mark entry for IgnoreAllList as not being allowed to uncheck
        if (xDic == SvxGetIgnoreAllList())
        {
            aDictsLB.SetEntryData( aDictsLB.GetEntryCount() - 1, (void *) 1 );
        }
    }

    // gemerkte Eintr"age wieder checken
    for ( i = 0; i < aArr.Count(); ++i )
        aDictsLB.CheckEntryPos( aArr[i] );

    if ( aDictsLB.GetEntryCount() )
    {
        aDictsTbx.EnableItem( TID_EDIT, sal_True );
        aDictsTbx.EnableItem( TID_DELETE, sal_True );
    }
    else
    {
        aDictsTbx.EnableItem( TID_EDIT, sal_False );
        aDictsTbx.EnableItem( TID_DELETE, sal_False );
    }

    aDictsLB.SetHighlightRange();
    aDictsLB.SelectEntryPos( 0 );   //! may implicitly change TID_DELETE
}

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
    if ( 0 != ( GROUP_SPELLING & nGrp ) )
    {
        aCapsBtn.Hide();
        aNumsBtn.Hide();
        aUpLowBtn.Hide();
        aAllLangBtn.Hide();
        aAuditBox.Hide();
    }

    if ( 0 != ( GROUP_HYPHEN & nGrp ) )
    {
        aPreBreakText.Hide();
        aPreBreakEdit.Hide();
        aAfterBreakText.Hide();
        aAfterBreakEdit.Hide();
        aAutoBtn.Hide();
        aSpecialBtn.Hide();
        aRulesBox.Hide();
    }

    if ( 0 != ( GROUP_LANGUAGE & nGrp ) )
    {
        aLanguageText.Hide();
        aLanguageLB.Hide();
    }
}

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideSpellCheckSpecial()
{
    aSpellSpecialBtn.Hide();
}

/* -----------------------------27.01.00 12:14--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguTabPage::EnableAutoSpelling()
{
    aAutoCheckBtn.Show();
    aMarkOffBtn.Show();
    aAutoSpellBox.Show();
}

/*-- 27.01.00 12:17:13---------------------------------------------------

  -----------------------------------------------------------------------*/

SvxExternalLinguTabPage::SvxExternalLinguTabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, ResId( RID_SVXPAGE_EXTERNAL_LINGU, DIALOG_MGR() ), rSet),
    aOptionsPB( this, ResId(PB_OPTIONS)),
    aLinguGB(   this, ResId(GB_LINGU    )),
    aLinguLB(   this, ResId(LB_LINGU    )),
    aStdSpellCB(this, ResId(CB_STDSPELL)),
    aStdThesCB( this, ResId(CB_STDTHES)),
    aStdHyphCB( this, ResId(CB_STDHYPH)),
    aSettingsGB(this, ResId(GB_SETTINGS))
{
    FreeResource();
    Reference< lang::XMultiServiceFactory >  xMgr = ::comphelper::getProcessServiceFactory();
    Reference< XInterface >  xInst = xMgr->createInstance(
            C2U("com.sun.star.linguistic2.OtherLingu") );
    xOtherLingu = Reference< XOtherLingu > (xInst, UNO_QUERY);
    if(xOtherLingu.is())
    {
        sal_Int16 nLinguCount = xOtherLingu->getCount();
        for(sal_uInt16 i = 0; i < nLinguCount; i++)
        {
            aLinguLB.InsertEntry(xOtherLingu->getIdentifier(i));
        }
        aLinguLB.SelectEntryPos(0);

        aLinguLB.SetSelectHdl(LINK(this, SvxExternalLinguTabPage, LBSelectHdl_Impl));
        aOptionsPB.SetClickHdl(LINK(this, SvxExternalLinguTabPage, OptDlgHdl_Impl));

        xInst = xMgr->createInstance(
                C2U("com.sun.star.linguistic2.LinguProperties") );
        xLinguProps = Reference< XPropertySet > (xInst, UNO_QUERY);
        if(xLinguProps.is())
        {
            Any aIndex = xLinguProps->getPropertyValue(C2U("OtherLinguIndex"));
            sal_Int16 nIndex;
            aIndex >>= nIndex;;
            if(nIndex >= 0 && nIndex < nLinguCount)
            {
                aLinguLB.SelectEntryPos(nIndex + 1);

                Any aStdSpell = xLinguProps->getPropertyValue(C2U("IsStandardSpellChecker"));
                aStdSpellCB.Check(lcl_Bool2Any(aStdSpell));

                Any aStdThes = xLinguProps->getPropertyValue(C2U("IsStandardThesaurus"));
                aStdThesCB.Check(lcl_Bool2Any(aStdThes));

                Any aStdHyph = xLinguProps->getPropertyValue(C2U("IsStandardHyphenator"));
                aStdHyphCB.Check(lcl_Bool2Any(aStdHyph));
            }

        }
        LBSelectHdl_Impl(&aLinguLB);
/*
    virtual sal_Int16           getCount() throw( RuntimeException );
    virtual ::rtl::OUString         getIdentifier(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual sal_Bool            hasOptionDialog(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual sal_Int32           startOptionDialog(sal_Int16 nIndex, sal_Int32 pParent) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XHyphenator >    getHyphenator(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XSpellChecker >      getSpellChecker(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XThesaurus >     getThesaurus(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );

*/
    }
    else
    {
        aOptionsPB  .Enable(sal_False);
        aLinguGB    .Enable(sal_False);
        aLinguLB    .Enable(sal_False);
        aStdSpellCB .Enable(sal_False);
        aStdThesCB  .Enable(sal_False);
        aStdHyphCB  .Enable(sal_False);
        aSettingsGB .Enable(sal_False);
    }
}
/*-- 27.01.00 12:17:13---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxExternalLinguTabPage::~SvxExternalLinguTabPage()
{
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SvxExternalLinguTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxExternalLinguTabPage(pParent, rSet);
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SvxExternalLinguTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_uInt16 nSelectPos = aLinguLB.GetSelectEntryPos();
    if( xLinguProps.is() &&
        nSelectPos != aLinguLB.GetSavedValue() ||
        (nSelectPos &&
            (   aStdSpellCB.IsChecked() != aStdSpellCB.GetSavedValue() ||
                aStdHyphCB.IsChecked() != aStdHyphCB.GetSavedValue() ||
                aStdThesCB.IsChecked() != aStdThesCB.GetSavedValue())
            )
        )
    {
        sal_Int16 nTmpIndex = nSelectPos ? nSelectPos - 1: -1;

        Any aOldIndex = xLinguProps->getPropertyValue(C2U("OtherLinguIndex"));
        sal_Int16 nOldIndex;
        aOldIndex >>= nOldIndex;
        if (nOldIndex != nTmpIndex)
        {
            // force new spelling and flushing of spell caches
            //! current implementation is a workaround until the correct
            //! interface is implemented.
            //TL:TODO: use XPropertyChangeListener mechanism to do this
            Reference< XDictionary1 >  xDic( SvxGetIgnoreAllList() );
            if (xDic.is())
            {
                OUString aTmp( C2U("v_7xj4") );
                sal_Bool bOk = xDic->add( aTmp, sal_False, OUString() );
                if (bOk)
                    xDic->remove( aTmp );
            }
        }

        if (xOtherLingu.is())
            xOtherLingu->selectLinguisticByIndex( nTmpIndex );

        Any aIndex;
        aIndex <<= nTmpIndex;
        xLinguProps->setPropertyValue(C2U("OtherLinguIndex"), aIndex);
        if(aStdSpellCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardSpellChecker"),
                    lcl_Bool2Any(aStdSpellCB.IsChecked()));
        }

        if(aStdThesCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardThesaurus"),
                                lcl_Bool2Any(aStdThesCB.IsChecked()));
        }

        if(aStdHyphCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardHyphenator"),
                lcl_Bool2Any(aStdHyphCB.IsChecked()));
        }

    }

    // The spellchecker may have changed or an external linguistics option
    // page may have changed relevant options or have added/deleted words from
    // their dictionaries.
    // Thus the only safe choice is to trigger spellchecking again.
    //TL:TODO: should only be called if the external linguistics
    //  option dialog was used or the item-state has changed.
    if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SPELLCHECKER_CHANGED, SFX_CALLMODE_ASYNCHRON );

    return sal_True;
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvxExternalLinguTabPage::Reset( const SfxItemSet& rSet )
{
    aStdHyphCB.SaveValue();
    aStdThesCB.SaveValue();
    aStdSpellCB.SaveValue();
    aLinguLB.SaveValue();
}
/* -----------------------------27.01.00 13:26--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxExternalLinguTabPage, LBSelectHdl_Impl, ListBox*, pLB)
{
    if(!xOtherLingu.is())
        return 0;

    sal_uInt16 nSel = pLB->GetSelectEntryPos();

    aOptionsPB.Enable(nSel > 0 && xOtherLingu->hasOptionDialog( nSel - 1));
    sal_uInt8 nUse = 0;
    if( nSel > 0 )
    {
        --nSel;
        aStdSpellCB.Enable( xOtherLingu->hasSpellChecker(nSel) );
        aStdThesCB.Enable( xOtherLingu->hasThesaurus(nSel) );
        aStdHyphCB.Enable( xOtherLingu->hasHyphenator(nSel) );
    }
    else
    {
        aStdSpellCB.Enable( sal_False );
        aStdThesCB.Enable( sal_False );
        aStdHyphCB.Enable( sal_False );
    }
    return 0;
}
/* -----------------------------27.01.00 13:26--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxExternalLinguTabPage, OptDlgHdl_Impl, PushButton*, pButton)
{
    sal_uInt16 nSel = aLinguLB.GetSelectEntryPos();
    if(nSel && xOtherLingu.is())
    {
        if(xOtherLingu->hasOptionDialog( nSel - 1 ))
        {
            xOtherLingu->selectLinguisticByIndex( nSel - 1 );
            xOtherLingu->startOptionDialog( 0 );
        }
    }
    return 0;
}



