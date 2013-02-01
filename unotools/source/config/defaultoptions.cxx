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


#include <unotools/defaultoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>

#include <unotools/localfilehelper.hxx>
#include <rtl/instance.hxx>

#include <itemholder1.hxx>

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

// define ----------------------------------------------------------------

#define DEFAULTPATH__ADDIN          0
#define DEFAULTPATH__AUTOCORRECT    1
#define DEFAULTPATH__AUTOTEXT       2
#define DEFAULTPATH__BACKUP         3
#define DEFAULTPATH__BASIC          4
#define DEFAULTPATH__BITMAP         5
#define DEFAULTPATH__CONFIG         6
#define DEFAULTPATH__DICTIONARY     7
#define DEFAULTPATH__FAVORITES      8
#define DEFAULTPATH__FILTER         9
#define DEFAULTPATH__GALLERY        10
#define DEFAULTPATH__GRAPHIC        11
#define DEFAULTPATH__HELP           12
#define DEFAULTPATH__LINGUISTIC     13
#define DEFAULTPATH__MODULE         14
#define DEFAULTPATH__PALETTE        15
#define DEFAULTPATH__PLUGIN         16
#define DEFAULTPATH__TEMP           17
#define DEFAULTPATH__TEMPLATE       18
#define DEFAULTPATH__USERCONFIG     19
#define DEFAULTPATH__WORK           20
#define DEFAULTPATH__USERDICTIONARY 21

// class SvtDefaultOptions_Impl ------------------------------------------

class SvtDefaultOptions_Impl : public utl::ConfigItem
{
public:
    String          m_aAddinPath;
    String          m_aAutoCorrectPath;
    String          m_aAutoTextPath;
    String          m_aBackupPath;
    String          m_aBasicPath;
    String          m_aBitmapPath;
    String          m_aConfigPath;
    String          m_aDictionaryPath;
    String          m_aFavoritesPath;
    String          m_aFilterPath;
    String          m_aGalleryPath;
    String          m_aGraphicPath;
    String          m_aHelpPath;
    String          m_aLinguisticPath;
    String          m_aModulePath;
    String          m_aPalettePath;
    String          m_aPluginPath;
    String          m_aTempPath;
    String          m_aTemplatePath;
    String          m_aUserConfigPath;
    String          m_aWorkPath;
    String          m_aUserDictionaryPath;

                    SvtDefaultOptions_Impl();

    String          GetDefaultPath( sal_uInt16 nId ) const;
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
};

// global ----------------------------------------------------------------

static SvtDefaultOptions_Impl*  pOptions = NULL;
static sal_Int32                nRefCount = 0;

typedef String SvtDefaultOptions_Impl:: *PathStrPtr;

struct PathToDefaultMapping_Impl
{
    SvtPathOptions::Paths   _ePath;
    PathStrPtr              _pDefaultPath;
};

