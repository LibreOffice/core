/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intlwrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:32:25 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#define _UNOTOOLS_INTLWRAPPER_HXX

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif


/**
    A wrapper of I18N wrappers. Using this is more expensive than using some
    single wrapper classes so use it only if you must pass a single pointer
    without knowing in advance what is needed, e.g. for
    SfxPoolItem::GetPresentation(). Remember that this wrapper was only created
    for convenience to bypass some oddities, if possible don't use it. <p>

    Implemented are only the const get...() methods of the wrappers, which are
    loaded on demand, for consistency reasons no change of locale is possible.
    Only default calendar and default collator are supported. <p>

    One exception though is the calendar wrapper: to be able to set a value and
    retrieve calendar values it is not const, so methods using this should
    reset the calendar to the previous value if it isn't sure where the
    IntlWrapper did come from. <p>
 */
class UNOTOOLS_DLLPUBLIC IntlWrapper
{
private:

    ::com::sun::star::lang::Locale  aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;

            CharClass*          pCharClass;
            LocaleDataWrapper*  pLocaleData;
            CalendarWrapper*    pCalendar;
            CollatorWrapper*    pCollator;
            CollatorWrapper*    pCaseCollator;

            LanguageType        eLanguage;

            void                ImplNewCharClass() const;
            void                ImplNewLocaleData() const;
            void                ImplNewCalendar() const;
            void                ImplNewCollator( BOOL bCaseSensitive ) const;


public:
                                IntlWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );
                                IntlWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    LanguageType eLang
                                    );
                                ~IntlWrapper();

    LanguageType                getLanguage() const { return eLanguage; }
    const ::com::sun::star::lang::Locale&   getLocale() const { return aLocale; }

    const CharClass*            getCharClass() const
                                    {
                                        if ( !pCharClass )
                                            ImplNewCharClass();
                                        return pCharClass;
                                    }
    const LocaleDataWrapper*    getLocaleData() const
                                    {
                                        if ( !pLocaleData )
                                            ImplNewLocaleData();
                                        return pLocaleData;
                                    }
    CalendarWrapper*            getCalendar() const
                                    {
                                        if ( !pCalendar )
                                            ImplNewCalendar();
                                        return pCalendar;
                                    }
    /// case insensitive collator, simple IGNORE_CASE
    const CollatorWrapper*      getCollator() const
                                    {
                                        if ( !pCollator )
                                            ImplNewCollator( FALSE );
                                        return pCollator;
                                    }
    /// case sensitive collator
    const CollatorWrapper*      getCaseCollator() const
                                    {
                                        if ( !pCaseCollator )
                                            ImplNewCollator( TRUE );
                                        return pCaseCollator;
                                    }
};

#endif // _UNOTOOLS_INTLWRAPPER_HXX
