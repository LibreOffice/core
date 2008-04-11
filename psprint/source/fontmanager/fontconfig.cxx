/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontconfig.cxx,v $
 * $Revision: 1.30 $
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
#include "precompiled_psprint.hxx"
#include <psprint/fontmanager.hxx>
#ifndef _PSPRINT_FONTCACHE_HXX_
#include <psprint/fontcache.hxx>
#endif
using namespace psp;

#ifdef ENABLE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <fontconfig/fcfreetype.h>
// be compatible with fontconfig 2.2.0 release
#ifndef FC_WEIGHT_BOOK
    #define FC_WEIGHT_BOOK 75
#endif
#ifndef FC_EMBEDDED_BITMAP
    #define FC_EMBEDDED_BITMAP "embeddedbitmap"
#endif
#ifndef FC_FAMILYLANG
    #define FC_FAMILYLANG "familylang"
#endif
#else
typedef void FcConfig;
typedef void FcObjectSet;
typedef void FcPattern;
typedef void FcFontSet;
typedef void FcCharSet;
typedef int FcResult;
typedef int FcBool;
typedef int FcMatchKind;
typedef char FcChar8;
typedef int FcChar32;
typedef unsigned int FT_UInt;
typedef void* FT_Face;
typedef int FcSetName;
#endif

#include <cstdio>
#include <cstdarg>
#include <unotools/atom.hxx>
#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif
#ifndef _OSL_THREAD_H
#include <osl/thread.h>
#endif
#ifndef _RTL_USTRBUF_HXX
#include <rtl/ustrbuf.hxx>
#endif
#include <osl/process.h>
#include <rtl/locale.hxx>
#include <sal/alloca.h>

#include <utility>
#include <algorithm>

using namespace osl;
using namespace rtl;

class FontCfgWrapper
{
    oslModule       m_pLib;
    FcConfig*       m_pDefConfig;
    FcFontSet*      m_pOutlineSet;

    FcBool          (*m_pFcInit)();
    int             (*m_pFcGetVersion)();
    FcConfig*       (*m_pFcConfigGetCurrent)();
    FcObjectSet*    (*m_pFcObjectSetVaBuild)(const char*,va_list);
    void            (*m_pFcObjectSetDestroy)(FcObjectSet* pSet);
    FcPattern*      (*m_pFcPatternCreate)();
    void            (*m_pFcPatternDestroy)(FcPattern*);
    FcFontSet*      (*m_pFcFontList)(FcConfig*,FcPattern*,FcObjectSet*);
    FcFontSet*      (*m_pFcConfigGetFonts)(FcConfig*,FcSetName);
    FcFontSet*      (*m_pFcFontSetCreate)();
    FcCharSet*                (*m_pFcCharSetCreate)();
    FcBool                    (*m_pFcCharSetAddChar)(FcCharSet *, FcChar32);
    FcBool          (*m_pFcCharSetHasChar)(FcCharSet *, FcChar32);
    void            (*m_pFcCharSetDestroy)(FcCharSet*);
    void            (*m_pFcFontSetDestroy)(FcFontSet*);
    FcBool          (*m_pFcFontSetAdd)(FcFontSet*,FcPattern*);
    void            (*m_pFcPatternReference)(FcPattern*);
    FcResult        (*m_pFcPatternGetCharSet)(const FcPattern*,const char*,int,FcCharSet**);
    FcResult        (*m_pFcPatternGetString)(const FcPattern*,const char*,int,FcChar8**);
    FcResult        (*m_pFcPatternGetInteger)(const FcPattern*,const char*,int,int*);
    FcResult        (*m_pFcPatternGetDouble)(const FcPattern*,const char*,int,double*);
    FcResult        (*m_pFcPatternGetBool)(const FcPattern*,const char*,int,FcBool*);
    void            (*m_pFcDefaultSubstitute)(FcPattern *);
    FcPattern*      (*m_pFcFontSetMatch)(FcConfig*,FcFontSet**, int, FcPattern*,FcResult*);
    FcBool          (*m_pFcConfigAppFontAddFile)(FcConfig*, const FcChar8*);
    FcBool          (*m_pFcConfigAppFontAddDir)(FcConfig*, const FcChar8*);
    FcBool          (*m_pFcConfigSubstitute)(FcConfig*,FcPattern*,FcMatchKind);
    FcBool          (*m_pFcPatternAddInteger)(FcPattern*,const char*,int);
    FcBool                    (*m_pFcPatternAddBool)(FcPattern*,const char*,FcBool);
    FcBool                    (*m_pFcPatternAddCharSet)(FcPattern*,const char*,const FcCharSet*);
    FcBool          (*m_pFcPatternAddString)(FcPattern*,const char*,const FcChar8*);
    FT_UInt         (*m_pFcFreeTypeCharIndex)(FT_Face,FcChar32);

    oslGenericFunction loadSymbol( const char* );
    void addFontSet( FcSetName );

    FontCfgWrapper();
    ~FontCfgWrapper();

public:
    static FontCfgWrapper& get();
    static void release();

    bool isValid() const
    { return m_pLib != NULL;}

    FcConfig* getDefConfig() { return m_pDefConfig; }
    FcFontSet* getFontSet();

    FcBool FcInit()
    { return m_pFcInit(); }

    int FcGetVersion()
    { return m_pFcGetVersion(); }

    FcConfig* FcConfigGetCurrent()
    { return m_pFcConfigGetCurrent(); }

    FcObjectSet* FcObjectSetBuild( const char* first, ... )
    {
        va_list ap;
        va_start( ap, first );
        FcObjectSet* pSet = m_pFcObjectSetVaBuild( first, ap );
        va_end( ap );
        return pSet;
    }

    void FcObjectSetDestroy( FcObjectSet* pSet )
    { m_pFcObjectSetDestroy( pSet ); }

    FcPattern* FcPatternCreate()
    { return m_pFcPatternCreate(); }

    void FcPatternDestroy( FcPattern* pPattern )
    { m_pFcPatternDestroy( pPattern ); }

    FcFontSet* FcFontList( FcConfig* pConfig, FcPattern* pPattern, FcObjectSet* pSet )
    { return m_pFcFontList( pConfig, pPattern, pSet ); }

    FcFontSet* FcConfigGetFonts( FcConfig* pConfig, FcSetName eSet)
    { return m_pFcConfigGetFonts( pConfig, eSet ); }

    FcFontSet* FcFontSetCreate()
    { return m_pFcFontSetCreate(); }

    FcCharSet* FcCharSetCreate()
    { return m_pFcCharSetCreate(); }

    FcBool FcCharSetAddChar(FcCharSet *fcs, FcChar32 ucs4)
    { return m_pFcCharSetAddChar(fcs, ucs4); }

    FcBool FcCharSetHasChar(FcCharSet *fcs, FcChar32 ucs4)
    { return m_pFcCharSetHasChar(fcs, ucs4); }

    void FcCharSetDestroy( FcCharSet* pSet )
    { m_pFcCharSetDestroy( pSet );}

    void FcFontSetDestroy( FcFontSet* pSet )
    { m_pFcFontSetDestroy( pSet );}

    FcBool FcFontSetAdd( FcFontSet* pSet, FcPattern* pPattern )
    { return m_pFcFontSetAdd( pSet, pPattern ); }

    void FcPatternReference( FcPattern* pPattern )
    { m_pFcPatternReference( pPattern ); }

    FcResult FcPatternGetCharSet( const FcPattern* pPattern, const char* object, int n, FcCharSet** s )
    { return m_pFcPatternGetCharSet( pPattern, object, n, s ); }

    FcResult FcPatternGetString( const FcPattern* pPattern, const char* object, int n, FcChar8** s )
    { return m_pFcPatternGetString( pPattern, object, n, s ); }

    FcResult FcPatternGetInteger( const FcPattern* pPattern, const char* object, int n, int* s )
    { return m_pFcPatternGetInteger( pPattern, object, n, s ); }

    FcResult FcPatternGetDouble( const FcPattern* pPattern, const char* object, int n, double* s )
    { return m_pFcPatternGetDouble( pPattern, object, n, s ); }

    FcResult FcPatternGetBool( const FcPattern* pPattern, const char* object, int n, FcBool* s )
    { return m_pFcPatternGetBool( pPattern, object, n, s ); }
    FcBool FcConfigAppFontAddFile( FcConfig* pConfig, const FcChar8* pFileName )
    { return m_pFcConfigAppFontAddFile( pConfig, pFileName ); }
    FcBool FcConfigAppFontAddDir(FcConfig* pConfig, const FcChar8* pDirName )
    { return m_pFcConfigAppFontAddDir( pConfig, pDirName ); }
    void FcDefaultSubstitute( FcPattern* pPattern )
    { m_pFcDefaultSubstitute( pPattern ); }
    FcPattern* FcFontSetMatch( FcConfig* pConfig, FcFontSet **ppFontSet, int nset, FcPattern* pPattern, FcResult* pResult )
    { return m_pFcFontSetMatch ? m_pFcFontSetMatch( pConfig, ppFontSet, nset, pPattern, pResult ) : 0; }
    FcBool FcConfigSubstitute( FcConfig* pConfig, FcPattern* pPattern, FcMatchKind eKind )
    { return m_pFcConfigSubstitute( pConfig, pPattern, eKind ); }
    FcBool FcPatternAddInteger( FcPattern* pPattern, const char* pObject, int nValue )
    { return m_pFcPatternAddInteger( pPattern, pObject, nValue ); }
    FcBool FcPatternAddString( FcPattern* pPattern, const char* pObject, const FcChar8* pString )
    { return m_pFcPatternAddString( pPattern, pObject, pString ); }
    FcBool FcPatternAddBool( FcPattern* pPattern, const char* pObject, bool nValue )
    { return m_pFcPatternAddBool( pPattern, pObject, nValue ); }
    FcBool FcPatternAddCharSet(FcPattern* pPattern,const char* pObject,const FcCharSet*pCharSet)
    { return m_pFcPatternAddCharSet(pPattern,pObject,pCharSet); }

    FT_UInt FcFreeTypeCharIndex( FT_Face face, FcChar32 ucs4 )
    { return m_pFcFreeTypeCharIndex ? m_pFcFreeTypeCharIndex( face, ucs4 ) : 0; }

public: // TODO: cleanup
    std::hash_map< rtl::OString, rtl::OString, rtl::OStringHash > m_aFontconfigNameToLocalized;
};

oslGenericFunction FontCfgWrapper::loadSymbol( const char* pSymbol )
{
    OUString aSym( OUString::createFromAscii( pSymbol ) );
    oslGenericFunction pSym = osl_getFunctionSymbol( m_pLib, aSym.pData );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s %s\n", pSymbol, pSym ? "found" : "not found" );
#endif
    return pSym;
}

FontCfgWrapper::FontCfgWrapper()
        : m_pLib( NULL ),
          m_pDefConfig( NULL ),
          m_pOutlineSet( NULL )
{
     OUString aLib( RTL_CONSTASCII_USTRINGPARAM( "libfontconfig.so.1" ) );
    m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    if( !m_pLib )
    {
        aLib = OUString( RTL_CONSTASCII_USTRINGPARAM( "libfontconfig.so" ) );
        m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    }

    if( ! m_pLib )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "no libfontconfig\n" );
#endif
        return;
    }

    m_pFcInit = (FcBool(*)())
        loadSymbol( "FcInit" );
    m_pFcGetVersion = (int(*)())
        loadSymbol( "FcGetVersion" );
    m_pFcConfigGetCurrent = (FcConfig *(*)())
        loadSymbol( "FcConfigGetCurrent" );
    m_pFcObjectSetVaBuild = (FcObjectSet*(*)(const char*,va_list))
        loadSymbol( "FcObjectSetVaBuild" );
    m_pFcObjectSetDestroy = (void(*)(FcObjectSet*))
        loadSymbol( "FcObjectSetDestroy" );
    m_pFcPatternCreate = (FcPattern*(*)())
        loadSymbol( "FcPatternCreate" );
    m_pFcPatternDestroy = (void(*)(FcPattern*))
        loadSymbol( "FcPatternDestroy" );
    m_pFcFontList = (FcFontSet*(*)(FcConfig*,FcPattern*,FcObjectSet*))
        loadSymbol( "FcFontList" );
    m_pFcConfigGetFonts = (FcFontSet*(*)(FcConfig*,FcSetName))
        loadSymbol( "FcConfigGetFonts" );
    m_pFcFontSetCreate = (FcFontSet*(*)())
        loadSymbol( "FcFontSetCreate" );
    m_pFcCharSetCreate = (FcCharSet*(*)())
        loadSymbol( "FcCharSetCreate" );
    m_pFcCharSetAddChar = (FcBool(*)(FcCharSet*, FcChar32))
        loadSymbol( "FcCharSetAddChar" );
    m_pFcCharSetHasChar = (FcBool(*)(FcCharSet*, FcChar32))
        loadSymbol( "FcCharSetHasChar" );
    m_pFcCharSetDestroy = (void(*)(FcCharSet*))
        loadSymbol( "FcCharSetDestroy" );
    m_pFcFontSetDestroy = (void(*)(FcFontSet*))
        loadSymbol( "FcFontSetDestroy" );
    m_pFcFontSetAdd = (FcBool(*)(FcFontSet*,FcPattern*))
        loadSymbol( "FcFontSetAdd" );
    m_pFcPatternReference = (void(*)(FcPattern*))
        loadSymbol( "FcPatternReference" );
    m_pFcPatternGetCharSet = (FcResult(*)(const FcPattern*,const char*,int,FcCharSet**))
        loadSymbol( "FcPatternGetCharSet" );
    m_pFcPatternGetString = (FcResult(*)(const FcPattern*,const char*,int,FcChar8**))
        loadSymbol( "FcPatternGetString" );
    m_pFcPatternGetInteger = (FcResult(*)(const FcPattern*,const char*,int,int*))
        loadSymbol( "FcPatternGetInteger" );
    m_pFcPatternGetDouble = (FcResult(*)(const FcPattern*,const char*,int,double*))
        loadSymbol( "FcPatternGetDouble" );
    m_pFcPatternGetBool = (FcResult(*)(const FcPattern*,const char*,int,FcBool*))
        loadSymbol( "FcPatternGetBool" );
    m_pFcConfigAppFontAddFile = (FcBool(*)(FcConfig*, const FcChar8*))
        loadSymbol( "FcConfigAppFontAddFile" );
    m_pFcConfigAppFontAddDir = (FcBool(*)(FcConfig*, const FcChar8*))
        loadSymbol( "FcConfigAppFontAddDir" );
    m_pFcDefaultSubstitute = (void(*)(FcPattern *))
        loadSymbol( "FcDefaultSubstitute" );
    m_pFcFontSetMatch = (FcPattern*(*)(FcConfig*,FcFontSet**,int,FcPattern*,FcResult*))
        loadSymbol( "FcFontSetMatch" );
    m_pFcConfigSubstitute = (FcBool(*)(FcConfig*,FcPattern*,FcMatchKind))
        loadSymbol( "FcConfigSubstitute" );
    m_pFcPatternAddInteger = (FcBool(*)(FcPattern*,const char*,int))
        loadSymbol( "FcPatternAddInteger" );
    m_pFcPatternAddBool = (FcBool(*)(FcPattern*,const char*,FcBool))
        loadSymbol( "FcPatternAddBool" );
    m_pFcPatternAddCharSet = (FcBool(*)(FcPattern*,const char*,const FcCharSet *))
        loadSymbol( "FcPatternAddCharSet" );
    m_pFcPatternAddString = (FcBool(*)(FcPattern*,const char*,const FcChar8*))
        loadSymbol( "FcPatternAddString" );
    m_pFcFreeTypeCharIndex = (FT_UInt(*)(FT_Face,FcChar32))
        loadSymbol( "FcFreeTypeCharIndex" );

    if( ! (
            m_pFcInit                       &&
            m_pFcGetVersion                 &&
            m_pFcConfigGetCurrent           &&
            m_pFcObjectSetVaBuild           &&
            m_pFcObjectSetDestroy           &&
            m_pFcPatternCreate              &&
            m_pFcPatternDestroy             &&
            m_pFcFontList                   &&
            m_pFcConfigGetFonts             &&
            m_pFcFontSetCreate              &&
            m_pFcCharSetCreate              &&
            m_pFcCharSetAddChar             &&
            m_pFcCharSetHasChar             &&
            m_pFcCharSetDestroy             &&
            m_pFcFontSetDestroy             &&
            m_pFcFontSetAdd                 &&
            m_pFcPatternReference           &&
            m_pFcPatternGetCharSet          &&
            m_pFcPatternGetString           &&
            m_pFcPatternGetInteger          &&
            m_pFcPatternGetDouble           &&
            m_pFcPatternGetBool             &&
            m_pFcConfigAppFontAddFile               &&
            m_pFcConfigAppFontAddDir                &&
            m_pFcDefaultSubstitute          &&
            m_pFcConfigSubstitute           &&
            m_pFcPatternAddInteger          &&
            m_pFcPatternAddCharSet          &&
            m_pFcPatternAddBool             &&
            m_pFcPatternAddString
            ) )
    {
        osl_unloadModule( (oslModule)m_pLib );
        m_pLib = NULL;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "not all needed symbols were found in libfontconfig\n" );
#endif
        return;
    }


    FcInit();
    m_pDefConfig = FcConfigGetCurrent();
    if( ! m_pDefConfig )
    {
        osl_unloadModule( (oslModule)m_pLib );
        m_pLib = NULL;
    }
}

void FontCfgWrapper::addFontSet( FcSetName eSetName )
{
    #ifdef ENABLE_FONTCONFIG
    /*
      add only acceptable outlined fonts to our config,
      for future fontconfig use
    */
    FcFontSet* pOrig = FcConfigGetFonts( getDefConfig(), eSetName );
    if( !pOrig )
        return;

    for( int i = 0; i < pOrig->nfont; ++i )
    {
        FcBool outline = false;
        FcPattern *pOutlinePattern = pOrig->fonts[i];
        FcResult eOutRes =
                 FcPatternGetBool( pOutlinePattern, FC_OUTLINE, 0, &outline );
        if( (eOutRes != FcResultMatch) || (outline != FcTrue) )
            continue;
        FcPatternReference(pOutlinePattern);
        FcFontSetAdd(m_pOutlineSet, pOutlinePattern);
    }
    // TODO: FcFontSetDestroy( pOrig );
    #else
    (void)eSetName; // prevent compiler warning about unused parameter
    #endif
}

FcFontSet* FontCfgWrapper::getFontSet()
{
    #ifdef ENABLE_FONTCONFIG
    if( !m_pOutlineSet )
    {
        m_pOutlineSet = FcFontSetCreate();
        addFontSet( FcSetSystem );
        const int nVersion = FcGetVersion();
    if( nVersion > 20400 )
            addFontSet( FcSetApplication );
    }
    #endif

    return m_pOutlineSet;
}

FontCfgWrapper::~FontCfgWrapper()
{
    if( m_pOutlineSet )
        FcFontSetDestroy( m_pOutlineSet );
    if( m_pLib )
        osl_unloadModule( (oslModule)m_pLib );
}

static FontCfgWrapper* pOneInstance = NULL;

FontCfgWrapper& FontCfgWrapper::get()
{
    if( ! pOneInstance )
        pOneInstance = new FontCfgWrapper();
    return *pOneInstance;
}

void FontCfgWrapper::release()
{
    if( pOneInstance )
    {
        delete pOneInstance;
        pOneInstance = NULL;
    }
}

#ifdef ENABLE_FONTCONFIG
namespace
{
    typedef std::pair<FcChar8*, FcChar8*> lang_and_family;

    class localizedsorter
    {
            rtl::OLocale maLoc;
        public:
            localizedsorter(rtl_Locale* pLoc) : maLoc(pLoc) {}
            FcChar8* bestname(const std::vector<lang_and_family> &families);
    };

    FcChar8* localizedsorter::bestname(const std::vector<lang_and_family> &families)
    {
        FcChar8* candidate = families.begin()->second;
        rtl::OString sLangMatch(rtl::OUStringToOString(maLoc.getLanguage().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8));
    rtl::OString sFullMatch = sLangMatch;
        sFullMatch += OString('-');
        sFullMatch += rtl::OUStringToOString(maLoc.getCountry().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8);

        std::vector<lang_and_family>::const_iterator aEnd = families.end();
        bool alreadyclosematch = false;
        for (std::vector<lang_and_family>::const_iterator aIter = families.begin(); aIter != aEnd; ++aIter)
        {
            const char *pLang = (const char*)aIter->first;
            //perfect
            if( rtl_str_compare(pLang,sFullMatch.getStr() ) == 0)
            {
                candidate = aIter->second;
                break;
            }
            else if( (rtl_str_compare(pLang,sLangMatch.getStr()) == 0) && (!alreadyclosematch))
            {
                candidate = aIter->second;
                alreadyclosematch = true;
            }
        }

        return candidate;
    }


