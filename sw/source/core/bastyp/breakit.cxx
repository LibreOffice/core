/*************************************************************************
 *
 *  $RCSfile: breakit.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 13:41:31 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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

#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;


SwBreakIt::SwBreakIt()
    : pLocale( NULL ), pForbidden( NULL )
{
    _GetLocale( (LanguageType)GetAppLanguage() );
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
    if ( xI.is() )
    {
        Any x = xI->queryInterface( ::getCppuType((const Reference< XBreakIterator >*)0) );
        x >>= xBreak;
    }
}

void SwBreakIt::_GetLocale( const LanguageType aLang )
{
    aLast = aLang;
    delete pLocale;
    pLocale = new Locale( SvxCreateLocale( aLast ) );
}

void SwBreakIt::_GetForbidden( const LanguageType aLang )
{
    aForbiddenLang = aLang;
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    LocaleDataWrapper aWrap( xMSF, GetLocale( aLang ) );
    delete pForbidden;
    pForbidden = new ForbiddenCharacters( aWrap.getForbiddenCharacters() );
}

USHORT SwBreakIt::GetRealScriptOfText( const String& rTxt,
                                        xub_StrLen nPos ) const
{
    USHORT nScript = ScriptType::WEAK;
    if( xBreak.is() && rTxt.Len() )
    {
        if( nPos && nPos == rTxt.Len() )
            --nPos;
        nScript = xBreak->getScriptType( rTxt, nPos );
        sal_Int32 nChgPos;
        if( ScriptType::WEAK == nScript && nPos &&
            0 < (nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript )) )
            nScript = xBreak->getScriptType( rTxt, nChgPos-1 );

        if( ScriptType::WEAK == nScript && rTxt.Len() >
            ( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
            0 <= nChgPos )
            nScript = xBreak->getScriptType( rTxt, nChgPos );
    }
    if( ScriptType::WEAK == nScript )
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
                n = xBreak->endOfScript( rTxt, n, nScript ) )
        {
            switch( nScript = xBreak->getScriptType( rTxt, n ) )
            {
            case ScriptType::LATIN:     nRet |= SCRIPTTYPE_LATIN;   break;
            case ScriptType::ASIAN:     nRet |= SCRIPTTYPE_ASIAN;   break;
            case ScriptType::COMPLEX:   nRet |= SCRIPTTYPE_COMPLEX; break;
            case ScriptType::WEAK:
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

