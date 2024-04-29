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
#ifndef INCLUDED_UNOTOOLS_PATHOPTIONS_HXX
#define INCLUDED_UNOTOOLS_PATHOPTIONS_HXX

#include <rtl/ustring.hxx>
#include <unotools/unotoolsdllapi.h>
#include <unotools/options.hxx>

#define SVT_SEARCHPATH_DELIMITER      ';'

class SvtPathOptions_Impl;

/*! Handle various defined paths

  All path functions return URLs!
*/
class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtPathOptions final : public utl::detail::Options
{
private:
    std::shared_ptr<SvtPathOptions_Impl>    pImpl;

public:

    enum class Paths : sal_uInt16
    {
        AddIn,
        AutoCorrect,
        AutoText,
        Backup,
        Basic,
        Bitmap,
        Config,
        Dictionary,
        Favorites,
        Filter,
        Gallery,
        Graphic,
        Help,
        IconSet,
        Linguistic,
        Module,
        Palette,
        Plugin,
        Storage,
        Temp,
        Template,
        UserConfig,
        Work,
        Classification,
        UIConfig,
        Fingerprint,
        NumberText,
        LAST // should always be the last element
    };


    SvtPathOptions();
    virtual ~SvtPathOptions() override;

    // get the paths, not const because of using a mutex
    const OUString& GetAddinPath() const;
    const OUString& GetAutoCorrectPath() const;
    const OUString& GetAutoTextPath() const;
    const OUString& GetBackupPath() const;
    const OUString& GetBasicPath() const;
    SAL_DLLPRIVATE const OUString& GetBitmapPath() const;
    const OUString& GetConfigPath() const;
    SAL_DLLPRIVATE const OUString& GetDictionaryPath() const;
    SAL_DLLPRIVATE const OUString& GetFavoritesPath() const;
    SAL_DLLPRIVATE const OUString& GetFilterPath() const;
    const OUString& GetGalleryPath() const;
    const OUString& GetGraphicPath() const;
    const OUString& GetHelpPath() const;
    SAL_DLLPRIVATE const OUString& GetLinguisticPath() const;
    SAL_DLLPRIVATE const OUString& GetModulePath() const;
    const OUString& GetIconsetPath() const;
    const OUString& GetPalettePath() const;
    SAL_DLLPRIVATE const OUString& GetPluginPath() const;
    const OUString& GetStoragePath() const;
    const OUString& GetTempPath() const;
    const OUString& GetTemplatePath() const;
    const OUString& GetUserConfigPath() const;
    const OUString& GetWorkPath() const;
    const OUString& GetFingerprintPath() const;
    const OUString& GetNumbertextPath() const;
    const OUString& GetClassificationPath() const;

    // set the paths
    void            SetAutoTextPath( const OUString& rPath );
    void            SetBasicPath( const OUString& rPath );
    void            SetTempPath( const OUString& rPath );
    void            SetWorkPath( const OUString& rPath );

    OUString        SubstituteVariable( const OUString& rVar ) const;
    OUString        ExpandMacros( const OUString& rPath ) const;
    SAL_DLLPRIVATE OUString UseVariable( const OUString& rVar ) const;
    bool            SearchFile( OUString& rIniFile, Paths ePath = Paths::UserConfig );
};

#endif // INCLUDED_UNOTOOLS_PATHOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
