/*************************************************************************
 *
 *  $RCSfile: olmenu.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:44 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif
#ifndef _SVX_SPLDLG_HXX
#include <svx/spldlg.hxx>
#endif

#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XOTHERLINGU2_HPP_
#include <com/sun/star/linguistic/XOtherLingu2.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif


#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        //CheckSpellChanges
#endif
#ifndef _OLMENU_HXX
#include <olmenu.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _TEMPAUTO_HXX
#include <tempauto.hxx>     // temporaere Autokorrektur
#endif
#ifndef _OLMENU_HRC
#include <olmenu.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)

/*--------------------------------------------------------------------------

---------------------------------------------------------------------------*/

util::Language lcl_CheckLanguage( const OUString &rWord, uno::Reference< linguistic::XSpellChecker1 >  xSpell )
{
    // andere Sprachen automatisch pr"ufen
    // auch wenn beliebiger Schwachsinn rauskommt
    // NUR: wenn Sprache KEINE oder die neue die Alte ist oder die Sprache
    // nicht vorhanden ist

     util::Language nLang = LANGUAGE_NONE;

    uno::Reference< linguistic::XSpellAlternatives >    xAlt;
    uno::Sequence< util::Language > aLangs;
    if (xSpell.is())
        aLangs = xSpell->getLanguages();
    const util::Language *pLang = aLangs.getConstArray();
    sal_Int16   nCount = aLangs.getLength();

    for (sal_Int16 i = 0;  i < nCount;  i++)
    {
        if (pLang[i] != LANGUAGE_NONE)
        {
            if (xSpell->isValid( rWord, pLang[i] ))
            {
                nLang = pLang[i];
                break;
            }
        }
    }
    return nLang;
}


