/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pathoptions.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:45:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#define INCLUDED_SVTOOLS_PATHOPTIONS_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

#define SVT_SEARCHPATH_DELIMITER      ';'

// class SvtPathOptions --------------------------------------------------

class SvtPathOptions_Impl;
class SVL_DLLPUBLIC SvtPathOptions: public svt::detail::Options
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

    BOOL            IsPathReadonly(Pathes ePath)const;
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

    String          SubstituteVariable( const String& rVar );
    String          UseVariable( const String& rVar );
    sal_Bool        SearchFile( String& rIniFile, Pathes ePath = PATH_USERCONFIG );
    ::com::sun::star::lang::Locale GetLocale() const;
    sal_Bool        IsReadonly() const;
};

#endif // #ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX

