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

#ifndef INCLUDED_SHELL_SOURCE_BACKENDS_GCONFBE_GCONFACCESS_HXX
#define INCLUDED_SHELL_SOURCE_BACKENDS_GCONFBE_GCONFACCESS_HXX

#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/beans/Optional.hpp"
#include "gconf/gconf-client.h"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace gconfaccess {

enum ConfigurationSetting
{
    SETTING_PROXY_MODE,
    SETTING_PROXY_HTTP_HOST,
    SETTING_PROXY_HTTP_PORT,
    SETTING_PROXY_HTTPS_HOST,
    SETTING_PROXY_HTTPS_PORT,
    SETTING_PROXY_FTP_HOST,
    SETTING_PROXY_FTP_PORT,
    SETTING_NO_PROXY_FOR,
    SETTING_ENABLE_ACCESSIBILITY,
    SETTING_MAILER_PROGRAM,
    SETTING_WORK_DIRECTORY,
    SETTING_SOURCEVIEWFONT_NAME,
    SETTING_SOURCEVIEWFONT_HEIGHT,
    SETTING_USER_GIVENNAME,
    SETTING_USER_SURNAME,

#ifdef ENABLE_LOCKDOWN

    SETTING_DISABLE_PRINTING,
    SETTING_USE_SYSTEM_FILE_DIALOG,
    SETTING_PRINTING_MODIFIES_DOCUMENT,
    SETTING_SHOW_ICONS_IN_MENUS,
    SETTING_SHOW_INACTIVE_MENUITEMS,
    SETTING_SHOW_FONT_PREVIEW,
    SETTING_SHOW_FONT_HISTORY,
    SETTING_ENABLE_OPENGL,
    SETTING_OPTIMIZE_OPENGL,
    SETTING_SAVE_DOCUMENT_WINDOWS,
    SETTING_SAVE_DOCUMENT_VIEW_INFO,
    SETTING_USE_SYSTEM_FONT,
    SETTING_USE_FONT_ANTI_ALIASING,
    SETTING_FONT_ANTI_ALIASING_MIN_PIXEL,
    SETTING_WARN_CREATE_PDF,
    SETTING_WARN_PRINT_DOC,
    SETTING_WARN_SAVEORSEND_DOC,
    SETTING_WARN_SIGN_DOC,
    SETTING_REMOVE_PERSONAL_INFO,
    SETTING_RECOMMEND_PASSWORD,
    SETTING_UNDO_STEPS,
    SETTING_SYMBOL_SET,
    SETTING_MACRO_SECURITY_LEVEL,
    SETTING_CREATE_BACKUP,
    SETTING_WARN_ALIEN_FORMAT,
    SETTING_AUTO_SAVE,
    SETTING_AUTO_SAVE_INTERVAL,
    SETTING_WRITER_DEFAULT_DOC_FORMAT,
    SETTING_IMPRESS_DEFAULT_DOC_FORMAT,
    SETTING_CALC_DEFAULT_DOC_FORMAT,

#endif // ENABLE_LOCKDOWN

    SETTINGS_LAST
};

struct ConfigurationValue
{
    const ConfigurationSetting nSettingId;
    const gchar *GconfItem;
    const char *OOoConfItem;
    const sal_Int32 nOOoConfItemLen;
    const sal_Bool bNeedsTranslation;
    const ConfigurationSetting nDependsOn;
};

extern ConfigurationValue const ConfigurationValues[];

extern std::size_t const nConfigurationValues;

com::sun::star::beans::Optional< com::sun::star::uno::Any > getValue(
    ConfigurationValue const & data);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
