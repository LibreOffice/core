/*************************************************************************
 *
 *  $RCSfile: fontconfig.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:08:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef ENABLE_FONTCONFIG
#include <fontconfig/fontconfig.h>
// be compatible with fontconfig 2.2.0 release
#ifndef FC_WEIGHT_BOOK
#define FC_WEIGHT_BOOK 75
#endif
#else
typedef void FcConfig;
typedef void FcObjectSet;
typedef void FcPattern;
typedef void FcFontSet;
typedef int FcResult;
typedef int FcBool;
typedef int FcMatchKind;
typedef char FcChar8;
#endif

#include <cstdio>
#include <cstdarg>

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PSPRINT_FONTCACHE_HXX_
#include <psprint/fontcache.hxx>
#endif
#ifndef _UTL_ATOM_HXX_
#include <unotools/atom.hxx>
#endif
#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif
#ifndef _OSL_THREAD_H
#include <osl/thread.h>
#endif

using namespace psp;
using namespace osl;
using namespace rtl;

class FontCfgWrapper
{
    void*           m_pLib;
    FcConfig*       m_pDefConfig;

    FcConfig*       (*m_pFcInitLoadConfigAndFonts)();
    FcObjectSet*    (*m_pFcObjectSetVaBuild)(const char*,va_list);
    void            (*m_pFcObjectSetDestroy)(FcObjectSet* pSet);
    FcPattern*      (*m_pFcPatternCreate)();
    void            (*m_pFcPatternDestroy)(FcPattern*);
    FcFontSet*      (*m_pFcFontList)(FcConfig*,FcPattern*,FcObjectSet*);
    FcFontSet*      (*m_pFcFontSetCreate)();
    void            (*m_pFcFontSetDestroy)(FcFontSet*);
    FcBool          (*m_pFcFontSetAdd)(FcFontSet*,FcPattern*);
    FcResult        (*m_pFcPatternGetString)(const FcPattern*,const char*,int,FcChar8**);
    FcResult        (*m_pFcPatternGetInteger)(const FcPattern*,const char*,int,int*);
    FcResult        (*m_pFcPatternGetDouble)(const FcPattern*,const char*,int,double*);
    FcResult        (*m_pFcPatternGetBool)(const FcPattern*,const char*,int,FcBool*);
    void            (*m_pFcDefaultSubstitute)(FcPattern *);
    FcPattern*      (*m_pFcFontMatch)(FcConfig*,FcPattern*,FcResult*);
    FcBool          (*m_pFcConfigSubstitute)(FcConfig*,FcPattern*,FcMatchKind);
    FcBool          (*m_pFcPatternAddInteger)(FcPattern*,const char*,int);
    FcBool          (*m_pFcPatternAddString)(FcPattern*,const char*,const FcChar8*);

    void* loadSymbol( const char* );

    FontCfgWrapper();
    ~FontCfgWrapper();

public:
    static FontCfgWrapper& get();
    static void release();

    bool isValid() const
    { return m_pLib != NULL;}

    FcConfig* getDefConfig() { return m_pDefConfig; }


    FcConfig* FcInitLoadConfigAndFonts()
    { return m_pFcInitLoadConfigAndFonts(); }

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

    FcFontSet* FcFontSetCreate()
    { return m_pFcFontSetCreate(); }
    void FcFontSetDestroy( FcFontSet* pSet )
    { m_pFcFontSetDestroy( pSet );}
    FcBool FcFontSetAdd( FcFontSet* pSet, FcPattern* pPattern )
    { return m_pFcFontSetAdd( pSet, pPattern ); }

    FcResult FcPatternGetString( const FcPattern* pPattern, const char* object, int n, FcChar8** s )
    { return m_pFcPatternGetString( pPattern, object, n, s ); }

    FcResult FcPatternGetInteger( const FcPattern* pPattern, const char* object, int n, int* s )
    { return m_pFcPatternGetInteger( pPattern, object, n, s ); }

    FcResult FcPatternGetDouble( const FcPattern* pPattern, const char* object, int n, double* s )
    { return m_pFcPatternGetDouble( pPattern, object, n, s ); }

    FcResult FcPatternGetBool( const FcPattern* pPattern, const char* object, int n, FcBool* s )
    { return m_pFcPatternGetBool( pPattern, object, n, s ); }
    void FcDefaultSubstitute( FcPattern* pPattern )
    { m_pFcDefaultSubstitute( pPattern ); }
    FcPattern* FcFontMatch( FcConfig* pConfig, FcPattern* pPattern, FcResult* pResult )
    { return m_pFcFontMatch( pConfig, pPattern, pResult ); }
    FcBool FcConfigSubstitute( FcConfig* pConfig, FcPattern* pPattern, FcMatchKind eKind )
    { return m_pFcConfigSubstitute( pConfig, pPattern, eKind ); }
    FcBool FcPatternAddInteger( FcPattern* pPattern, const char* pObject, int nValue )
    { return m_pFcPatternAddInteger( pPattern, pObject, nValue ); }
    FcBool FcPatternAddString( FcPattern* pPattern, const char* pObject, const FcChar8* pString )
    { return m_pFcPatternAddString( pPattern, pObject, pString ); }
};

void* FontCfgWrapper::loadSymbol( const char* pSymbol )
{
    OUString aSym( OUString::createFromAscii( pSymbol ) );
    void* pSym = osl_getSymbol( m_pLib, aSym.pData );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s %s\n", pSymbol, pSym ? "found" : "not found" );
#endif
    return pSym;
}

FontCfgWrapper::FontCfgWrapper()
        : m_pLib( NULL ),
          m_pDefConfig( NULL )
{
#ifdef ENABLE_FONTCONFIG
    OUString aLib( RTL_CONSTASCII_USTRINGPARAM( "libfontconfig.so.1" ) );
    m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    if( !m_pLib )
    {
        aLib = OUString( RTL_CONSTASCII_USTRINGPARAM( "libfontconfig.so" ) );
        m_pLib = osl_loadModule( aLib.pData, SAL_LOADMODULE_LAZY );
    }
#endif

    if( ! m_pLib )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "no libfontconfig\n" );
#endif
        return;
    }

    m_pFcInitLoadConfigAndFonts = (FcConfig*(*)())
        loadSymbol( "FcInitLoadConfigAndFonts" );
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
    m_pFcFontSetCreate = (FcFontSet*(*)())
        loadSymbol( "FcFontSetCreate" );
    m_pFcFontSetDestroy = (void(*)(FcFontSet*))
        loadSymbol( "FcFontSetDestroy" );
    m_pFcFontSetAdd = (FcBool(*)(FcFontSet*,FcPattern*))
        loadSymbol( "FcFontSetAdd" );
    m_pFcPatternGetString = (FcResult(*)(const FcPattern*,const char*,int,FcChar8**))
        loadSymbol( "FcPatternGetString" );
    m_pFcPatternGetInteger = (FcResult(*)(const FcPattern*,const char*,int,int*))
        loadSymbol( "FcPatternGetInteger" );
    m_pFcPatternGetDouble = (FcResult(*)(const FcPattern*,const char*,int,double*))
        loadSymbol( "FcPatternGetDouble" );
    m_pFcPatternGetBool = (FcResult(*)(const FcPattern*,const char*,int,FcBool*))
        loadSymbol( "FcPatternGetBool" );
    m_pFcDefaultSubstitute = (void(*)(FcPattern *))
        loadSymbol( "FcDefaultSubstitute" );
    m_pFcFontMatch = (FcPattern*(*)(FcConfig*,FcPattern*,FcResult*))
        loadSymbol( "FcFontMatch" );
    m_pFcConfigSubstitute = (FcBool(*)(FcConfig*,FcPattern*,FcMatchKind))
        loadSymbol( "FcConfigSubstitute" );
    m_pFcPatternAddInteger = (FcBool(*)(FcPattern*,const char*,int))
        loadSymbol( "FcPatternAddInteger" );
    m_pFcPatternAddString = (FcBool(*)(FcPattern*,const char*,const FcChar8*))
        loadSymbol( "FcPatternAddString" );

    if( ! (
            m_pFcInitLoadConfigAndFonts     &&
            m_pFcObjectSetVaBuild           &&
            m_pFcObjectSetDestroy           &&
            m_pFcPatternCreate              &&
            m_pFcPatternDestroy             &&
            m_pFcFontList                   &&
            m_pFcFontSetCreate              &&
            m_pFcFontSetDestroy             &&
            m_pFcFontSetAdd                 &&
            m_pFcPatternGetString           &&
            m_pFcPatternGetInteger          &&
            m_pFcPatternGetDouble           &&
            m_pFcPatternGetBool             &&
            m_pFcDefaultSubstitute          &&
            m_pFcFontMatch                  &&
            m_pFcConfigSubstitute           &&
            m_pFcPatternAddInteger          &&
            m_pFcPatternAddString
            ) )
     {
         osl_unloadModule( m_pLib );
         m_pLib = NULL;
#if OSL_DEBUG_LEVEL > 1
         fprintf( stderr, "not all needed symbols were found in libfontconfig\n" );
#endif
     }

    m_pDefConfig = FcInitLoadConfigAndFonts();
    if( ! m_pDefConfig )
    {
        osl_unloadModule( m_pLib );
        m_pLib = NULL;
    }
}

FontCfgWrapper::~FontCfgWrapper()
{
    if( m_pLib )
        osl_unloadModule( m_pLib );
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

/*
 * PrintFontManager::initFontconfig
 */
