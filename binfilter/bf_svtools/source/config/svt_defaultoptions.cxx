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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <bf_svtools/defaultoptions.hxx>
#include <bf_svtools/pathoptions.hxx>

#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <tools/debug.hxx>
#include <tools/resmgr.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>

#include <unotools/localfilehelper.hxx>
#include <rtl/instance.hxx>

#include <itemholder1.hxx>

using namespace osl;
using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

namespace binfilter
{

// define ----------------------------------------------------------------

#define ASCII_STR(s)				OUString( RTL_CONSTASCII_USTRINGPARAM(s) )

#define DEFAULTPATH__ADDIN			0
#define DEFAULTPATH__AUTOCORRECT	1
#define DEFAULTPATH__AUTOTEXT		2
#define DEFAULTPATH__BACKUP			3
#define DEFAULTPATH__BASIC			4
#define DEFAULTPATH__BITMAP			5
#define DEFAULTPATH__CONFIG			6
#define DEFAULTPATH__DICTIONARY		7
#define DEFAULTPATH__FAVORITES		8
#define DEFAULTPATH__FILTER			9
#define DEFAULTPATH__GALLERY		10
#define DEFAULTPATH__GRAPHIC		11
#define DEFAULTPATH__HELP			12
#define DEFAULTPATH__LINGUISTIC		13
#define DEFAULTPATH__MODULE			14
#define DEFAULTPATH__PALETTE		15
#define DEFAULTPATH__PLUGIN			16
#define DEFAULTPATH__TEMP			17
#define DEFAULTPATH__TEMPLATE		18
#define DEFAULTPATH__USERCONFIG		19
#define DEFAULTPATH__USERDICTIONARY	20
#define DEFAULTPATH__WORK			21

// class SvtDefaultOptions_Impl ------------------------------------------

class SvtDefaultOptions_Impl : public utl::ConfigItem
{
public:
    String			m_aAddinPath;
    String			m_aAutoCorrectPath;
    String			m_aAutoTextPath;
    String			m_aBackupPath;
    String			m_aBasicPath;
    String			m_aBitmapPath;
    String			m_aConfigPath;
    String			m_aDictionaryPath;
    String			m_aFavoritesPath;
    String			m_aFilterPath;
    String			m_aGalleryPath;
    String			m_aGraphicPath;
    String			m_aHelpPath;
    String			m_aLinguisticPath;
    String			m_aModulePath;
    String			m_aPalettePath;
    String			m_aPluginPath;
    String			m_aTempPath;
    String			m_aTemplatePath;
    String			m_aUserConfigPath;
    String			m_aUserDictionaryPath;
    String			m_aWorkPath;

                    SvtDefaultOptions_Impl();
    void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& );
    void Commit();
};

    void SvtDefaultOptions_Impl::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}
    void SvtDefaultOptions_Impl::Commit() {}

// global ----------------------------------------------------------------

static SvtDefaultOptions_Impl*	pOptions = NULL;
static sal_Int32				nRefCount = 0;

typedef String SvtDefaultOptions_Impl:: *PathStrPtr;

struct PathToDefaultMapping_Impl
{
    SvtPathOptions::Pathes	_ePath;
    PathStrPtr				_pDefaultPath;
};

static PathToDefaultMapping_Impl __READONLY_DATA PathMap_Impl[] =
{
    { SvtPathOptions::PATH_ADDIN,			&SvtDefaultOptions_Impl::m_aAddinPath },
    { SvtPathOptions::PATH_AUTOCORRECT,	&SvtDefaultOptions_Impl::m_aAutoCorrectPath },
    { SvtPathOptions::PATH_AUTOTEXT,		&SvtDefaultOptions_Impl::m_aAutoTextPath },
    { SvtPathOptions::PATH_BACKUP,		&SvtDefaultOptions_Impl::m_aBackupPath },
    { SvtPathOptions::PATH_BASIC,			&SvtDefaultOptions_Impl::m_aBasicPath },
    { SvtPathOptions::PATH_BITMAP,		&SvtDefaultOptions_Impl::m_aBitmapPath },
    { SvtPathOptions::PATH_CONFIG,		&SvtDefaultOptions_Impl::m_aConfigPath },
    { SvtPathOptions::PATH_DICTIONARY,	&SvtDefaultOptions_Impl::m_aDictionaryPath },
    { SvtPathOptions::PATH_FAVORITES,		&SvtDefaultOptions_Impl::m_aFavoritesPath },
    { SvtPathOptions::PATH_FILTER,		&SvtDefaultOptions_Impl::m_aFilterPath },
    { SvtPathOptions::PATH_GALLERY,		&SvtDefaultOptions_Impl::m_aGalleryPath },
    { SvtPathOptions::PATH_GRAPHIC,		&SvtDefaultOptions_Impl::m_aGraphicPath },
    { SvtPathOptions::PATH_HELP,			&SvtDefaultOptions_Impl::m_aHelpPath },
    { SvtPathOptions::PATH_LINGUISTIC,	&SvtDefaultOptions_Impl::m_aLinguisticPath },
    { SvtPathOptions::PATH_MODULE,		&SvtDefaultOptions_Impl::m_aModulePath },
    { SvtPathOptions::PATH_PALETTE,		&SvtDefaultOptions_Impl::m_aPalettePath },
    { SvtPathOptions::PATH_PLUGIN,		&SvtDefaultOptions_Impl::m_aPluginPath },
    { SvtPathOptions::PATH_TEMP,          &SvtDefaultOptions_Impl::m_aTempPath },
    { SvtPathOptions::PATH_TEMPLATE,		&SvtDefaultOptions_Impl::m_aTemplatePath },
    { SvtPathOptions::PATH_USERCONFIG,	&SvtDefaultOptions_Impl::m_aUserConfigPath },
    { SvtPathOptions::PATH_USERDICTIONARY,&SvtDefaultOptions_Impl::m_aUserDictionaryPath },
    { SvtPathOptions::PATH_WORK,			&SvtDefaultOptions_Impl::m_aWorkPath }
};

