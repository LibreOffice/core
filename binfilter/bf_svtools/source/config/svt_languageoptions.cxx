/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <bf_svtools/languageoptions.hxx>

#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <bf_svtools/cjkoptions.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <bf_svtools/ctloptions.hxx>
#endif
#ifndef _SV_SVAPP_HXX 
#include <vcl/svapp.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ 
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

using namespace ::com::sun::star;
// global ----------------------------------------------------------------------

namespace { struct ALMutex : public rtl::Static< ::osl::Mutex, ALMutex > {}; }

namespace binfilter
{

// -----------------------------------------------------------------------------
// returns for a language the scripttype
sal_uInt16 SvtLanguageOptions::GetScriptTypeOfLanguage( sal_uInt16 nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if( LANGUAGE_SYSTEM == nLang  )
        nLang = Application::GetSettings().GetLanguage();

    sal_Int16 nScriptType = MsLangId::getScriptType( nLang );
    USHORT nScript;
    switch (nScriptType)
    {
        case ::com::sun::star::i18n::ScriptType::ASIAN:
            nScript = SCRIPTTYPE_ASIAN;
            break;
        case ::com::sun::star::i18n::ScriptType::COMPLEX:
            nScript = SCRIPTTYPE_COMPLEX;
            break;
        default:
            nScript = SCRIPTTYPE_LATIN;
    }
    return nScript;
}
// -----------------------------------------------------------------------------


/*-- 27.10.2005 08:18:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SvtSystemLanguageOptions::SvtSystemLanguageOptions() : 
    utl::ConfigItem( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("System/L10N") ))
{
    uno::Sequence< rtl::OUString > aPropertyNames(1);
    rtl::OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SystemLocale"));
    uno::Sequence< uno::Any > aValues = GetProperties( aPropertyNames );

    if ( aValues.getLength() )
    {
        aValues[0]>>= m_sWin16SystemLocale;
    }
}
/*-- 27.10.2005 08:18:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SvtSystemLanguageOptions::~SvtSystemLanguageOptions()
{
}
/*-- 27.10.2005 08:18:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvtSystemLanguageOptions::Commit()
{
    //does nothing
}
/*-- 27.10.2005 08:36:14---------------------------------------------------

  -----------------------------------------------------------------------*/
LanguageType SvtSystemLanguageOptions::GetWin16SystemLanguage()
{
    if( m_sWin16SystemLocale.getLength() == 0 )
        return LANGUAGE_NONE;
    return MsLangId::convertIsoStringToLanguage( m_sWin16SystemLocale );
}

    void SvtSystemLanguageOptions::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

}