bool PrintFontManager::initFontconfig()
{
    int nFonts = 0;

#ifdef ENABLE_FONTCONFIG
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return false;

    FcConfig* pConfig = rWrapper.getDefConfig();
    FcObjectSet* pOSet = rWrapper.FcObjectSetBuild( FC_FAMILY,
                                                    FC_STYLE,
                                                    FC_SLANT,
                                                    FC_WEIGHT,
                                                    FC_SPACING,
                                                    FC_FILE,
                                                    FC_OUTLINE,
                                                    FC_INDEX,
                                                    NULL );
    FcPattern* pPattern = rWrapper.FcPatternCreate();
    FcFontSet* pFSet = rWrapper.FcFontList( pConfig, pPattern, pOSet );

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
            FcBool outline = false;

            FcResult eFileRes   = rWrapper.FcPatternGetString( pFSet->fonts[i], FC_FILE, 0, &file );
            FcResult eFamilyRes = rWrapper.FcPatternGetString( pFSet->fonts[i], FC_FAMILY, 0, &family );
            FcResult eStyleRes  = rWrapper.FcPatternGetString( pFSet->fonts[i], FC_STYLE, 0, &style );
            FcResult eSlantRes  = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SLANT, 0, &slant );
            FcResult eWeightRes = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_WEIGHT, 0, &weight );
            FcResult eSpacRes   = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SPACING, 0, &spacing );
            FcResult eOutRes    = rWrapper.FcPatternGetBool( pFSet->fonts[i], FC_OUTLINE, 0, &outline );
            FcResult eIndexRes = rWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_INDEX, 0, &nCollectionEntry );

            if( eFileRes != FcResultMatch || eFamilyRes != FcResultMatch || eOutRes != FcResultMatch )
                continue;

