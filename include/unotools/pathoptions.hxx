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
    const OUString& GetAddinPath() const;
    const OUString& GetAutoCorrectPath() const;
    const OUString& GetAutoTextPath() const;
    const OUString& GetBackupPath() const;
    const OUString& GetBasicPath() const;
    const OUString& GetBitmapPath() const;
    const OUString& GetConfigPath() const;
    const OUString& GetDictionaryPath() const;
    const OUString& GetFavoritesPath() const;
    const OUString& GetFilterPath() const;
    const OUString& GetGalleryPath() const;
    const OUString& GetGraphicPath() const;
    const OUString& GetHelpPath() const;
    const OUString& GetLinguisticPath() const;
    const OUString& GetModulePath() const;
    const OUString& GetPalettePath() const;
    const OUString& GetPluginPath() const;
    const OUString& GetStoragePath() const;
    const OUString& GetTempPath() const;
    const OUString& GetTemplatePath() const;
    const OUString& GetUserConfigPath() const;
    const OUString& GetWorkPath() const;
    const OUString& GetUIConfigPath() const;
    const OUString& GetFingerprintPath() const;

    // set the paths
    void            SetAddinPath( const OUString& rPath );
    void            SetAutoCorrectPath( const OUString& rPath );
    void            SetAutoTextPath( const OUString& rPath );
    void            SetBackupPath( const OUString& rPath );
    void            SetBasicPath( const OUString& rPath );
    void            SetBitmapPath( const OUString& rPath );
    void            SetConfigPath( const OUString& rPath );
    void            SetDictionaryPath( const OUString& rPath );
    void            SetFavoritesPath( const OUString& rPath );
    void            SetFilterPath( const OUString& rPath );
    void            SetGalleryPath( const OUString& rPath );
    void            SetGraphicPath( const OUString& rPath );
    void            SetHelpPath( const OUString& rPath );
    void            SetLinguisticPath( const OUString& rPath );
    void            SetModulePath( const OUString& rPath );
    void            SetPalettePath( const OUString& rPath );
    void            SetPluginPath( const OUString& rPath );
    void            SetStoragePath( const OUString& rPath );
    void            SetTempPath( const OUString& rPath );
    void            SetTemplatePath( const OUString& rPath );
    void            SetUserConfigPath( const OUString& rPath );
    void            SetWorkPath( const OUString& rPath );

    OUString        SubstituteVariable( const OUString& rVar ) const;
    OUString        ExpandMacros( const OUString& rPath ) const;
    OUString        UseVariable( const OUString& rVar ) const;
    bool            SearchFile( OUString& rIniFile, Paths ePath = PATH_USERCONFIG );
    const LanguageTag& GetLanguageTag() const;
    bool            IsReadonly() const;
};

#endif // #ifndef INCLUDED_unotools_PATHOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