// functions -------------------------------------------------------------

Sequence< OUString > GetDefaultPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Addin",			// PATH_ADDIN
        "AutoCorrect",		// PATH_AUTOCORRECT
        "AutoText",			// PATH_AUTOTEXT
        "Backup",			// PATH_BACKUP
        "Basic",			// PATH_BASIC
        "Bitmap",			// PATH_BITMAP
        "Config",			// PATH_CONFIG
        "Dictionary",		// PATH_DICTIONARY
        "Favorite",			// PATH_FAVORITES
        "Filter",			// PATH_FILTER
        "Gallery",			// PATH_GALLERY
        "Graphic",			// PATH_GRAPHIC
        "Help",				// PATH_HELP
        "Linguistic",		// PATH_LINGUISTIC
        "Module",			// PATH_MODULE
        "Palette",			// PATH_PALETTE
        "Plugin",			// PATH_PLUGIN
        "Temp",		 		// PATH_TEMP
        "Template",			// PATH_TEMPLATE
        "UserConfig",		// PATH_USERCONFIG
        "UserDictionary",	// PATH_USERDICTIONARY
        "Work"				// PATH_WORK
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------

SvtDefaultOptions_Impl::SvtDefaultOptions_Impl() : ConfigItem( ASCII_STR("Office.Common/Path/Default") )
{
    /*ConfigManager* pCfgMgr =*/ ConfigManager::GetConfigManager();
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
                        // multi pathes
                        if ( pValues[nProp] >>= aTempStr )
                            aFullPath = aPathOpt.SubstituteVariable( aTempStr );
                        else
                        {
                            DBG_ERRORFILE( "any operator >>= failed" );
                        }
                        break;
                    }

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    {
                        // single pathes
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
                            DBG_ERRORFILE( "any operator >>= failed" );
                        }
                        break;
                    }

                    default:
                    {
                        DBG_ERRORFILE( "Wrong any type" );
                    }
                }

                switch ( nProp )
                {
                    case DEFAULTPATH__ADDIN:			m_aAddinPath = String( aFullPath );			break;
                    case DEFAULTPATH__AUTOCORRECT:		m_aAutoCorrectPath = String( aFullPath );	break;
                    case DEFAULTPATH__AUTOTEXT:			m_aAutoTextPath = String( aFullPath );		break;
                    case DEFAULTPATH__BACKUP:			m_aBackupPath = String( aFullPath );		break;
                    case DEFAULTPATH__BASIC:			m_aBasicPath = String( aFullPath );			break;
                    case DEFAULTPATH__BITMAP:			m_aBitmapPath = String( aFullPath );		break;
                    case DEFAULTPATH__CONFIG:			m_aConfigPath = String( aFullPath );		break;
                    case DEFAULTPATH__DICTIONARY:		m_aDictionaryPath = String( aFullPath );	break;
                    case DEFAULTPATH__FAVORITES:		m_aFavoritesPath = String( aFullPath );		break;
                    case DEFAULTPATH__FILTER:			m_aFilterPath = String( aFullPath );		break;
                    case DEFAULTPATH__GALLERY:			m_aGalleryPath = String( aFullPath );		break;
                    case DEFAULTPATH__GRAPHIC:			m_aGraphicPath = String( aFullPath );		break;
                    case DEFAULTPATH__HELP:				m_aHelpPath = String( aFullPath );			break;
                    case DEFAULTPATH__LINGUISTIC:		m_aLinguisticPath = String( aFullPath );	break;
                    case DEFAULTPATH__MODULE:			m_aModulePath = String( aFullPath );		break;
                    case DEFAULTPATH__PALETTE:			m_aPalettePath = String( aFullPath );		break;
                    case DEFAULTPATH__PLUGIN:			m_aPluginPath = String( aFullPath );		break;
                    case DEFAULTPATH__TEMP:				m_aTempPath = String( aFullPath );			break;
                    case DEFAULTPATH__TEMPLATE:			m_aTemplatePath = String( aFullPath );		break;
                    case DEFAULTPATH__USERCONFIG:		m_aUserConfigPath = String( aFullPath );	break;
                    case DEFAULTPATH__USERDICTIONARY:	m_aUserDictionaryPath = String( aFullPath );break;
                    case DEFAULTPATH__WORK:				m_aWorkPath = String( aFullPath );			break;

                    default:
                        DBG_ERRORFILE( "invalid index to load a default path" );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