static PathToDefaultMapping_Impl const PathMap_Impl[] =
{
    { SvtPathOptions::PATH_ADDIN,           &SvtDefaultOptions_Impl::m_aAddinPath },
    { SvtPathOptions::PATH_AUTOCORRECT,     &SvtDefaultOptions_Impl::m_aAutoCorrectPath },
    { SvtPathOptions::PATH_AUTOTEXT,        &SvtDefaultOptions_Impl::m_aAutoTextPath },
    { SvtPathOptions::PATH_BACKUP,          &SvtDefaultOptions_Impl::m_aBackupPath },
    { SvtPathOptions::PATH_BASIC,           &SvtDefaultOptions_Impl::m_aBasicPath },
    { SvtPathOptions::PATH_BITMAP,          &SvtDefaultOptions_Impl::m_aBitmapPath },
    { SvtPathOptions::PATH_CONFIG,          &SvtDefaultOptions_Impl::m_aConfigPath },
    { SvtPathOptions::PATH_DICTIONARY,      &SvtDefaultOptions_Impl::m_aDictionaryPath },
    { SvtPathOptions::PATH_FAVORITES,       &SvtDefaultOptions_Impl::m_aFavoritesPath },
    { SvtPathOptions::PATH_FILTER,          &SvtDefaultOptions_Impl::m_aFilterPath },
    { SvtPathOptions::PATH_GALLERY,         &SvtDefaultOptions_Impl::m_aGalleryPath },
    { SvtPathOptions::PATH_GRAPHIC,         &SvtDefaultOptions_Impl::m_aGraphicPath },
    { SvtPathOptions::PATH_HELP,            &SvtDefaultOptions_Impl::m_aHelpPath },
    { SvtPathOptions::PATH_LINGUISTIC,      &SvtDefaultOptions_Impl::m_aLinguisticPath },
    { SvtPathOptions::PATH_MODULE,          &SvtDefaultOptions_Impl::m_aModulePath },
    { SvtPathOptions::PATH_PALETTE,         &SvtDefaultOptions_Impl::m_aPalettePath },
    { SvtPathOptions::PATH_PLUGIN,          &SvtDefaultOptions_Impl::m_aPluginPath },
    { SvtPathOptions::PATH_TEMP,            &SvtDefaultOptions_Impl::m_aTempPath },
    { SvtPathOptions::PATH_TEMPLATE,        &SvtDefaultOptions_Impl::m_aTemplatePath },
    { SvtPathOptions::PATH_USERCONFIG,      &SvtDefaultOptions_Impl::m_aUserConfigPath },
    { SvtPathOptions::PATH_WORK,            &SvtDefaultOptions_Impl::m_aWorkPath }
};

// functions -------------------------------------------------------------

Sequence< OUString > GetDefaultPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Addin",            // PATH_ADDIN
        "AutoCorrect",      // PATH_AUTOCORRECT
        "AutoText",         // PATH_AUTOTEXT
        "Backup",           // PATH_BACKUP
        "Basic",            // PATH_BASIC
        "Bitmap",           // PATH_BITMAP
        "Config",           // PATH_CONFIG
        "Dictionary",       // PATH_DICTIONARY
        "Favorite",         // PATH_FAVORITES
        "Filter",           // PATH_FILTER
        "Gallery",          // PATH_GALLERY
        "Graphic",          // PATH_GRAPHIC
        "Help",             // PATH_HELP
        "Linguistic",       // PATH_LINGUISTIC
        "Module",           // PATH_MODULE
        "Palette",          // PATH_PALETTE
        "Plugin",           // PATH_PLUGIN
        "Temp",             // PATH_TEMP
        "Template",         // PATH_TEMPLATE
        "UserConfig",       // PATH_USERCONFIG
        "Work"              // PATH_WORK
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

void SvtDefaultOptions_Impl::Notify( const Sequence< rtl::OUString >&  )
{
    // no notification, will never be changed
}

void SvtDefaultOptions_Impl::Commit()
{
    // will never be changed
}

// class SvtDefaultOptions_Impl ------------------------------------------

String SvtDefaultOptions_Impl::GetDefaultPath( sal_uInt16 nId ) const
{
    rtl::OUString aRet;
    sal_uInt16 nIdx = 0;

    while ( PathMap_Impl[nIdx]._ePath <= SvtPathOptions::PATH_WORK )
    {
        if ( nId == PathMap_Impl[nIdx]._ePath && PathMap_Impl[nIdx]._pDefaultPath )
        {
            aRet = this->*(PathMap_Impl[nIdx]._pDefaultPath);
            if ( nId == SvtPathOptions::PATH_ADDIN ||
                 nId == SvtPathOptions::PATH_FILTER ||
                 nId == SvtPathOptions::PATH_HELP ||
                 nId == SvtPathOptions::PATH_MODULE ||
                 nId == SvtPathOptions::PATH_PLUGIN )
            {
                rtl::OUString aTmp;
                ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aRet, aTmp );
                aRet = aTmp;
            }

            break;
        }
        ++nIdx;
    }

    return aRet;
}

// -----------------------------------------------------------------------