    FcResult lcl_FamilyFromPattern(FontCfgWrapper& rWrapper, FcPattern* pPattern, FcChar8 **family,
        std::hash_map< rtl::OString, rtl::OString, rtl::OStringHash > &aFontconfigNameToLocalized)
    {
        FcChar8 *origfamily;
        FcResult eFamilyRes = rWrapper.FcPatternGetString( pPattern, FC_FAMILY, 0, &origfamily );
        *family = origfamily;

        if( eFamilyRes == FcResultMatch)
        {
            FcChar8* familylang = NULL;
            if (rWrapper.FcPatternGetString( pPattern, FC_FAMILYLANG, 0, &familylang ) == FcResultMatch)
            {
                std::vector< lang_and_family > lang_and_families;
                lang_and_families.push_back(lang_and_family(familylang, *family));
                int k = 1;
                while (1)
                {
                    if (rWrapper.FcPatternGetString( pPattern, FC_FAMILYLANG, k, &familylang ) != FcResultMatch)
                        break;
                    if (rWrapper.FcPatternGetString( pPattern, FC_FAMILY, k, family ) != FcResultMatch)
                        break;
                    lang_and_families.push_back(lang_and_family(familylang, *family));
                    ++k;
                }

                //possible to-do, sort by UILocale instead of process locale
                rtl_Locale* pLoc;
                osl_getProcessLocale(&pLoc);
                localizedsorter aSorter(pLoc);
                *family = aSorter.bestname(lang_and_families);

                std::vector<lang_and_family>::const_iterator aEnd = lang_and_families.end();
                for (std::vector<lang_and_family>::const_iterator aIter = lang_and_families.begin(); aIter != aEnd; ++aIter)
                {
                    const char *candidate = (const char*)(aIter->second);
                    if (rtl_str_compare(candidate, (const char*)(*family)) != 0)
                        aFontconfigNameToLocalized[OString(candidate)] = OString((const char*)(*family));
                }
            }
        }

        return eFamilyRes;
    }
}


/*
 * PrintFontManager::initFontconfig
 */
bool PrintFontManager::initFontconfig()
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return false;
    return true;
}

int PrintFontManager::countFontconfigFonts()
{
    int nFonts = 0;

    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( !rWrapper.isValid() )
        return 0;

    FcFontSet* pFSet = rWrapper.getFontSet();
    if( pFSet )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %d entries in fontconfig fontset\n", pFSet->nfont );
#endif
        for( int i = 0; i < pFSet->nfont; i++ )
        {
            FcChar8* file = NULL;
            FcChar8* family = NULL;
            FcChar8* style = NULL;
            int slant = 0;
            int weight = 0;
            int spacing = 0;
            int nCollectionEntry = -1;
            FcBool outline = false, embitmap = true, antialias = true;

            FcResult eFileRes         = rWrapper.FcPatternGetString( pFSet->fonts[i], FC_FILE, 0, &file );
            FcResult eFamilyRes       = lcl_FamilyFromPattern(rWrapper, pFSet->fonts[i], &family, rWrapper.m_aFontconfigNameToLocalized );
            FcResult eStyleRes        = rWrapper.FcPatternGetString( pFSet->fonts[i], FC_STYLE, 0, &style );
            FcResult eSlantRes        = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SLANT, 0, &slant );
            FcResult eWeightRes       = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_WEIGHT, 0, &weight );
            FcResult eSpacRes         = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SPACING, 0, &spacing );
            FcResult eOutRes          = rWrapper.FcPatternGetBool( pFSet->fonts[i], FC_OUTLINE, 0, &outline );
            FcResult eIndexRes        = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_INDEX, 0, &nCollectionEntry );
            FcResult eEmbeddedBitmap  = rWrapper.FcPatternGetBool( pFSet->fonts[i], FC_EMBEDDED_BITMAP, 0, &embitmap );
            FcResult eAntialias       = rWrapper.FcPatternGetBool( pFSet->fonts[i], FC_ANTIALIAS, 0, &antialias );

            if( eFileRes != FcResultMatch || eFamilyRes != FcResultMatch || eOutRes != FcResultMatch )
                continue;

#if (OSL_DEBUG_LEVEL > 2)
            fprintf( stderr, "found font \"%s\" in file %s\n"
                     "   weight = %d, slant = %d, style = \"%s\"\n"
                     "   spacing = %d, outline = %d\n"
                     , family, file
                     , eWeightRes == FcResultMatch ? weight : -1
                     , eSpacRes == FcResultMatch ? slant : -1
                     , eStyleRes == FcResultMatch ? (const char*) style : "<nil>"
                     , eSpacRes == FcResultMatch ? spacing : -1
                     , eOutRes == FcResultMatch ? outline : -1
                     );
#endif

            OSL_ASSERT(eOutRes != FcResultMatch || outline);

            // only outline fonts are usable to psprint anyway
            if( eOutRes == FcResultMatch && ! outline )
                continue;

            // see if this font is already cached
            // update attributes
            std::list< PrintFont* > aFonts;
            OString aDir, aBase, aOrgPath( (sal_Char*)file );
            splitPath( aOrgPath, aDir, aBase );
            int nDirID = getDirectoryAtom( aDir, true );
            if( ! m_pFontCache->getFontCacheFile( nDirID, aBase, aFonts ) )
            {
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "file %s not cached\n", aBase.getStr() );
#endif
                // not known, analyze font file to get attributes
                // not described by fontconfig (e.g. alias names, PSName)
                std::list< OString > aDummy;
                analyzeFontFile( nDirID, aBase, aDummy, aFonts );
#if OSL_DEBUG_LEVEL > 1
                if( aFonts.empty() )
                    fprintf( stderr, "Warning: file \"%s\" is unusable to psprint\n", aOrgPath.getStr() );