#if OSL_DEBUG_LEVEL > 1
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

            // only outline fonts are usable to psprint anyway
            if( eOutRes == FcResultMatch && ! outline )
                continue;

            // see if this font is already cached
            // update attributes
            std::list< PrintFont* > aFonts;
            OString aDir, aBase, aOrgPath( (sal_Char*)file );
            splitPath( aOrgPath, aDir, aBase );
            int nDirID = getDirectoryAtom( aDir, true );
            if( ! m_pFontCache->getFontCacheFile( nDirID, aDir, aBase, aFonts ) )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "file %s not cached\n", aBase.getStr() );
#endif
                // not known, analyze font file to get attributes
                // not described by fontconfig (e.g. alias names, PSName)
                std::list< OString > aDummy;
                analyzeFontFile( nDirID, aBase, true, aDummy, aFonts );
#if OSL_DEBUG_LEVEL > 1
                if( aFonts.empty() )
                    fprintf( stderr, "Warning: file is unusable to psprint\n" );
#endif
            }
            if( aFonts.empty() )
                continue;

            int nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( OString( (sal_Char*)family ), osl_getThreadTextEncoding() ), sal_True );
            PrintFont* pUpdate = aFonts.front();
            if( ++aFonts.begin() != aFonts.end() ) // more than one font
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

                // update font cache
                m_pFontCache->updateFontCacheEntry( pUpdate, false );
                // sort into known fonts
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = pUpdate;
                m_aFontFileToFontID[ aBase ].insert( aFont );
                nFonts++;