SvtDefaultOptions_Impl::SvtDefaultOptions_Impl() : ConfigItem( "Office.Common/Path/Default" )
{
    Sequence< OUString > aNames = GetDefaultPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        SvtPathOptions aPathOpt;
        OUString aTempStr, aFullPath;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                switch ( pValues[nProp].getValueTypeClass() )
                {
                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        // multi paths
                        if ( pValues[nProp] >>= aTempStr )
                            aFullPath = aPathOpt.SubstituteVariable( aTempStr );
                        else
                        {
                            SAL_WARN( "unotools.config", "any operator >>= failed" );
                        }
                        break;
                    }

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    {
                        // single paths
                        aFullPath = OUString();
                        Sequence < OUString > aList;
                        if ( pValues[nProp] >>= aList )
                        {
                            sal_Int32 nCount = aList.getLength();
                            for ( sal_Int32 nPosition = 0; nPosition < nCount; ++nPosition )
                            {
                                aTempStr = aPathOpt.SubstituteVariable( aList[ nPosition ] );
                                aFullPath += aTempStr;
                                if ( nPosition < nCount-1 )
                                    aFullPath += OUString( RTL_CONSTASCII_USTRINGPARAM(";") );
                            }
                        }
                        else
                        {
                            SAL_WARN( "unotools.config", "any operator >>= failed" );
                        }
                        break;
                    }

                    default:
                    {
                        SAL_WARN( "unotools.config", "Wrong any type" );
                    }
                }

                switch ( nProp )
                {
                    case DEFAULTPATH__ADDIN:            m_aAddinPath = String( aFullPath );         break;
                    case DEFAULTPATH__AUTOCORRECT:      m_aAutoCorrectPath = String( aFullPath );   break;
                    case DEFAULTPATH__AUTOTEXT:         m_aAutoTextPath = String( aFullPath );      break;
                    case DEFAULTPATH__BACKUP:           m_aBackupPath = String( aFullPath );        break;
                    case DEFAULTPATH__BASIC:            m_aBasicPath = String( aFullPath );         break;
                    case DEFAULTPATH__BITMAP:           m_aBitmapPath = String( aFullPath );        break;
                    case DEFAULTPATH__CONFIG:           m_aConfigPath = String( aFullPath );        break;
                    case DEFAULTPATH__DICTIONARY:       m_aDictionaryPath = String( aFullPath );    break;
                    case DEFAULTPATH__FAVORITES:        m_aFavoritesPath = String( aFullPath );     break;
                    case DEFAULTPATH__FILTER:           m_aFilterPath = String( aFullPath );        break;
                    case DEFAULTPATH__GALLERY:          m_aGalleryPath = String( aFullPath );       break;
                    case DEFAULTPATH__GRAPHIC:          m_aGraphicPath = String( aFullPath );       break;
                    case DEFAULTPATH__HELP:             m_aHelpPath = String( aFullPath );          break;
                    case DEFAULTPATH__LINGUISTIC:       m_aLinguisticPath = String( aFullPath );    break;
                    case DEFAULTPATH__MODULE:           m_aModulePath = String( aFullPath );        break;
                    case DEFAULTPATH__PALETTE:          m_aPalettePath = String( aFullPath );       break;
                    case DEFAULTPATH__PLUGIN:           m_aPluginPath = String( aFullPath );        break;
                    case DEFAULTPATH__TEMP:             m_aTempPath = String( aFullPath );          break;
                    case DEFAULTPATH__TEMPLATE:         m_aTemplatePath = String( aFullPath );      break;
                    case DEFAULTPATH__USERCONFIG:       m_aUserConfigPath = String( aFullPath );    break;
                    case DEFAULTPATH__WORK:             m_aWorkPath = String( aFullPath );          break;
                    case DEFAULTPATH__USERDICTIONARY:   m_aUserDictionaryPath = String( aFullPath );break;

                    default:
                        SAL_WARN( "unotools.config", "invalid index to load a default path" );
                }
            }
        }
    }
}

// class SvtDefaultOptions -----------------------------------------------
namespace { struct lclMutex : public rtl::Static< ::osl::Mutex, lclMutex > {}; }

SvtDefaultOptions::SvtDefaultOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    if ( !pOptions )
    {
        pOptions = new SvtDefaultOptions_Impl;
        ItemHolder1::holdConfigItem(E_DEFAULTOPTIONS);
    }
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtDefaultOptions::~SvtDefaultOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

String SvtDefaultOptions::GetDefaultPath( sal_uInt16 nId ) const
{
    return pImp->GetDefaultPath( nId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
