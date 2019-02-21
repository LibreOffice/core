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

#include <sal/config.h>

#include <osl/file.hxx>
#include <sal/log.hxx>
#include <unotools/defaultoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>

#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>

#include "itemholder1.hxx"

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;

#define DEFAULTPATH_ADDIN          0
#define DEFAULTPATH_AUTOCORRECT    1
#define DEFAULTPATH_AUTOTEXT       2
#define DEFAULTPATH_BACKUP         3
#define DEFAULTPATH_BASIC          4
#define DEFAULTPATH_BITMAP         5
#define DEFAULTPATH_CONFIG         6
#define DEFAULTPATH_DICTIONARY     7
#define DEFAULTPATH_FAVORITES      8
#define DEFAULTPATH_FILTER         9
#define DEFAULTPATH_GALLERY        10
#define DEFAULTPATH_GRAPHIC        11
#define DEFAULTPATH_HELP           12
#define DEFAULTPATH_LINGUISTIC     13
#define DEFAULTPATH_MODULE         14
#define DEFAULTPATH_PALETTE        15
#define DEFAULTPATH_PLUGIN         16
#define DEFAULTPATH_TEMP           17
#define DEFAULTPATH_TEMPLATE       18
#define DEFAULTPATH_USERCONFIG     19
#define DEFAULTPATH_WORK           20
#define DEFAULTPATH_CLASSIFICATION 21
#define DEFAULTPATH_USERDICTIONARY 22

// class SvtDefaultOptions_Impl ------------------------------------------

class SvtDefaultOptions_Impl : public utl::ConfigItem
{
public:
    OUString         m_aAddinPath;
    OUString         m_aAutoCorrectPath;
    OUString         m_aAutoTextPath;
    OUString         m_aBackupPath;
    OUString         m_aBasicPath;
    OUString         m_aBitmapPath;
    OUString         m_aConfigPath;
    OUString         m_aDictionaryPath;
    OUString         m_aFavoritesPath;
    OUString         m_aFilterPath;
    OUString         m_aGalleryPath;
    OUString         m_aGraphicPath;
    OUString         m_aHelpPath;
    OUString         m_aLinguisticPath;
    OUString         m_aModulePath;
    OUString         m_aPalettePath;
    OUString         m_aPluginPath;
    OUString         m_aTempPath;
    OUString         m_aTemplatePath;
    OUString         m_aUserConfigPath;
    OUString         m_aWorkPath;
    OUString         m_aClassificationPath;

                    SvtDefaultOptions_Impl();
                    virtual ~SvtDefaultOptions_Impl() override;

    OUString         GetDefaultPath( sal_uInt16 nId ) const;
    virtual void    Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

private:
    virtual void    ImplCommit() final override;
};

// global ----------------------------------------------------------------

namespace {

std::weak_ptr<SvtDefaultOptions_Impl> g_pOptions;

}

typedef OUString SvtDefaultOptions_Impl:: *PathStrPtr;

struct PathToDefaultMapping_Impl
{
    SvtPathOptions::Paths const   _ePath;
    PathStrPtr const              _pDefaultPath;
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
    { SvtPathOptions::PATH_WORK,            &SvtDefaultOptions_Impl::m_aWorkPath },
    { SvtPathOptions::PATH_CLASSIFICATION,  &SvtDefaultOptions_Impl::m_aClassificationPath }
};

// functions -------------------------------------------------------------

static Sequence< OUString > GetDefaultPropertyNames()
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
        "Work",             // PATH_WORK
        "Classification"    // PATH_CLASSIFICATION
    };

    const int nCount = SAL_N_ELEMENTS( aPropNames );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

void SvtDefaultOptions_Impl::Notify( const Sequence< OUString >&  )
{
    // no notification, will never be changed
}

void SvtDefaultOptions_Impl::ImplCommit()
{
    // will never be changed
}

// class SvtDefaultOptions_Impl ------------------------------------------

OUString SvtDefaultOptions_Impl::GetDefaultPath( sal_uInt16 nId ) const
{
    OUString aRet;
    sal_uInt16 nIdx = 0;

    while ( PathMap_Impl[nIdx]._ePath <= SvtPathOptions::PATH_CLASSIFICATION )
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
                OUString aTmp;
                osl::FileBase::getFileURLFromSystemPath( aRet, aTmp );
                aRet = aTmp;
            }

            break;
        }
        ++nIdx;
    }

    return aRet;
}

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
        OUString aTempStr;
        OUStringBuffer aFullPathBuf;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                switch ( pValues[nProp].getValueTypeClass() )
                {
                    case css::uno::TypeClass_STRING :
                    {
                        // multi paths
                        if ( pValues[nProp] >>= aTempStr )
                            aFullPathBuf = aPathOpt.SubstituteVariable( aTempStr );
                        else
                        {
                            SAL_WARN( "unotools.config", "any operator >>= failed" );
                        }
                        break;
                    }

                    case css::uno::TypeClass_SEQUENCE :
                    {
                        // single paths
                        aFullPathBuf.setLength(0);
                        Sequence < OUString > aList;
                        if ( pValues[nProp] >>= aList )
                        {
                            sal_Int32 nCount = aList.getLength();
                            for ( sal_Int32 nPosition = 0; nPosition < nCount; ++nPosition )
                            {
                                aFullPathBuf.append(aPathOpt.SubstituteVariable( aList[ nPosition ] ));
                                if ( nPosition < nCount-1 )
                                    aFullPathBuf.append(";");
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

                auto aFullPath = aFullPathBuf.makeStringAndClear();
                switch ( nProp )
                {
                    case DEFAULTPATH_ADDIN:            m_aAddinPath = aFullPath;         break;
                    case DEFAULTPATH_AUTOCORRECT:      m_aAutoCorrectPath = aFullPath;   break;
                    case DEFAULTPATH_AUTOTEXT:         m_aAutoTextPath = aFullPath;      break;
                    case DEFAULTPATH_BACKUP:           m_aBackupPath = aFullPath;        break;
                    case DEFAULTPATH_BASIC:            m_aBasicPath = aFullPath;         break;
                    case DEFAULTPATH_BITMAP:           m_aBitmapPath = aFullPath;        break;
                    case DEFAULTPATH_CONFIG:           m_aConfigPath = aFullPath;        break;
                    case DEFAULTPATH_DICTIONARY:       m_aDictionaryPath = aFullPath;    break;
                    case DEFAULTPATH_FAVORITES:        m_aFavoritesPath = aFullPath;     break;
                    case DEFAULTPATH_FILTER:           m_aFilterPath = aFullPath;        break;
                    case DEFAULTPATH_GALLERY:          m_aGalleryPath = aFullPath;       break;
                    case DEFAULTPATH_GRAPHIC:          m_aGraphicPath = aFullPath;       break;
                    case DEFAULTPATH_HELP:             m_aHelpPath = aFullPath;          break;
                    case DEFAULTPATH_LINGUISTIC:       m_aLinguisticPath = aFullPath;    break;
                    case DEFAULTPATH_MODULE:           m_aModulePath = aFullPath;        break;
                    case DEFAULTPATH_PALETTE:          m_aPalettePath = aFullPath;       break;
                    case DEFAULTPATH_PLUGIN:           m_aPluginPath = aFullPath;        break;
                    case DEFAULTPATH_TEMP:             m_aTempPath = aFullPath;          break;
                    case DEFAULTPATH_TEMPLATE:         m_aTemplatePath = aFullPath;      break;
                    case DEFAULTPATH_USERCONFIG:       m_aUserConfigPath = aFullPath;    break;
                    case DEFAULTPATH_WORK:             m_aWorkPath = aFullPath;          break;
                    case DEFAULTPATH_CLASSIFICATION:   m_aClassificationPath = aFullPath;break;
                    case DEFAULTPATH_USERDICTIONARY:   break;

                    default:
                        SAL_WARN( "unotools.config", "invalid index to load a default path" );
                }
            }
        }
    }
}

SvtDefaultOptions_Impl::~SvtDefaultOptions_Impl()
{
    if ( IsModified() )
        Commit();
}

// class SvtDefaultOptions -----------------------------------------------
namespace { struct lclMutex : public rtl::Static< ::osl::Mutex, lclMutex > {}; }

SvtDefaultOptions::SvtDefaultOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    pImpl = g_pOptions.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtDefaultOptions_Impl>();
        g_pOptions = pImpl;
        ItemHolder1::holdConfigItem(EItem::DefaultOptions);
    }
}

SvtDefaultOptions::~SvtDefaultOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    pImpl.reset();
}

OUString SvtDefaultOptions::GetDefaultPath( sal_uInt16 nId ) const
{
    return pImpl->GetDefaultPath( nId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
