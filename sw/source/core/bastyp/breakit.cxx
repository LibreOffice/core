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
#include "precompiled_sw.hxx"

#include "breakit.hxx"
#include <unicode/uchar.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#include <unotools/localedatawrapper.hxx>

#ifndef _SVX_LINGU_HXX
#include <editeng/unolingu.hxx>
#endif
#include <editeng/scripttypeitem.hxx>
#include "swtypes.hxx"

using namespace com::sun::star;

SwBreakIt * pBreakIt = 0;

void SwBreakIt::_Create(
    const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
{
    delete pBreakIt, pBreakIt = new SwBreakIt( rxMSF );
}

void SwBreakIt::_Delete()
{
    delete pBreakIt, pBreakIt = 0;
}

SwBreakIt * SwBreakIt::Get()
{
    return pBreakIt;
}

SwBreakIt::SwBreakIt(
    const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
    : m_xMSF( rxMSF ),
      m_pLocale( NULL ),
      m_pForbidden( NULL ),
      aLast( LANGUAGE_DONTKNOW ),
      aForbiddenLang( LANGUAGE_DONTKNOW)
{
    DBG_ASSERT( m_xMSF.is(), "SwBreakIt: no MultiServiceFactory" );
    //if ( m_xMSF.is() )
    //{
 //       xBreak = uno::Reference< i18n::XBreakIterator >(
    //      m_xMSF->createInstance(
    //          rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) ),
 //           uno::UNO_QUERY);

 //       xCTLDetect = uno::Reference< i18n::XScriptTypeDetector >(
 //           m_xMSF->createInstance(
 //                rtl::OUString::createFromAscii( "com.sun.star.i18n.ScriptTypeDetector" ) ),
 //           uno::UNO_QUERY);
 //   }
}

SwBreakIt::~SwBreakIt()
{
    delete m_pLocale;
    delete m_pForbidden;
}
void SwBreakIt::createBreakIterator() const
{
    if ( m_xMSF.is() && !xBreak.is() )
        xBreak.set(m_xMSF->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.BreakIterator"))),uno::UNO_QUERY);
}
void SwBreakIt::createScriptTypeDetector()
{
    if ( m_xMSF.is() && !xCTLDetect.is() )
        xCTLDetect.set(m_xMSF->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.ScriptTypeDetector" ))),uno::UNO_QUERY);
}
void SwBreakIt::_GetLocale( const LanguageType aLang )
{
    aLast = aLang;
    delete m_pLocale;
    m_pLocale = new lang::Locale( SvxCreateLocale( aLast ) );
}

void SwBreakIt::_GetForbidden( const LanguageType aLang )
{
    LocaleDataWrapper aWrap( m_xMSF, GetLocale( aLang ) );

    aForbiddenLang = aLang;
    delete m_pForbidden;
    m_pForbidden = new i18n::ForbiddenCharacters( aWrap.getForbiddenCharacters() );
}

sal_uInt16 SwBreakIt::GetRealScriptOfText( const String& rTxt,
                                        xub_StrLen nPos ) const
{
    createBreakIterator();
    sal_uInt16 nScript = i18n::ScriptType::WEAK;
    if( xBreak.is() && rTxt.Len() )
    {
        if( nPos && nPos == rTxt.Len() )
            --nPos;
        nScript = xBreak->getScriptType( rTxt, nPos );
        sal_Int32 nChgPos = 0;
        if ( i18n::ScriptType::WEAK == nScript && nPos + 1 < rTxt.Len() )
        {
            // A weak character followed by a mark may be meant to combine with
            // the mark, so prefer the following character's script
            switch ( u_charType(rTxt.GetChar(nPos + 1) ) ) {
            case U_NON_SPACING_MARK:
            case U_ENCLOSING_MARK:
            case U_COMBINING_SPACING_MARK:
                nScript = xBreak->getScriptType( rTxt, nPos+1 );
                break;
            }
        }
        if( i18n::ScriptType::WEAK == nScript && nPos &&
            0 < (nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript )) )
            nScript = xBreak->getScriptType( rTxt, nChgPos-1 );

        if( i18n::ScriptType::WEAK == nScript && rTxt.Len() >
            ( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
            0 <= nChgPos )
            nScript = xBreak->getScriptType( rTxt, nChgPos );
    }
    if( i18n::ScriptType::WEAK == nScript )
        nScript = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
    return nScript;
}

sal_uInt16 SwBreakIt::GetAllScriptsOfText( const String& rTxt ) const
{
    const sal_uInt16 coAllScripts = ( SCRIPTTYPE_LATIN |
                                  SCRIPTTYPE_ASIAN |
                                  SCRIPTTYPE_COMPLEX );
    createBreakIterator();
    sal_uInt16 nRet = 0, nScript;
    if( !xBreak.is() )
        nRet = coAllScripts;
    else if( rTxt.Len() )
    {
        for( xub_StrLen n = 0, nEnd = rTxt.Len(); n < nEnd;
                n = static_cast<xub_StrLen>(xBreak->endOfScript( rTxt, n, nScript )) )
        {
            switch( nScript = xBreak->getScriptType( rTxt, n ) )
            {
            case i18n::ScriptType::LATIN:       nRet |= SCRIPTTYPE_LATIN;   break;
            case i18n::ScriptType::ASIAN:       nRet |= SCRIPTTYPE_ASIAN;   break;
            case i18n::ScriptType::COMPLEX: nRet |= SCRIPTTYPE_COMPLEX; break;
            case i18n::ScriptType::WEAK:
                    if( !nRet )
                        nRet |= coAllScripts;
                    break;
            }
            if( coAllScripts == nRet )
                break;
        }
    }
    return nRet;
}

