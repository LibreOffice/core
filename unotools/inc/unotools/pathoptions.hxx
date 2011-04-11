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
#ifndef INCLUDED_unotools_PATHOPTIONS_HXX
#define INCLUDED_unotools_PATHOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <tools/string.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unotools/options.hxx>

#define SVT_SEARCHPATH_DELIMITER      ';'

// class SvtPathOptions --------------------------------------------------

class SvtPathOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtPathOptions: public utl::detail::Options
{
private:
    SvtPathOptions_Impl*    pImp;

public:
    enum Pathes
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

    // get the pathes, not const because of using a mutex
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

    sal_Bool            IsPathReadonly(Pathes ePath)const;
    const String&   GetPath(Pathes ePath) const;

    // set the pathes
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
    void            SetPath( SvtPathOptions::Pathes ePath, const String& rNewPath );

    String          SubstituteVariable( const String& rVar ) const;
    String          ExpandMacros( const String& rPath ) const;
    String          UseVariable( const String& rVar ) const;
    sal_Bool        SearchFile( String& rIniFile, Pathes ePath = PATH_USERCONFIG );
    ::com::sun::star::lang::Locale GetLocale() const;
    sal_Bool        IsReadonly() const;
};

#endif // #ifndef INCLUDED_unotools_PATHOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
