/*************************************************************************
 *
 *  $RCSfile: breakit.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-23 08:28:06 $
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

#ifndef _BREAKIT_HXX
#define _BREAKIT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
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

namespace css = com::sun::star;

class SwBreakIt
{
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xMSF;

    css::lang::Locale * m_pLocale;
    css::i18n::ForbiddenCharacters * m_pForbidden;

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
        const css::uno::Reference< css::lang::XMultiServiceFactory > & rxMSF);
    ~SwBreakIt();

public:
    // private (see @ source/core/bastyp/init.cxx).
    static void _Create(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & rxMSF);
    static void _Delete();

public:
    static SwBreakIt * Get();

    // @@@ backward compatibility @@@
    css::uno::Reference< css::i18n::XBreakIterator > xBreak;

    const css::uno::Reference< css::i18n::XBreakIterator > & GetBreakIter()
    {
        return xBreak;
    }

    const css::lang::Locale& GetLocale( const LanguageType aLang )
    {
        if( !m_pLocale || aLast != aLang )
            _GetLocale( aLang );
        return *m_pLocale;
    }

    const css::i18n::ForbiddenCharacters& GetForbidden( const LanguageType aLang )
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

