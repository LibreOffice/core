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
