/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acceleratorinfo.hxx,v $
 * $Revision: 1.6 $
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

#ifndef __MISC_HELPER_HXX_
#define __MISC_HELPER_HXX_

#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <i18npool/lang.h>
#include <svtools/languageoptions.hxx>

namespace framework
{

inline bool IsScriptTypeMatchingToLanguage( sal_Int16 nScriptType, LanguageType nLang )
{
    return 0 != (nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

    
class LanguageGuessingHelper
{
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing >    m_xLanguageGuesser;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
    
public:
    LanguageGuessingHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceManager) : m_xServiceManager(_xServiceManager){}

    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing >  GetGuesser() const;
};

::rtl::OUString RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xServiceFactory
            ,::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >&        _xUICommandLabels
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame
            ,::rtl::OUString& _rModuleIdentifier
            ,sal_Bool& _rIni
            ,const sal_Char* _pName);

} // namespace framework

#endif // __MISC_HELPER_HXX_

