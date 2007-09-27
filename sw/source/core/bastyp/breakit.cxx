/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakit.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:26:34 $
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

#include "breakit.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _SVX_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif

#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

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
    if ( m_xMSF.is() )
    {
        xBreak = uno::Reference< i18n::XBreakIterator >(
            m_xMSF->createInstance(
                rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) ),
            uno::UNO_QUERY);
    }
}

SwBreakIt::~SwBreakIt()
{
    delete m_pLocale;
    delete m_pForbidden;
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

USHORT SwBreakIt::GetRealScriptOfText( const String& rTxt,
                                        xub_StrLen nPos ) const
{
    USHORT nScript = i18n::ScriptType::WEAK;
    if( xBreak.is() && rTxt.Len() )
    {
        if( nPos && nPos == rTxt.Len() )
            --nPos;
        nScript = xBreak->getScriptType( rTxt, nPos );
        sal_Int32 nChgPos = 0;
        if( i18n::ScriptType::WEAK == nScript && nPos &&
            0 < (nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript )) )
            nScript = xBreak->getScriptType( rTxt, nChgPos-1 );

        if( i18n::ScriptType::WEAK == nScript && rTxt.Len() >
            ( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
            0 <= nChgPos )
            nScript = xBreak->getScriptType( rTxt, nChgPos );
    }
    if( i18n::ScriptType::WEAK == nScript )
        nScript = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
    return nScript;
}

USHORT SwBreakIt::GetAllScriptsOfText( const String& rTxt ) const
{
    const USHORT coAllScripts = ( SCRIPTTYPE_LATIN |
                                  SCRIPTTYPE_ASIAN |
                                  SCRIPTTYPE_COMPLEX );
    USHORT nRet = 0, nScript;
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