#endif
            }
            if( aFonts.empty() )
                continue;

            int nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( OString( (sal_Char*)family ), RTL_TEXTENCODING_UTF8 ), sal_True );
            PrintFont* pUpdate = aFonts.front();
            std::list<PrintFont*>::const_iterator second_font = aFonts.begin();
            ++second_font;
            if( second_font != aFonts.end() ) // more than one font
            {
                // a collection entry, get the correct index
                if( eIndexRes == FcResultMatch && nCollectionEntry != -1 )
                {
                    for( std::list< PrintFont* >::iterator it = aFonts.begin(); it != aFonts.end(); ++it )
                    {
                        if( (*it)->m_eType == fonttype::TrueType &&
                            static_cast<TrueTypeFontFile*>(*it)->m_nCollectionEntry == nCollectionEntry )
                        {
                            pUpdate = *it;
                            break;
                        }
                    }
                    // update collection entry
                    // additional entries will be created in the cache
                    // if this is a new index (that is if the loop above
                    // ran to the end of the list)
                    if( pUpdate->m_eType == fonttype::TrueType ) // sanity check, this should always be the case here
                        static_cast<TrueTypeFontFile*>(pUpdate)->m_nCollectionEntry = nCollectionEntry;
                }
                else
                {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "multiple fonts for file, but no index in fontconfig pattern ! (index res = %d collection entry = %d\nfile will not be used\n", eIndexRes, nCollectionEntry );
#endif
                    // we have found more than one font in this file
                    // but fontconfig will not tell us which index is meant
                    // -> something is in disorder, do not use this font
                    pUpdate = NULL;
                }
            }

            if( pUpdate )
            {
                // set family name
                if( pUpdate->m_nFamilyName != nFamilyName )
                {
                    pUpdate->m_aAliases.remove( pUpdate->m_nFamilyName );
                    pUpdate->m_aAliases.push_back( pUpdate->m_nFamilyName );
                    pUpdate->m_aAliases.remove( nFamilyName );
                    pUpdate->m_nFamilyName = nFamilyName;
                }
                if( eWeightRes == FcResultMatch )
                {
                    // set weight
                    if( weight <= FC_WEIGHT_THIN )
                        pUpdate->m_eWeight = weight::Thin;
                    else if( weight <= FC_WEIGHT_ULTRALIGHT )
                        pUpdate->m_eWeight = weight::UltraLight;
                    else if( weight <= FC_WEIGHT_LIGHT )
                        pUpdate->m_eWeight = weight::Light;
                    else if( weight <= FC_WEIGHT_BOOK )
                        pUpdate->m_eWeight = weight::SemiLight;
                    else if( weight <= FC_WEIGHT_NORMAL )
                        pUpdate->m_eWeight = weight::Normal;
                    else if( weight <= FC_WEIGHT_MEDIUM )
                        pUpdate->m_eWeight = weight::Medium;
                    else if( weight <= FC_WEIGHT_SEMIBOLD )
                        pUpdate->m_eWeight = weight::SemiBold;
                    else if( weight <= FC_WEIGHT_BOLD )
                        pUpdate->m_eWeight = weight::Bold;
                    else if( weight <= FC_WEIGHT_ULTRABOLD )
                        pUpdate->m_eWeight = weight::UltraBold;
                    else
                        pUpdate->m_eWeight = weight::Black;
                }
                if( eSpacRes == FcResultMatch )
                {
                    // set pitch
                    if( spacing == FC_PROPORTIONAL )
                        pUpdate->m_ePitch = pitch::Variable;
                    else if( spacing == FC_MONO || spacing == FC_CHARCELL )
                        pUpdate->m_ePitch = pitch::Fixed;
                }
                if( eSlantRes == FcResultMatch )
                {
                    // set italic
                    if( slant == FC_SLANT_ROMAN )
                        pUpdate->m_eItalic = italic::Upright;
                    else if( slant == FC_SLANT_ITALIC )
                        pUpdate->m_eItalic = italic::Italic;
                    else if( slant == FC_SLANT_OBLIQUE )
                        pUpdate->m_eItalic = italic::Oblique;
                }
                if( eStyleRes == FcResultMatch )
                {
                    pUpdate->m_aStyleName = OStringToOUString( OString( (sal_Char*)style ), RTL_TEXTENCODING_UTF8 );
                }
                if( eEmbeddedBitmap == FcResultMatch )
                {
                  pUpdate->m_eEmbeddedbitmap = embitmap ? fcstatus::istrue : fcstatus::isfalse;
                }
                if( eAntialias == FcResultMatch )
                {
                  pUpdate->m_eAntialias = antialias ? fcstatus::istrue : fcstatus::isfalse;
                }


                // update font cache
                m_pFontCache->updateFontCacheEntry( pUpdate, false );
                // sort into known fonts
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = pUpdate;
                m_aFontFileToFontID[ aBase ].insert( aFont );
                nFonts++;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "inserted font %s as fontID %d\n", family, aFont );
#endif
            }
            // clean up the fonts we did not put into the list
            for( std::list< PrintFont* >::iterator it = aFonts.begin(); it != aFonts.end(); ++it )
            {
                if( *it != pUpdate )
                {
                    m_pFontCache->updateFontCacheEntry( *it, false ); // prepare a cache entry for a collection item
                    delete *it;
                }
            }
        }
    }

    // how does one get rid of the config ?
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "inserted %d fonts from fontconfig\n", nFonts );
#endif
    return nFonts;
}

void PrintFontManager::deinitFontconfig()
{
    FontCfgWrapper::release();
}

int PrintFontManager::FreeTypeCharIndex( void *pFace, sal_uInt32 aChar )
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    return rWrapper.isValid() ? rWrapper.FcFreeTypeCharIndex( (FT_Face)pFace, aChar ) : 0;
}

