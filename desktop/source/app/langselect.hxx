/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <list>
#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svl/languageoptions.hxx>

namespace desktop
{

class LanguageSelection
{
public:
    enum LanguageSelectionStatus
    {
        LS_STATUS_OK,
        LS_STATUS_CANNOT_DETERMINE_LANGUAGE,
        LS_STATUS_CONFIGURATIONACCESS_BROKEN
    };

    static com::sun::star::lang::Locale IsoStringToLocale(const rtl::OUString& str);
    static rtl::OUString getLanguageString();
    static bool prepareLanguage();
    static LanguageSelectionStatus getStatus();

private:
    static const rtl::OUString usFallbackLanguage;
    static rtl::OUString aFoundLanguage;
    static sal_Bool bFoundLanguage;
    static LanguageSelectionStatus m_eStatus;

    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
        getConfigAccess(const sal_Char* pPath, sal_Bool bUpdate=sal_False);
    static com::sun::star::uno::Sequence< rtl::OUString > getInstalledLanguages();
    static sal_Bool isInstalledLanguage(rtl::OUString& usLocale, sal_Bool bExact=sal_False);
    static rtl::OUString getFirstInstalledLanguage();
    static rtl::OUString getUserUILanguage();
    static rtl::OUString getUserLanguage();
    static rtl::OUString getSystemLanguage();
    static void resetUserLanguage();
    static void setDefaultLanguage(const rtl::OUString&);
};

} //namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