/*SwSpellPopup::SwSpellPopup( SwWrtShell* pWrtSh, const uno::Reference< linguistic::XSpellAlternatives >  &xAlt ) :
    PopupMenu(SW_RES(MN_SPELL_POPUP)),
    pSh ( pWrtSh ),
    xSpellAlt   (xAlt)
{
    DBG_ASSERT(xSpellAlt.is(), "no spelling alternatives available");

    uno::Sequence< UString >    aStrings;
    if (xSpellAlt.is())
        aStrings = xSpellAlt->getAlternatives();
    const UString *pString = aStrings.getConstArray();
    sal_Int16 nStringCount = aStrings.getLength();

    PopupMenu *pMenu = GetPopupMenu(MN_AUTOCORR);
    sal_Bool bEnable = sal_False;
    if( nStringCount )
    {
        InsertSeparator(0);
        bEnable = sal_True;
        for( sal_uInt16 i = 0, nPos = 1, nId = MN_AUTOCORR_START + 1;
            i < nStringCount; ++i, ++nPos, ++nId )
        {
            const String aEntry = U2S( pString[ i ] );
            InsertItem( nPos, aEntry, 0, i );
            SetHelpId( nPos, HID_LINGU_REPLACE);
            pMenu->InsertItem( nId, aEntry );
            pMenu->SetHelpId( nId, HID_LINGU_AUTOCORR);
        }
    }
    EnableItem( MN_AUTOCORR, bEnable );

    nCorrLang = LANGUAGE_NONE;
    if (xSpellAlt.is())
        nCorrLang = lcl_CheckLanguage( xSpellAlt->getWord(), ::GetSpellChecker() );
    if( nCorrLang != LANGUAGE_NONE )
    {
        InsertSeparator();
        String aTmp( ::GetLanguageString( nCorrLang ) );
        InsertItem( MN_LANGUAGE_WORD, String( SW_RES( STR_WORD ) ) + aTmp );
        SetHelpId( MN_LANGUAGE_WORD, HID_LINGU_WORD_LANGUAGE );
        InsertItem( MN_LANGUAGE_PARA, String( SW_RES( STR_PARAGRAPH ) ) + aTmp );
        SetHelpId( MN_LANGUAGE_PARA, HID_LINGU_PARA_LANGUAGE );
    }

    pMenu = GetPopupMenu(MN_INSERT);
    //DBG_ASSERT(0, "Fremdlinguistik fehlt noch");
<<<<<<< olmenu.cxx
#ifdef NOT_YET
    if( pSpell->HasOtherLingu() && pSpell->HasOtherSpell() )
    {
        bEnable = sal_True;
        pMenu->InsertItem( MN_INSERT_START,
            pSpell->GetOtherLinguName( pSpell->GetOtherIdx() ) );
        pMenu->SetHelpId( MN_INSERT_START, HID_LINGU_ADD_WORD);
    }
    else
#endif
    {
        bEnable = sal_False;

        uno::Reference< linguistic::XDictionaryList >   xDicList( SvxGetDictionaryList() );
=======


    bEnable = FALSE;    // enable MN_INSERT?

    INT16 nOtherIndex       = -1;
    BOOL  bHasOtherFunc     = FALSE;
    BOOL  bIsStandardSpell  = TRUE;
    XPropertySetRef xProp( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        nOtherIndex = xProp->getPropertyValue(
                            S2U(UPN_OTHER_LINGU_INDEX) ).getINT16();
        bIsStandardSpell = xProp->getPropertyValue(
                            S2U(UPN_IS_STANDARD_SPELL_CHECKER) ).getBOOL();
    }

    if (nOtherIndex >= 0)
    {
        XMultiServiceFactoryRef xMgr( NAMESPACE_USR( getProcessServiceManager)() );
        xOther = XOtherLinguRef( xMgr->createInstance(
                        S2U("com.sun.star.linguistic.OtherLingu") ), USR_QUERY );
        if (xOther.is())
        {
            bHasOtherFunc = xOther->hasSpellChecker( nOtherIndex );
            if (  bHasOtherFunc)
            {
                bEnable = TRUE;
                pMenu->InsertItem( MN_INSERT_START,
                    U2S( xOther->getIdentifier( nOtherIndex ) ) );
                pMenu->SetHelpId( MN_INSERT_START, HID_LINGU_ADD_WORD);
            }
        }
    }

    if (nOtherIndex < 0  ||  bIsStandardSpell)
    {
        XDictionaryListRef  xDicList( SvxGetDictionaryList() );
>>>>>>> 1.55
        if (xDicList.is())
        {
            // add active, positive dictionary to dic-list (if not already done).
            // This is to ensure that there is at least on dictionary to which
            // words could be added.
            uno::Reference< linguistic::XDictionary1 >  xDic( SvxGetOrCreatePosDic( xDicList ) );
            if (xDic.is())
                xDic->setActive( sal_True );

            aDics = xDicList->getDictionaries();
            const uno::Reference< linguistic::XDictionary >  *pDic = aDics.getConstArray();
            sal_Int32 nDicCount = aDics.getLength();

            sal_Int16 nLanguage = LANGUAGE_NONE;
            if (xSpellAlt.is())
                nLanguage = SvxLocaleToLanguage( xSpellAlt->getLocale() );

            for( sal_Int32 i = 0; i < nDicCount; i++ )
            {
                uno::Reference< linguistic::XDictionary1 >  xDic( pDic[i], uno::UNO_QUERY );
                if (!xDic.is())
                    continue;

                LanguageType nActLanguage = xDic->getLanguage();
                if( xDic->isActive()
                    &&  xDic->getDictionaryType() != linguistic::DictionaryType_NEGATIVE
                    && (nLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                    && SvxGetIgnoreAllList() != xDic )
                {
                    // the extra 1 is because of the (possible) external
                    // linguistic entry above
                    pMenu->InsertItem( MN_INSERT_START + i + 1, U2S( xDic->getName() ) );
                    bEnable = sal_True;
                }
            }
        }
    }
    EnableItem( MN_INSERT, bEnable );

    RemoveDisabledEntries( sal_True, sal_True );
}
*/
SwSpellPopup::SwSpellPopup( SwWrtShell* pWrtSh, const uno::Reference< linguistic::XSpellAlternatives >  &xAlt ) :
    PopupMenu(SW_RES(MN_SPELL_POPUP)),
    pSh ( pWrtSh ),
    xSpellAlt   (xAlt)
{
    DBG_ASSERT(xSpellAlt.is(), "no spelling alternatives available");

    uno::Sequence< OUString >   aStrings;
    if (xSpellAlt.is())
        aStrings = xSpellAlt->getAlternatives();
    const OUString *pString = aStrings.getConstArray();
    sal_Int16 nStringCount = aStrings.getLength();

    PopupMenu *pMenu = GetPopupMenu(MN_AUTOCORR);
    sal_Bool bEnable = sal_False;
    if( nStringCount )
    {
        InsertSeparator(0);
        bEnable = sal_True;
        for( sal_uInt16 i = 0, nPos = 1, nId = MN_AUTOCORR_START + 1;
            i < nStringCount; ++i, ++nPos, ++nId )
        {
            const String aEntry = pString[ i ];
            InsertItem( nPos, aEntry, 0, i );
            SetHelpId( nPos, HID_LINGU_REPLACE);
            pMenu->InsertItem( nId, aEntry );
            pMenu->SetHelpId( nId, HID_LINGU_AUTOCORR);
        }
    }
    EnableItem( MN_AUTOCORR, bEnable );

    nCorrLang = LANGUAGE_NONE;
    if (xSpellAlt.is())
        nCorrLang = lcl_CheckLanguage( xSpellAlt->getWord(), ::GetSpellChecker() );
    if( nCorrLang != LANGUAGE_NONE )
    {
        InsertSeparator();
        String aTmp( ::GetLanguageString( nCorrLang ) );
        InsertItem( MN_LANGUAGE_WORD, String( SW_RES( STR_WORD ) ).Append(aTmp) );
        SetHelpId( MN_LANGUAGE_WORD, HID_LINGU_WORD_LANGUAGE );
        InsertItem( MN_LANGUAGE_PARA, String( SW_RES( STR_PARAGRAPH ) ).Append(aTmp) );
        SetHelpId( MN_LANGUAGE_PARA, HID_LINGU_PARA_LANGUAGE );
    }

    pMenu = GetPopupMenu(MN_INSERT);
    //DBG_ASSERT(0, "Fremdlinguistik fehlt noch");

    bEnable = FALSE;    // enable MN_INSERT?

    sal_Int16 nOtherIndex       = -1;
    sal_Bool  bHasOtherFunc     = FALSE;
    sal_Bool  bIsStandardSpell  = TRUE;
    uno::Reference< beans::XPropertySet > xProp( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        nOtherIndex = *(sal_Int16*)xProp->getPropertyValue(C2U(UPN_OTHER_LINGU_INDEX) ).getValue();
        bIsStandardSpell = *(sal_Bool*)xProp->getPropertyValue(
                            C2U(UPN_IS_STANDARD_SPELL_CHECKER) ).getValue();
    }

    if (nOtherIndex >= 0)
    {

        uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = utl::getProcessServiceFactory();
        xOther = uno::Reference< linguistic::XOtherLingu >( xMgr->createInstance(
                            C2U("com.sun.star.linguistic.OtherLingu") ), uno::UNO_QUERY );
        if (xOther.is())
        {
            bHasOtherFunc = xOther->hasSpellChecker( nOtherIndex );
            if (/*nOtherIndex >= 0  &&*/  bHasOtherFunc)
            {
                bEnable = TRUE;
                pMenu->InsertItem( MN_INSERT_START,
                    xOther->getIdentifier( nOtherIndex ) );
                pMenu->SetHelpId( MN_INSERT_START, HID_LINGU_ADD_WORD);
            }
        }
    }

    if (nOtherIndex < 0  ||  bIsStandardSpell)
    {
        uno::Reference< linguistic::XDictionaryList >   xDicList( SvxGetDictionaryList() );
        if (xDicList.is())
        {
            // add active, positive dictionary to dic-list (if not already done).
            // This is to ensure that there is at least on dictionary to which
            // words could be added.
            uno::Reference< linguistic::XDictionary1 >  xDic( SvxGetOrCreatePosDic( xDicList ) );
            if (xDic.is())
                xDic->setActive( sal_True );

            aDics = xDicList->getDictionaries();
            const uno::Reference< linguistic::XDictionary >  *pDic = aDics.getConstArray();
            sal_Int32 nDicCount = aDics.getLength();

            sal_Int16 nLanguage = LANGUAGE_NONE;
            if (xSpellAlt.is())
                nLanguage = SvxLocaleToLanguage( xSpellAlt->getLocale() );

            for( sal_Int32 i = 0; i < nDicCount; i++ )
            {
                uno::Reference< linguistic::XDictionary1 >  xDic( pDic[i], uno::UNO_QUERY );
                if (!xDic.is() || SvxGetIgnoreAllList() == xDic)
                    continue;

                uno::Reference< frame::XStorable > xStor( xDic, uno::UNO_QUERY );
                LanguageType nActLanguage = xDic->getLanguage();
                if( xDic->isActive()
                    &&  xDic->getDictionaryType() != linguistic::DictionaryType_NEGATIVE
                    && (nLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                    && (!xStor.is() || !xStor->isReadonly()) )
                {
                    // the extra 1 is because of the (possible) external
                    // linguistic entry above
                    pMenu->InsertItem( MN_INSERT_START + i + 1, xDic->getName() );
                    bEnable = sal_True;
                }
            }
        }
    }
    EnableItem( MN_INSERT, bEnable );

    RemoveDisabledEntries( TRUE, TRUE );

}

/*--------------------------------------------------------------------------

---------------------------------------------------------------------------*/
sal_uInt16  SwSpellPopup::Execute( Window* pWin, const Point& rWordPos )
{
    sal_uInt16 nRet = PopupMenu::Execute(pWin, pWin->LogicToPixel(rWordPos));
    sal_Bool bAutoCorr = sal_False;
    if( nRet > MN_AUTOCORR_START && nRet != USHRT_MAX )
    {
        nRet -= MN_AUTOCORR_START;
        bAutoCorr = sal_True;
    }

    if( nRet && nRet != USHRT_MAX)
    {
        int nAltIdx = nRet - 1;
        if( xSpellAlt.is()  &&  nAltIdx < xSpellAlt->getAlternativesCount() )
        {
            sal_Bool bOldIns = pSh->IsInsMode();
            pSh->SetInsMode( sal_True );

            pSh->StartUndo(UIUNDO_REPLACE);
            pSh->StartAction();
            pSh->DelLeft();

            const OUString *pString = xSpellAlt->getAlternatives().getConstArray();
            DBG_ASSERT( 0 <= nAltIdx && nAltIdx <= xSpellAlt->getAlternativesCount(),
                    "index out of range");
            String aTmp( pString[ nAltIdx ] );
            String aOrig( xSpellAlt->getWord());

            // if orginal word has a trailing . (likely the end of a sentence)
            // and the replacement text hasn't, then add it to the replacement
            if (aTmp.Len() && aOrig.Len() &&
                '.' == aOrig.GetChar( aOrig.Len() - 1) && /* !IsAlphaNumeric ??*/
                '.' != aTmp.GetChar( aTmp.Len() - 1))
            {
                aTmp += '.';
            }

            pSh->Insert( aTmp );
            pSh->EndAction();
            pSh->EndUndo(UIUNDO_REPLACE);

            pSh->SetInsMode( bOldIns );


            // nur aufnehmen, wenn es NICHT schon in der Autokorrektur vorhanden ist
            SvxAutoCorrect* pACorr = OFF_APP()->GetAutoCorrect();

            LanguageType eLanguage = SvxLocaleToLanguage( xSpellAlt->getLocale() );

            String aOrigWord( xSpellAlt->getWord() ) ;
            String aNewWord;
            if( nRet )
                aNewWord = pString[ nAltIdx ];
            else
                aNewWord = aOrigWord;
            SvxPrepareAutoCorrect( aOrigWord, aNewWord );
            if( bAutoCorr )
            {
                pACorr->PutText( aOrigWord, aNewWord, eLanguage );
                if( pTempAuto )
                    pTempAuto->Delete( aOrigWord );
            }
            else
            {
                SvxAutocorrWord aAWord( aOrigWord, aEmptyStr );
                if( !pACorr->GetAutocorrWordList(eLanguage)->Seek_Entry( &aAWord ))
                {
                    SwCorrection* pCorr = new SwCorrection( aAWord.GetShort() );
                    pCorr->Correct() = aNewWord;
                    if( !pTempAuto )
                        pTempAuto = new SwTempAuto();
                    pTempAuto->Insert( pCorr );
                }
            }

        }
        else
            switch( nRet )
            {
                case MN_SPELLING:
                {
                    pSh->Left();
                    {
                        SvxDicListChgClamp aClamp( SvxGetDictionaryList() );
                        pSh->GetView().GetViewFrame()->GetDispatcher()->
                            Execute( FN_SPELLING_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    // should be superfluos by adding the ignore list to
                    // the dictionary list.
                    // SW_MOD()->CheckSpellChanges( sal_False, );
                }
                break;
                case MN_IGNORE :
                {
                    sal_Int16 nAddRes = SvxAddEntryToDic(
                            uno::Reference< linguistic::XDictionary > ( SvxGetIgnoreAllList(), uno::UNO_QUERY ),
                            xSpellAlt->getWord(), sal_False,
                            aEmptyStr, LANGUAGE_NONE );
                }
                break;
                case MN_INSERT:
                    DBG_ERROR("geht noch nicht!")
                break;
                case MN_LANGUAGE_WORD:
                case MN_LANGUAGE_PARA:
                {
                    pSh->StartAction();

                    if( MN_LANGUAGE_PARA == nRet )
                    {
                        if( !pSh->IsSttPara() )
                            pSh->MovePara( fnParaCurr, fnParaStart );
                        pSh->SwapPam();
                        if( !pSh->IsEndPara() )
                            pSh->MovePara( fnParaCurr,  fnParaEnd );
                    }

                    SfxItemSet aSet(pSh->GetAttrPool(), RES_CHRATR_LANGUAGE,
                                                        RES_CHRATR_LANGUAGE);
                    aSet.Put( SvxLanguageItem( nCorrLang ) );
                    pSh->SetAttr( aSet );

                    pSh->EndAction();
                }
                break;
                default:
                    if(nRet >= MN_INSERT_START )
                    {
                        OUString aWord( xSpellAlt->getWord() );
                        if (MN_INSERT_START == nRet)
                        {
                            uno::Reference< linguistic::XOtherLingu2 > xOther2( xOther, uno::UNO_QUERY );
                            if (xOther2.is())
                            {
                                xOther2->addWord( 0, aWord,
                                    SvxLocaleToLanguage( xSpellAlt->getLocale() ) );

                                // first parameter is TRUE since this menue
                                // is only invoked while online-spelling is
                                // active
                                SW_MOD()->CheckSpellChanges( TRUE, TRUE, FALSE );
                            }
                        }
                        else
                        {
                            INT32 nDicIdx = nRet - MN_INSERT_START - 1;
                            DBG_ASSERT( nDicIdx < aDics.getLength(),
                                        "dictionary index out of range" );
                            uno::Reference< linguistic::XDictionary > xDic =
                                aDics.getConstArray()[nDicIdx];
                            INT16 nAddRes = SvxAddEntryToDic( xDic,
                                aWord, FALSE, aEmptyStr, LANGUAGE_NONE );

                            /*if (DIC_ERR_NONE == nAddRes)
                                SW_MOD()->CheckSpellChanges( FALSE, TRUE, FALSE );
                            else*/
                            if (DIC_ERR_NONE != nAddRes
                                 && !xDic->getEntry( aWord ).is())
                            {
                                SvxDicError(
                                    &pSh->GetView().GetViewFrame()->GetWindow(),
                                    nAddRes );
                            }
                        }

                        //should work with dictionaryListener mechanism
                        //SW_MOD()->CheckSpellChanges( FALSE, FALSE, FALSE );
                    }
            }
    }
    pSh->EnterStdMode();
    return nRet;
}




/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.64  2000/09/18 16:05:54  willem.vandorp
      OpenOffice header added.

      Revision 1.63  2000/09/07 15:59:25  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.62  2000/05/24 11:54:40  khz
      Change for Unicode

      Revision 1.61  2000/05/16 17:27:46  jp
      Changes for Unicode

      Revision 1.60  2000/05/16 09:15:13  os
      project usr removed

      Revision 1.59  2000/05/09 14:46:08  os
      UNICODE

      Revision 1.58  2000/04/18 15:35:07  os
      UNICODE

      Revision 1.57  2000/04/12 11:01:24  tl
      #74727# bugfix von src569 nachgezogen

      Revision 1.56  2000/03/23 07:32:27  os
      UNO III

      Revision 1.55  2000/03/16 10:51:17  tl
      #73839# use language of word to check for dictionories to be displayed in SwSpellPopup

      Revision 1.54  2000/03/15 14:49:19  tl
      #70734# AddWord for external linguistic added

      Revision 1.53  2000/03/07 10:40:19  tl
      #72833# SvxPrepareAutoCorrect used

      Revision 1.52  2000/02/25 10:44:07  tl
      #62869# new error message when adding word to dictionary failed

      Revision 1.51  2000/02/22 16:38:05  tl
      #73380# standard.dic created, acivated and added to list

      Revision 1.50  2000/02/21 15:57:17  tl
      #72081# strip '.' from word before adding to dictionaries

      Revision 1.49  2000/02/16 20:58:50  tl
      #72219# Locale Umstellung

      Revision 1.48  2000/02/04 12:22:19  tl
      #72446# IgnoreAllList removed from list of selectable dictionaries

      Revision 1.47  2000/02/01 11:53:12  tl
      #71634# insert '.' to replacement if original word has it

      Revision 1.46  2000/01/11 10:28:08  tl
      #70735# use isActive/setActive from linguistic::XDictionary now

      Revision 1.45  1999/12/22 19:56:14  jp
      Bug #71270#: SwSpellPopup - insert separator

      Revision 1.44  1999/12/16 13:52:07  tl
      #70973# PopupMenu ok even without spelling alternatives

      Revision 1.43  1999/11/24 18:30:28  tl
      check for Service availability

      Revision 1.42  1999/11/19 16:40:23  os
      modules renamed

      Revision 1.41  1999/11/10 11:03:36  tl
      Ongoing ONE_LINGU implementation

      Revision 1.40  1999/10/18 13:15:34  os
      external lingu: no special handling in WIN

      Revision 1.39  1999/09/21 10:22:26  rt
      #65293# sysdep.hxx -> svwin.h

      Revision 1.38  1999/09/21 10:16:23  os
      Sysdepen::GethWnd -> return 0

      Revision 1.37  1999/09/20 11:08:21  hr
      #65293#: removed <vcl/sysdep.hxx>

      Revision 1.36  1999/08/31 08:37:26  TL
      #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.35   31 Aug 1999 10:37:26   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.34   22 Feb 1999 13:37:56   MA
   #62126# gesperrte Eintraege entfernen lassen

      Rev 1.33   24 Nov 1998 10:05:10   OS
   #59590# mehrsprachige Autokorrektur

      Rev 1.32   29 Oct 1998 16:29:54   AMA
   Fix #58621#: Falsch geschriebene Woerter _nicht_ in AusnahmeWB uebernehmen

      Rev 1.31   28 Jul 1998 15:34:50   AMA
   Fix #54147#: Nicht zuviel loeschen, Ueberschreibmodus kurzzeitig ausschalten

      Rev 1.30   09 Jun 1998 15:28:44   AMA
   Fix #50994#: Bei der Absatzselektion nicht uebers Ziel hinausschiessen

      Rev 1.29   07 May 1998 15:05:04   AMA
   New: SpellPopup bietet Sprachumstellung am Wort oder Absatz an

      Rev 1.28   29 Apr 1998 09:30:04   MA
   ucb::Command, Contextmenu auch fuer Tastatur

      Rev 1.27   21 Nov 1997 13:00:26   MA
   includes

      Rev 1.26   18 Sep 1997 14:00:10   OS
   Pointer::GetPosPixel fuer VCL ersetzt

      Rev 1.25   29 Aug 1997 16:35:50   MH
   chg: Syntax

      Rev 1.24   29 Aug 1997 15:59:24   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.23   29 Aug 1997 14:02:46   OS
   DLL-Umbau

      Rev 1.22   07 Aug 1997 14:34:00   HJS
   includes

      Rev 1.21   14 Jul 1997 08:07:58   OS
   HelpIDs fuer SpellPopup #41627#

      Rev 1.20   30 Jun 1997 10:29:06   MH
   cat fuer OS2

      Rev 1.19   26 Jun 1997 13:31:40   AMA
   New: AddWord mit Window-Handle + Automatisches Aufnehmen unbekannter Woerter

      Rev 1.18   05 Jun 1997 17:28:52   AMA
   New: Fremdlinguistik nimmt auch Woerter auf

      Rev 1.17   04 Feb 1997 13:13:00   AMA
   New: Online-Spelling-Vorschlag in die Autokorrektur aufnehmen

      Rev 1.16   11 Nov 1996 10:58:52   MA
   ResMgr

      Rev 1.15   06 Nov 1996 19:57:04   MH
   SwSplCfg -> OfaSplCfg

      Rev 1.14   24 Oct 1996 13:36:28   JP
   String Umstellung: [] -> GetChar()

      Rev 1.13   23 Oct 1996 13:49:28   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.12   15 Oct 1996 14:14:46   JP
   AutoCorrCfg in die OfficeApplication verschoben

      Rev 1.11   28 Aug 1996 12:52:34   OS
   includes

      Rev 1.10   21 Jun 1996 10:33:32   AMA
   New #28010#: Temporaere Autokorrektur

      Rev 1.9   20 May 1996 09:58:38   AMA
   Fix: SvStringsDtor statt SvStrings

      Rev 1.8   26 Apr 1996 15:47:10   AMA
   Fix #27394#: Geprueftes Wort jetzt als erstes der Alternativen

      Rev 1.7   26 Apr 1996 10:18:10   AMA
   Fix #27394#: Geprueftes Wort jetzt als erstes Element der Alternativ-Liste.

      Rev 1.6   23 Apr 1996 10:33:28   AMA
   Fix #27260#: Online-Spelling-PopUp und Punkte, Anfuehrungszeichen etc.

      Rev 1.5   05 Mar 1996 11:13:30   AMA
   Fix: GetCorrect() selektiert; Fix: Punkte bei Abkrzg. und Satzende beachten

      Rev 1.4   29 Feb 1996 10:31:30   AMA
   Opt: SpellItAgainSam startet Idlen ggf. auch ohne Spell-Invalidierung

      Rev 1.3   27 Feb 1996 17:04:56   AMA
   New: CheckSpellChanges()

      Rev 1.2   26 Feb 1996 07:50:56   OS
   Aufnehmen-enable berichtigt

      Rev 1.1   23 Feb 1996 08:33:42   OS
   Woerterbuecher nach Sprache anbieten, Woerter aufnehmen

      Rev 1.0   22 Feb 1996 15:55:28   OS
   Initial revision.

*************************************************************************/




















