bool PrintFontManager::addFontconfigDir( const rtl::OString& rDirName )
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return false;

    // libfontcconfig's AppFontAddDir was broken in version 2.4.0
    const int nVersion = rWrapper.FcGetVersion();
    if( nVersion <= 20400 )
    return false;
    const char* pDirName = (const char*)rDirName.getStr();
    bool bRet = (rWrapper.FcConfigAppFontAddDir( rWrapper.getDefConfig(), (FcChar8*)pDirName ) == FcTrue);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "FcConfigAppFontAddDir( \"%s\") => %d\n", pDirName, bRet );
#endif

    return bRet;
}

static void addtopattern(FontCfgWrapper& rWrapper, FcPattern *pPattern,
    italic::type eItalic, weight::type eWeight, width::type eWidth, pitch::type ePitch)
{
    if( eItalic != italic::Unknown )
    {
        int nSlant = FC_SLANT_ROMAN;
        switch( eItalic )
        {
            case italic::Italic:        nSlant = FC_SLANT_ITALIC;break;
            case italic::Oblique:       nSlant = FC_SLANT_OBLIQUE;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_SLANT, nSlant );
    }
    if( eWeight != weight::Unknown )
    {
        int nWeight = FC_WEIGHT_NORMAL;
        switch( eWeight )
        {
            case weight::Thin:          nWeight = FC_WEIGHT_THIN;break;
            case weight::UltraLight:    nWeight = FC_WEIGHT_ULTRALIGHT;break;
            case weight::Light:         nWeight = FC_WEIGHT_LIGHT;break;
            case weight::SemiLight:     nWeight = FC_WEIGHT_BOOK;break;
            case weight::Normal:        nWeight = FC_WEIGHT_NORMAL;break;
            case weight::Medium:        nWeight = FC_WEIGHT_MEDIUM;break;
            case weight::SemiBold:      nWeight = FC_WEIGHT_SEMIBOLD;break;
            case weight::Bold:          nWeight = FC_WEIGHT_BOLD;break;
            case weight::UltraBold:     nWeight = FC_WEIGHT_ULTRABOLD;break;
            case weight::Black:         nWeight = FC_WEIGHT_BLACK;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_WEIGHT, nWeight );
    }
    if( eWidth != width::Unknown )
    {
        int nWidth = FC_WIDTH_NORMAL;
        switch( eWidth )
        {
            case width::UltraCondensed: nWidth = FC_WIDTH_ULTRACONDENSED;break;
            case width::ExtraCondensed: nWidth = FC_WIDTH_EXTRACONDENSED;break;
            case width::Condensed:      nWidth = FC_WIDTH_CONDENSED;break;
            case width::SemiCondensed:  nWidth = FC_WIDTH_SEMICONDENSED;break;
            case width::Normal:         nWidth = FC_WIDTH_NORMAL;break;
            case width::SemiExpanded:   nWidth = FC_WIDTH_SEMIEXPANDED;break;
            case width::Expanded:       nWidth = FC_WIDTH_EXPANDED;break;
            case width::ExtraExpanded:  nWidth = FC_WIDTH_EXTRAEXPANDED;break;
            case width::UltraExpanded:  nWidth = FC_WIDTH_ULTRACONDENSED;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_WIDTH, nWidth );
    }
    if( ePitch != pitch::Unknown )
    {
        int nSpacing = FC_PROPORTIONAL;
        switch( ePitch )
        {
            case pitch::Fixed:          nSpacing = FC_MONO;break;
            case pitch::Variable:       nSpacing = FC_PROPORTIONAL;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_SPACING, nSpacing );
        if (nSpacing == FC_MONO)
            rWrapper.FcPatternAddString( pPattern, FC_FAMILY, (FcChar8*)"monospace");
    }
}

