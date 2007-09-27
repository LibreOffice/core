/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakit.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:56:19 $
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

#ifndef _BREAKIT_HXX
#define _BREAKIT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HDL_
#include <com/sun/star/i18n/ForbiddenCharacters.hdl>
#endif

class String;

/*************************************************************************
 *                      class SwBreakIt
 *************************************************************************/


class SwBreakIt
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xMSF;

    com::sun::star::lang::Locale * m_pLocale;
    com::sun::star::i18n::ForbiddenCharacters * m_pForbidden;

    LanguageType aLast;          // language of the current locale
    LanguageType aForbiddenLang; // language of the current forbiddenChar struct

    void _GetLocale( const LanguageType aLang );
    void _GetForbidden( const LanguageType  aLang );

    // forbidden and not implemented.
    SwBreakIt();
    SwBreakIt( const SwBreakIt &);
    SwBreakIt & operator= ( const SwBreakIt &);

    // private (see @ _Create, _Delete).
    explicit SwBreakIt(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rxMSF);
    ~SwBreakIt();

public:
    // private (see @ source/core/bastyp/init.cxx).
    static void _Create(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rxMSF);
    static void _Delete();

public:
    static SwBreakIt * Get();

    // @@@ backward compatibility @@@
    com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > xBreak;

    const com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > & GetBreakIter()
    {
        return xBreak;
    }

    const com::sun::star::lang::Locale& GetLocale( const LanguageType aLang )
    {
        if( !m_pLocale || aLast != aLang )
            _GetLocale( aLang );
        return *m_pLocale;
    }

    const com::sun::star::i18n::ForbiddenCharacters& GetForbidden( const LanguageType aLang )
    {
        if( !m_pForbidden || aForbiddenLang != aLang )
            _GetForbidden( aLang );
        return *m_pForbidden;
    }

    USHORT GetRealScriptOfText( const String& rTxt, xub_StrLen nPos ) const;
       USHORT GetAllScriptsOfText( const String& rTxt ) const;
};

#define SW_BREAKITER()  SwBreakIt::Get()
#define SW_XBREAKITER() SW_BREAKITER()->GetBreakIter()

// @@@ backward compatibility @@@
extern SwBreakIt* pBreakIt;

#endif

