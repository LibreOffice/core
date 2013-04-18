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
#ifndef INCLUDED_unotools_PATHOPTIONS_HXX
#define INCLUDED_unotools_PATHOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <tools/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/options.hxx>

#define SVT_SEARCHPATH_DELIMITER      ';'

// class SvtPathOptions --------------------------------------------------

class SvtPathOptions_Impl;
class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtPathOptions : public utl::detail::Options
{
private:
    SvtPathOptions_Impl*    pImp;

public:
    enum Paths
    {
        PATH_ADDIN,
        PATH_AUTOCORRECT,
        PATH_AUTOTEXT,
        PATH_BACKUP,
        PATH_BASIC,
        PATH_BITMAP,
        PATH_CONFIG,
        PATH_DICTIONARY,
        PATH_FAVORITES,
        PATH_FILTER,
        PATH_GALLERY,
        PATH_GRAPHIC,
        PATH_HELP,
        PATH_LINGUISTIC,
        PATH_MODULE,
        PATH_PALETTE,
        PATH_PLUGIN,
        PATH_STORAGE,
        PATH_TEMP,
        PATH_TEMPLATE,
        PATH_USERCONFIG,
        PATH_WORK,
        PATH_UICONFIG,
        PATH_FINGERPRINT,
        PATH_COUNT // should always be the last element
    };

    SvtPathOptions();
    virtual ~SvtPathOptions();

    // get the paths, not const because of using a mutex
    const String&   GetAddinPath() const;
    const String&   GetAutoCorrectPath() const;
    const String&   GetAutoTextPath() const;
    const String&   GetBackupPath() const;
    const String&   GetBasicPath() const;
    const String&   GetBitmapPath() const;
    const String&   GetConfigPath() const;
    const String&   GetDictionaryPath() const;
    const String&   GetFavoritesPath() const;
    const String&   GetFilterPath() const;
    const String&   GetGalleryPath() const;
    const String&   GetGraphicPath() const;
    const String&   GetHelpPath() const;
    const String&   GetLinguisticPath() const;
    const String&   GetModulePath() const;
    const String&   GetPalettePath() const;
    const String&   GetPluginPath() const;
    const String&   GetStoragePath() const;
    const String&   GetTempPath() const;
    const String&   GetTemplatePath() const;
    const String&   GetUserConfigPath() const;
    const String&   GetWorkPath() const;
    const String&   GetUIConfigPath() const;
    const String&   GetFingerprintPath() const;

    // set the paths
    void            SetAddinPath( const String& rPath );
    void            SetAutoCorrectPath( const String& rPath );
    void            SetAutoTextPath( const String& rPath );
    void            SetBackupPath( const String& rPath );
    void            SetBasicPath( const String& rPath );
    void            SetBitmapPath( const String& rPath );
    void            SetConfigPath( const String& rPath );
    void            SetDictionaryPath( const String& rPath );
    void            SetFavoritesPath( const String& rPath );
    void            SetFilterPath( const String& rPath );
    void            SetGalleryPath( const String& rPath );
    void            SetGraphicPath( const String& rPath );
    void            SetHelpPath( const String& rPath );
    void            SetLinguisticPath( const String& rPath );
    void            SetModulePath( const String& rPath );
    void            SetPalettePath( const String& rPath );
    void            SetPluginPath( const String& rPath );
    void            SetStoragePath( const String& rPath );
    void            SetTempPath( const String& rPath );
    void            SetTemplatePath( const String& rPath );
    void            SetUserConfigPath( const String& rPath );
    void            SetWorkPath( const String& rPath );

    String          SubstituteVariable( const String& rVar ) const;
    String          ExpandMacros( const String& rPath ) const;
    String          UseVariable( const String& rVar ) const;
    sal_Bool        SearchFile( String& rIniFile, Paths ePath = PATH_USERCONFIG );
    const LanguageTag& GetLanguageTag() const;
    sal_Bool        IsReadonly() const;
};

#endif // #ifndef INCLUDED_unotools_PATHOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
