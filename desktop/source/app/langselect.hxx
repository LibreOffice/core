/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <list>
#include <sal/types.h>
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

    static OUString getLanguageString();
    static bool prepareLanguage();
    static LanguageSelectionStatus getStatus();

private:
    static OUString aFoundLanguage;
    static sal_Bool bFoundLanguage;
    static LanguageSelectionStatus m_eStatus;

    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
        getConfigAccess(const sal_Char* pPath, sal_Bool bUpdate=sal_False);
    static com::sun::star::uno::Sequence< OUString > getInstalledLanguages();
    static sal_Bool isInstalledLanguage(OUString& usLocale, sal_Bool bExact=sal_False);
    static OUString getFirstInstalledLanguage();
    static OUString getUserUILanguage();
    static OUString getUserLanguage();
    static OUString getSystemLanguage();
    static void resetUserLanguage();
    static void setDefaultLanguage(const OUString&);
};

} //namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
