/*************************************************************************
 *
 *  $RCSfile: unolingu.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-27 07:37:52 $
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

#include <unolingu.hxx>

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#include <comphelper/processfactory.hxx>

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#include "dialogs.hrc"

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;

///////////////////////////////////////////////////////////////////////////

typedef cppu::WeakImplHelper1 < XEventListener > LinguMgrAppExitLstnrBaseClass;

class LinguMgrAppExitLstnr : public LinguMgrAppExitLstnrBaseClass
{
    Reference< XComponent >         xDesktop;

public:
    LinguMgrAppExitLstnr();
    virtual ~LinguMgrAppExitLstnr();

    virtual void    AtExit() = 0;


    // lang::XEventListener
    virtual void    SAL_CALL disposing(const EventObject& rSource)
            throw( RuntimeException );
};

LinguMgrAppExitLstnr::LinguMgrAppExitLstnr()
{
    // add object to frame::Desktop EventListeners in order to properly call
    // the AtExit function at appliction exit.

    Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if ( xMgr.is() )
    {
        xDesktop = Reference< XComponent > ( xMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY ) ;
        if (xDesktop.is())
            xDesktop->addEventListener( this );
    }
}

LinguMgrAppExitLstnr::~LinguMgrAppExitLstnr()
{
    if (xDesktop.is())
    {
        xDesktop->removeEventListener( this );
        xDesktop = NULL;    //! release reference to desktop
    }
    DBG_ASSERT(!xDesktop.is(), "reference to desktop should be realeased");
}

void LinguMgrAppExitLstnr::disposing(const EventObject& rSource)
        throw( RuntimeException )
{
    if (xDesktop.is()  &&  rSource.Source == xDesktop)
    {
        xDesktop->removeEventListener( this );
        xDesktop = NULL;    //! release reference to desktop

        AtExit();
    }
}

///////////////////////////////////////////////////////////////////////////

class LinguMgrExitLstnr : public LinguMgrAppExitLstnr
{
public:
    virtual void    AtExit();
};

void LinguMgrExitLstnr::AtExit()
{
    // release references
    LinguMgr::xLngSvcMgr    = 0;
    LinguMgr::xSpell        = 0;
    LinguMgr::xHyph         = 0;
    LinguMgr::xThes         = 0;
    LinguMgr::xDicList      = 0;
    LinguMgr::xProp         = 0;
    LinguMgr::xIgnoreAll    = 0;
    LinguMgr::xChangeAll    = 0;

    LinguMgr::bExiting      = sal_True;

    //TL:TODO: MBA fragen wie ich ohne Absturz hier meinen Speicher
    //  wieder freibekomme...
    //delete LinguMgr::pExitLstnr;
    LinguMgr::pExitLstnr    = 0;
}

///////////////////////////////////////////////////////////////////////////


// static member initialization
LinguMgrExitLstnr *             LinguMgr::pExitLstnr    = 0;
sal_Bool                        LinguMgr::bExiting      = sal_False;
Reference< XLinguServiceManager >   LinguMgr::xLngSvcMgr    = 0;
Reference< XSpellChecker1 >     LinguMgr::xSpell        = 0;
Reference< XHyphenator >        LinguMgr::xHyph         = 0;
Reference< XThesaurus >         LinguMgr::xThes         = 0;
Reference< XDictionaryList >    LinguMgr::xDicList      = 0;
Reference< XPropertySet >       LinguMgr::xProp         = 0;
Reference< XDictionary1 >       LinguMgr::xIgnoreAll    = 0;
Reference< XDictionary1 >       LinguMgr::xChangeAll    = 0;


static Reference< XLinguServiceManager > GetLngSvcMgr_Impl()
{
    Reference< XLinguServiceManager > xRes;
    Reference< XMultiServiceFactory >  xMgr = getProcessServiceFactory();
    if (xMgr.is())
    {
        xRes = Reference< XLinguServiceManager > ( xMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM (
                    "com.sun.star.linguistic2.LinguServiceManager" ) ) ), UNO_QUERY ) ;
    }
    return xRes;
}


Reference< XSpellChecker1 > LinguMgr::GetSpellChecker()
{
    return xSpell.is() ? xSpell : GetSpell();
}

Reference< XHyphenator > LinguMgr::GetHyphenator()
{
    return xHyph.is() ? xHyph : GetHyph();
}

Reference< XThesaurus > LinguMgr::GetThesaurus()
{
    return xThes.is() ? xThes : GetThes();
}

Reference< XDictionaryList > LinguMgr::GetDictionaryList()
{
    return xDicList.is() ? xDicList : GetDicList();
}

Reference< XPropertySet > LinguMgr::GetLinguPropertySet()
{
    return xProp.is() ? xProp : GetProp();
}

Reference< XDictionary1 > LinguMgr::GetStandardDic()
{
    //! don't hold reference to this
    //! (it may be removed from dictionary list and needs to be
    //! created empty if accessed again)
    return GetStandard();
}

Reference< XDictionary1 > LinguMgr::GetIgnoreAllList()
{
    return xIgnoreAll.is() ? xIgnoreAll : GetIgnoreAll();
}

Reference< XDictionary1 > LinguMgr::GetChangeAllList()
{
    return xChangeAll.is() ? xChangeAll : GetChangeAll();
}

Reference< XSpellChecker1 > LinguMgr::GetSpell()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xSpell = Reference< XSpellChecker1 > (
                        xLngSvcMgr->getSpellChecker(), UNO_QUERY );
    }
    return xSpell;
}

Reference< XHyphenator > LinguMgr::GetHyph()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xHyph = xLngSvcMgr->getHyphenator();
    }
    return xHyph;
}

Reference< XThesaurus > LinguMgr::GetThes()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    if (!xLngSvcMgr.is())
        xLngSvcMgr = GetLngSvcMgr_Impl();

    if (xLngSvcMgr.is())
    {
        xThes = xLngSvcMgr->getThesaurus();
    }
    return xThes;
}

Reference< XDictionaryList > LinguMgr::GetDicList()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xDicList = Reference< XDictionaryList > ( xMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.linguistic2.DictionaryList") ), UNO_QUERY );
    }
    return xDicList;
}

Reference< XPropertySet > LinguMgr::GetProp()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    if (xMgr.is())
    {
        xProp = Reference< XPropertySet > ( xMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.linguistic2.LinguProperties") ), UNO_QUERY );
    }
    return xProp;
}

Reference< XDictionary1 > LinguMgr::GetIgnoreAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (xTmpDicList.is())
    {
        xIgnoreAll = Reference< XDictionary1 > ( xTmpDicList->getDictionaryByName(
                    OUString::createFromAscii("IgnoreAllList") ), UNO_QUERY );
    }
    return xIgnoreAll;
}

Reference< XDictionary1 > LinguMgr::GetChangeAll()
{
    if (bExiting)
        return 0;

    if (!pExitLstnr)
        pExitLstnr = new LinguMgrExitLstnr;

    Reference< XDictionaryList >  xDicList( GetDictionaryList() , UNO_QUERY );
    if (xDicList.is())
    {
        xChangeAll = Reference< XDictionary1 > (
                        xDicList->createDictionary(
                            OUString::createFromAscii("ChangeAllList"),
                            SvxCreateLocale( LANGUAGE_NONE ),
                            DictionaryType_NEGATIVE, String() ), UNO_QUERY );
    }
    return xChangeAll;
}

Reference< XDictionary1 > LinguMgr::GetStandard()
{
    // Tries to return a dictionary which may hold positive entries is
    // persistent and not read-only.

    if (bExiting)
        return 0;

    Reference< XDictionaryList >  xTmpDicList( GetDictionaryList() );
    if (!xTmpDicList.is())
        return NULL;

    const OUString aDicName( RTL_CONSTASCII_USTRINGPARAM( "standard.dic" ) );
    Reference< XDictionary1 >   xDic( xTmpDicList->getDictionaryByName( aDicName ),
                                      UNO_QUERY );
    Reference< XStorable >      xStor( xDic, UNO_QUERY );

    if (   !(xDic.is() && xDic->getDictionaryType() != DictionaryType_NEGATIVE &&
                xDic->getLanguage() == LANGUAGE_NONE)
        || !(xStor.is() && xStor->hasLocation() && !xStor->isReadonly()) )
    {
        Reference< XDictionary >  xTmp;

        // try to create standard dictionary
        try
        {
            xTmp =  xTmpDicList->createDictionary( aDicName,
                        SvxCreateLocale( LANGUAGE_NONE ),
                        DictionaryType_POSITIVE,
                        SvxGetDictionaryURL( aDicName, sal_True ) );
        }
        catch(...)
        {
        }

        // add new dictionary to list
        if (xTmp.is())
            xTmpDicList->addDictionary( xTmp );
        xDic = Reference< XDictionary1 > ( xTmp, UNO_QUERY );
    }
    DBG_ASSERT(xDic.is() && xDic->getDictionaryType() != DictionaryType_NEGATIVE,
        "wrong dictionary type");

    return xDic;
}

///////////////////////////////////////////////////////////////////////////

Reference< XSpellChecker1 >  SvxGetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

Reference< XHyphenator >  SvxGetHyphenator()
{
    return LinguMgr::GetHyphenator();
}

Reference< XThesaurus >  SvxGetThesaurus()
{
    return LinguMgr::GetThesaurus();
}

Reference< XDictionaryList >  SvxGetDictionaryList()
{
    return LinguMgr::GetDictionaryList();
}

Reference< XPropertySet >   SvxGetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one
Reference< XDictionary1 >  SvxGetOrCreatePosDic(
        Reference< XDictionaryList >  xDicList )
{
    return LinguMgr::GetStandardDic();
}

Reference< XDictionary1 >  SvxGetIgnoreAllList()
{
    return LinguMgr::GetIgnoreAllList();
}

Reference< XDictionary1 >  SvxGetChangeAllList()
{
    return LinguMgr::GetChangeAllList();
}

///////////////////////////////////////////////////////////////////////////


#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord )
{
    SvxAlternativeSpelling aRes;
    if (rHyphWord.is() && rHyphWord->isAlternativeSpelling())
    {
        OUString aWord( rHyphWord->getWord() ),
                 aHyphenatedWord( rHyphWord->getHyphenatedWord() );
        INT16   nHyphenationPos     = rHyphWord->getHyphenationPos(),
                nHyphenPos          = rHyphWord->getHyphenPos();
        const sal_Unicode *pWord    = aWord.getStr(),
                          *pAltWord = aHyphenatedWord.getStr();

        // at least char changes directly left or right to the hyphen
        // should(!) be handled properly...
        //! nHyphenationPos and nHyphenPos differ at most by 1 (see above)
        //! Beware: eg "Schiffahrt" in German (pre spelling reform)
        //! proves to be a bit nasty (nChgPosLeft and nChgPosRight overlap
        //! to an extend.)

        // find first different char from left
        sal_Int32   nPosL    = 0,
                    nAltPosL = 0;
        for (INT16 i = 0 ;  pWord[ nPosL ] == pAltWord[ nAltPosL ];  nPosL++, nAltPosL++, i++)
        {
            // restrict changes area beginning to the right to
            // the char immediately following the hyphen.
            //! serves to insert the additional "f" in "Schiffahrt" at
            //! position 5 rather than position 6.
            if (i >= nHyphenationPos + 1)
                break;
        }

        // find first different char from right
        sal_Int32   nPosR    = aWord.getLength() - 1,
                    nAltPosR = aHyphenatedWord.getLength() - 1;
        for ( ;  nPosR >= nPosL  &&  nAltPosR >= nAltPosL
                    &&  pWord[ nPosR ] == pAltWord[ nAltPosR ];
              nPosR--, nAltPosR--)
            ;

        INT16   nChgLen = nPosR - nPosL + 1;
        DBG_ASSERT( nChgLen >= 0, "nChgLen < 0");

        sal_Int32 nTxtStart = nPosL;
        sal_Int32 nTxtLen   = nAltPosL - nPosL + 1;
        OUString aRplc( aHyphenatedWord.copy( nTxtStart, nTxtLen ) );

        aRes.aReplacement       = aRplc;
        aRes.nChangedPos        = (INT16) nPosL;
        aRes.nChangedLength     = nChgLen;
        aRes.bIsAltSpelling     = TRUE;
        aRes.xHyphWord          = rHyphWord;
    }
    return aRes;
}


///////////////////////////////////////////////////////////////////////////

SvxDicListChgClamp::SvxDicListChgClamp( Reference< XDictionaryList >  &rxDicList ) :
    xDicList    ( rxDicList )
{
    if (xDicList.is())
    {
        xDicList->beginCollectEvents();
    }
}

SvxDicListChgClamp::~SvxDicListChgClamp()
{
    if (xDicList.is())
    {
        xDicList->endCollectEvents();
    }
}

///////////////////////////////////////////////////////////////////////////

//! some code somewhere requires LANGUAGE_NONE to be the first entry!
static const Language aLanguages[] =
{
    LANGUAGE_NONE,
    LANGUAGE_GERMAN,
    LANGUAGE_GERMAN_SWISS,
    LANGUAGE_ENGLISH_US,
    LANGUAGE_ENGLISH_UK,
    LANGUAGE_FRENCH,
    LANGUAGE_ITALIAN,
    LANGUAGE_SPANISH,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_DANISH,
    LANGUAGE_DUTCH,
    LANGUAGE_SWEDISH,
    LANGUAGE_FINNISH,
    LANGUAGE_NORWEGIAN_BOKMAL,
    LANGUAGE_NORWEGIAN_NYNORSK,

    // newly IPR supported languages
    LANGUAGE_AFRIKAANS,
    LANGUAGE_CATALAN,
    LANGUAGE_CZECH,
    LANGUAGE_GREEK,
    LANGUAGE_HUNGARIAN,
    LANGUAGE_POLISH,
    LANGUAGE_PORTUGUESE_BRAZILIAN,
    LANGUAGE_RUSSIAN
};

const Sequence< Language > & SvxGetSelectableLanguages()
{
    static const Sequence< Language >
        aSeq( aLanguages, (sizeof(aLanguages) / sizeof(aLanguages[0])) );
    return aSeq;
}

sal_Int32 SvxGetLanguagePos(const Sequence< Language > &rSeq, Language nLang)
{
    sal_Int32 nRes = -1;

    sal_Int32 n = rSeq.getLength();
    const Language *pLang = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < n;  i++)
        if (pLang[i] == nLang)
        {
            nRes = i;
            break;
        }

    return nRes;
}

String SvxGetDictionaryURL(const String &rDicName, sal_Bool bIsUserDic)
{
    // build URL to use for new (persistent) dictionaries

    SvtPathOptions aPathOpt;
    String aDirName( bIsUserDic ?
            aPathOpt.GetUserDictionaryPath() : aPathOpt.GetDictionaryPath() );

    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( aDirName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
    aURLObj.Append( rDicName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");

    return aURLObj.GetMainURL();
}

//TL:TODO: soll mal den rictigen Rückgabetyp bekommen!
sal_Bool SvxAddEntryToDic(
        Reference< XDictionary >  &rxDic,
        const OUString &rWord, sal_Bool bIsNeg,
        const OUString &rRplcTxt, sal_Int16 nRplcLang,
        sal_Bool bStripDot )
{
    if (!rxDic.is())
        return DIC_ERR_NOT_EXISTS;

    OUString aTmp( rWord );
    if (bStripDot)
    {
        sal_Int32 nLen = rWord.getLength();
        if (nLen > 0  &&  '.' == rWord[ nLen - 1])
        {
            // remove trailing '.'
            // (this is the official way to do this :-( )
            aTmp = aTmp.copy( 0, nLen - 1 );
        }
    }
    sal_Bool bAddOk = rxDic->add( aTmp, bIsNeg, rRplcTxt );

    sal_Int16 nRes = DIC_ERR_NONE;
    if (!bAddOk)
    {
        if (rxDic->isFull())
            nRes = DIC_ERR_FULL;
        else
        {
            Reference< XStorable >  xStor( rxDic, UNO_QUERY );
            if (xStor.is() && xStor->isReadonly())
                nRes = DIC_ERR_READONLY;
            else
                nRes = DIC_ERR_UNKNOWN;
        }
    }

    return nRes;
}

short SvxDicError( Window *pParent, sal_Int16 nError )
{
    short nRes = 0;
    if (DIC_ERR_NONE != nError)
    {
        int nRid;
        switch (nError)
        {
            case DIC_ERR_FULL     : nRid = RID_SVXSTR_DIC_ERR_FULL;  break;
            case DIC_ERR_READONLY : nRid = RID_SVXSTR_DIC_ERR_READONLY;  break;
            default:
                nRid = RID_SVXSTR_DIC_ERR_UNKNOWN;
                DBG_ASSERT(0, "unexpected case");
        }
        nRes = InfoBox( pParent, SVX_RESSTR( nRid ) ).Execute();
    }
    return nRes;
}

sal_Bool SvxSaveDictionaries( const Reference< XDictionaryList >  &xDicList )
{
    if (!xDicList.is())
        return sal_True;

    sal_Bool bRet = sal_True;

    Sequence< Reference< XDictionary >  > aDics( xDicList->getDictionaries() );
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    INT32 nCount = aDics.getLength();
    for (INT32 i = 0;  i < nCount;  i++)
    {
        try
        {
            Reference< XStorable >  xStor( pDic[i], UNO_QUERY );
            if (xStor.is())
            {
                if (!xStor->isReadonly() && xStor->hasLocation())
                    xStor->store();
            }
        }
        catch(...)
        {
            bRet = sal_False;
        }
    }

    return bRet;
}

LanguageType SvxLocaleToLanguage( const Locale& rLocale )
{
    //  empty language -> LANGUAGE_NONE
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_NONE;

    String aLangStr = rLocale.Language;
    String aCtryStr = rLocale.Country;
    //  Variant is ignored

    LanguageType eRet = ConvertIsoNamesToLanguage( aLangStr, aCtryStr );
    if ( eRet == LANGUAGE_SYSTEM )
        eRet = LANGUAGE_NONE;

    return eRet;
}

Locale& SvxLanguageToLocale( Locale& rLocale, LanguageType eLang )
{
    String aLangStr, aCtryStr;
    if ( eLang == LANGUAGE_NONE )
        eLang = LANGUAGE_SYSTEM;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );
    rLocale.Language = aLangStr;
    rLocale.Country  = aCtryStr;

    return rLocale;
}

Locale SvxCreateLocale( LanguageType eLang )
{
    if ( eLang == LANGUAGE_NONE )
        eLang = LANGUAGE_SYSTEM;
    String aLangStr, aCtryStr;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );

    return Locale( aLangStr, aCtryStr, String() );
}