#if OSL_DEBUG_LEVEL > 1
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

    // cleanup
    if( pPattern )
        rWrapper.FcPatternDestroy( pPattern );
    if( pFSet )
        rWrapper.FcFontSetDestroy( pFSet );
    if( pOSet )
        rWrapper.FcObjectSetDestroy( pOSet );

    // how does one get rid of the config ?
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "inserted %d fonts from fontconfig\n", nFonts );
#endif
#endif // ENABLE_FONTCONFIG
    return nFonts != 0;
}

void PrintFontManager::deinitFontconfig()
{
    FontCfgWrapper::release();
}

bool PrintFontManager::matchFont( FastPrintFontInfo& rInfo )
{
#ifdef ENABLE_FONTCONFIG
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    if( ! rWrapper.isValid() )
        return false;

    FcConfig* pConfig = rWrapper.getDefConfig();
    FcPattern* pPattern = rWrapper.FcPatternCreate();

    // populate pattern with font characteristics
    OString aFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );
    if( aFamily.getLength() )
        rWrapper.FcPatternAddString( pPattern, FC_FAMILY, (FcChar8*)aFamily.getStr() );
    if( rInfo.m_eItalic != italic::Unknown )
    {
        int nSlant = FC_SLANT_ROMAN;
        switch( rInfo.m_eItalic )
        {
            case italic::Italic:        nSlant = FC_SLANT_ITALIC;break;
            case italic::Oblique:       nSlant = FC_SLANT_OBLIQUE;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_SLANT, nSlant );
    }
    if( rInfo.m_eWeight != weight::Unknown )
    {
        int nWeight = FC_WEIGHT_NORMAL;
        switch( rInfo.m_eWeight )
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
    if( rInfo.m_eWidth != width::Unknown )
    {
        int nWidth = FC_WIDTH_NORMAL;
        switch( rInfo.m_eWidth )
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
    if( rInfo.m_ePitch != pitch::Unknown )
    {
        int nSpacing = FC_PROPORTIONAL;
        switch( rInfo.m_ePitch )
        {
            case pitch::Fixed:          nSpacing = FC_MONO;break;
            case pitch::Variable:       nSpacing = FC_PROPORTIONAL;break;
            default:
                break;
        }
        rWrapper.FcPatternAddInteger( pPattern, FC_SPACING, nSpacing );
    }

    rWrapper.FcConfigSubstitute( pConfig, pPattern, FcMatchPattern );
    rWrapper.FcDefaultSubstitute( pPattern );
    FcResult eResult = FcResultNoMatch;
    FcPattern* pResult = rWrapper.FcFontMatch( pConfig, pPattern, &eResult );
    bool bSuccess = false;
    if( pResult )
    {
        FcFontSet* pSet = rWrapper.FcFontSetCreate();
        rWrapper.FcFontSetAdd( pSet, pResult );
        if( pSet->nfont > 0 )
        {
            //extract the closest match
            FcChar8* file = NULL;
            FcResult eFileRes   = rWrapper.FcPatternGetString( pSet->fonts[0], FC_FILE, 0, &file );
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
#else
    return false;
#endif
}