rtl::OUString PrintFontManager::Substitute(const rtl::OUString& rFontName,
    rtl::OUString& rMissingCodes, const rtl::OString &rLangAttrib,
    italic::type eItalic, weight::type eWeight,
    width::type eWidth, pitch::type ePitch) const
{
    rtl::OUString aName;
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return aName;

    // build pattern argument for fontconfig query
    FcPattern* pPattern = rWrapper.FcPatternCreate();

    // Prefer scalable fonts
    rWrapper.FcPatternAddBool( pPattern, FC_SCALABLE, FcTrue );

    const rtl::OString aTargetName = rtl::OUStringToOString( rFontName, RTL_TEXTENCODING_UTF8 );
    const FcChar8* pTargetNameUtf8 = (FcChar8*)aTargetName.getStr();
    rWrapper.FcPatternAddString( pPattern, FC_FAMILY, pTargetNameUtf8 );

    const FcChar8* pLangAttribUtf8 = (FcChar8*)rLangAttrib.getStr();
    if( rLangAttrib.getLength() )
        rWrapper.FcPatternAddString( pPattern, FC_LANG, pLangAttribUtf8 );

    // Add required Unicode characters, if any
    if ( rMissingCodes.getLength() )
    {
       FcCharSet *unicodes = rWrapper.FcCharSetCreate();
       for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
       {
           // also handle unicode surrogates
           const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
           rWrapper.FcCharSetAddChar( unicodes, nCode );
       }
       rWrapper.FcPatternAddCharSet( pPattern, FC_CHARSET, unicodes);
       rWrapper.FcCharSetDestroy( unicodes );
    }

    addtopattern(rWrapper, pPattern, eItalic, eWeight, eWidth, ePitch);

    // query fontconfig for a substitute
    rWrapper.FcConfigSubstitute( rWrapper.getDefConfig(), pPattern, FcMatchPattern );
    rWrapper.FcDefaultSubstitute( pPattern );

    // process the result of the fontconfig query
    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = rWrapper.FcFontSetMatch( rWrapper.getDefConfig(), &pFontSet, 1, pPattern, &eResult );
    rWrapper.FcPatternDestroy( pPattern );

    FcFontSet*  pSet = NULL;
    if( pResult )
    {
        pSet = rWrapper.FcFontSetCreate();
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        rWrapper.FcFontSetAdd( pSet, pResult );
    }

    if( pSet )
    {
        if( pSet->nfont > 0 )
        {
            //extract the closest match
            FcChar8* family = NULL;
            FcResult eFileRes = rWrapper.FcPatternGetString( pSet->fonts[0], FC_FAMILY, 0, &family );

            // get the family name
            if( eFileRes == FcResultMatch )
            {
                OString sFamily((sal_Char*)family);
                std::hash_map< rtl::OString, rtl::OString, rtl::OStringHash >::const_iterator aI = rWrapper.m_aFontconfigNameToLocalized.find(sFamily);
                if (aI != rWrapper.m_aFontconfigNameToLocalized.end())
                    sFamily = aI->second;
                aName = rtl::OStringToOUString( sFamily, RTL_TEXTENCODING_UTF8 );
            }

            // update rMissingCodes by removing resolved unicodes
            if( rMissingCodes.getLength() > 0 )
            {
                sal_uInt32* pRemainingCodes = (sal_uInt32*)alloca( rMissingCodes.getLength() * sizeof(sal_uInt32) );
                int nRemainingLen = 0;
                FcCharSet* unicodes;
                if( !rWrapper.FcPatternGetCharSet( pSet->fonts[0], FC_CHARSET, 0, &unicodes ) )
                {
                       for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
                    {
                        // also handle unicode surrogates
                        const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
                        if( rWrapper.FcCharSetHasChar( unicodes, nCode ) != FcTrue )
                            pRemainingCodes[ nRemainingLen++ ] = nCode;
                    }
                }
                rMissingCodes = OUString( pRemainingCodes, nRemainingLen );
            }
        }

        rWrapper.FcFontSetDestroy( pSet );
    }

    return aName;
}

bool PrintFontManager::matchFont( FastPrintFontInfo& rInfo, const com::sun::star::lang::Locale& rLocale )
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return false;

    FcConfig* pConfig = rWrapper.getDefConfig();
    FcPattern* pPattern = rWrapper.FcPatternCreate();

    OString aLangAttrib;
    // populate pattern with font characteristics
    if( rLocale.Language.getLength() )
    {
        OUStringBuffer aLang(6);
        aLang.append( rLocale.Language );
        if( rLocale.Country.getLength() )
        {
            aLang.append( sal_Unicode('-') );
            aLang.append( rLocale.Country );
        }
        aLangAttrib = OUStringToOString( aLang.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
    }
    if( aLangAttrib.getLength() )
        rWrapper.FcPatternAddString( pPattern, FC_LANG, (FcChar8*)aLangAttrib.getStr() );

    OString aFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );
    if( aFamily.getLength() )
        rWrapper.FcPatternAddString( pPattern, FC_FAMILY, (FcChar8*)aFamily.getStr() );

    addtopattern(rWrapper, pPattern, rInfo.m_eItalic, rInfo.m_eWeight, rInfo.m_eWidth, rInfo.m_ePitch);

    rWrapper.FcConfigSubstitute( pConfig, pPattern, FcMatchPattern );
    rWrapper.FcDefaultSubstitute( pPattern );
    FcResult eResult = FcResultNoMatch;
    FcFontSet *pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = rWrapper.FcFontSetMatch( pConfig, &pFontSet, 1, pPattern, &eResult );
    bool bSuccess = false;
    if( pResult )
    {
        FcFontSet* pSet = rWrapper.FcFontSetCreate();
        rWrapper.FcFontSetAdd( pSet, pResult );
        if( pSet->nfont > 0 )
        {
            //extract the closest match
            FcChar8* file = NULL;
            FcResult eFileRes = rWrapper.FcPatternGetString( pSet->fonts[0], FC_FILE, 0, &file );
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( (sal_Char*)file );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir, true );
                fontID aFont = findFontFileID( nDirID, aBase );
                if( aFont > 0 )
                    bSuccess = getFontFastInfo( aFont, rInfo );
            }
        }
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        rWrapper.FcFontSetDestroy( pSet );
    }

    // cleanup
    rWrapper.FcPatternDestroy( pPattern );

    return bSuccess;
}

#else // ENABLE_FONTCONFIG not defined

bool PrintFontManager::initFontconfig()
{
    return false;
}

int PrintFontManager::countFontconfigFonts()
{
    return 0;
}

void PrintFontManager::deinitFontconfig()
{}

bool PrintFontManager::addFontconfigDir( const rtl::OString& )
{
    return false;
}

bool PrintFontManager::matchFont( FastPrintFontInfo&, const com::sun::star::lang::Locale& )
{
    return false;
}

int PrintFontManager::FreeTypeCharIndex( void*, sal_uInt32 )
{
    return 0;
}

rtl::OUString PrintFontManager::Substitute( const rtl::OUString&,
    rtl::OUString&, const rtl::OString&, italic::type, weight::type, width::type, pitch::type) const
{
    rtl::OUString aName;
    return aName;
}

#endif // ENABLE_FONTCONFIG

