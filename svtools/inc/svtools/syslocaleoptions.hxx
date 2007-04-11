/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syslocaleoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:39:17 $
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

#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#define INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

// bits for broadcasting hints of changes in a SfxSimpleHint, may be combined
const ULONG SYSLOCALEOPTIONS_HINT_LOCALE    = 0x00000001;
const ULONG SYSLOCALEOPTIONS_HINT_CURRENCY  = 0x00000002;

class SvtSysLocaleOptions_Impl;
class SvtListener;
namespace osl { class Mutex; }

class SVT_DLLPUBLIC SvtSysLocaleOptions: public svt::detail::Options
{
    static  SvtSysLocaleOptions_Impl*   pOptions;
    static  sal_Int32                   nRefCount;

    SVT_DLLPRIVATE static  ::osl::Mutex&       GetMutex();

public:

    enum EOption
    {
        E_LOCALE,
        E_CURRENCY
    };
                                SvtSysLocaleOptions();
                                virtual ~SvtSysLocaleOptions();

    // ConfigItem methods

            sal_Bool            IsModified();
            void                Commit();

                                /** Add a listener to react on config changes
                                    which are broadcasted in a SfxSimpleHint
                                    @return
                                        <TRUE/> if added
                                        <FALSE/> if not added
                                 */
            BOOL                AddListener( SvtListener& );
            BOOL                RemoveListener( SvtListener& );

    /** Block broadcasts and accumulate hints. This may be useful if, for
        example, the locale and currency are changed and the currency was
        empty before, since changing the locale with an empty currency does
        also broadcast a change hint for the currency which would result in
        two currency changes broadcasted.

        @param bBlock
            <TRUE/>: broadcasts are blocked until reversed.
            <FALSE/>: broadcasts are not blocked anymore. Pending hints are
            broadcasted if no other instance blocks the broadcast.

        @ATTENTION
        All SvtSysLocaleOptions instances point to exactly one refcounted
        internal representation instance and broadcast blocks are counted.
        Thus if you issue a BlockBroadcasts(TRUE) you MUST issue a matching
        BlockBroadcasts(FALSE) or otherwise pending hints would never be
        broadcasted again.
     */
            void                BlockBroadcasts( BOOL bBlock );

    // config value access methods

    /// The config string may be empty to denote the SYSTEM locale
            const ::rtl::OUString&  GetLocaleConfigString() const;
            void                SetLocaleConfigString( const ::rtl::OUString& rStr );

    /// The config string may be empty to denote the default currency of the locale
            const ::rtl::OUString&  GetCurrencyConfigString() const;
            void                SetCurrencyConfigString( const ::rtl::OUString& rStr );
    // determine whether the decimal separator defined in the keyboard layout is used
    // or the one approriate to the locale
            sal_Bool            IsDecimalSeparatorAsLocale() const;
            void                SetDecimalSeparatorAsLocale( sal_Bool bSet);

    // convenience methods

    /** Get the LanguageType of the current locale, may be LANGUAGE_SYSTEM if
        LocaleConfigString is empty. If you need the real locale used in the
        application, call Application::GetSettings().GetLanguage() instead */
            LanguageType        GetLocaleLanguageType() const;

    /// Get currency abbreviation and locale from an USD-en-US or EUR-de-DE string
    static  void                GetCurrencyAbbrevAndLanguage(
                                    String& rAbbrev,
                                    LanguageType& eLang,
                                    const ::rtl::OUString& rConfigString );

    /// Create an USD-en-US or EUR-de-DE string
    static  ::rtl::OUString     CreateCurrencyConfigString(
                                    const String& rAbbrev,
                                    LanguageType eLang );

            void                GetCurrencyAbbrevAndLanguage(
                                        String& rAbbrev,
                                        LanguageType& eLang ) const
                                    {
                                        GetCurrencyAbbrevAndLanguage( rAbbrev,
                                            eLang, GetCurrencyConfigString() );
                                    }

            void                SetCurrencyAbbrevAndLanguage(
                                        const String& rAbbrev,
                                        LanguageType eLang )
                                    {
                                        SetCurrencyConfigString(
                                            CreateCurrencyConfigString(
                                            rAbbrev, eLang ) );
                                    }

    /** Set a link to a method to be called whenever the default currency
        changes. This can be only one method, and normally it is the static
        link method which calls SvNumberFormatter::SetDefaultSystemCurrency().
        This is needed because the number formatter isn't part of the svl light
        library, otherwise we could call SetDefaultSystemCurrency() directly.
     */
    static  void                SetCurrencyChangeLink( const Link& rLink );
    static  const Link&         GetCurrencyChangeLink();

    /** return the readonly state of the queried option. */
            sal_Bool            IsReadOnly( EOption eOption ) const;
};

#endif  // INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
