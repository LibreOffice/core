/*************************************************************************
 *
 *  $RCSfile: fontconfig.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-01 09:35:26 $
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
#else
typedef void FcConfig;
typedef void FcObjectSet;
typedef void FcPattern;
typedef void FcFontSet;
typedef int FcResult;
typedef int FcBool;
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

    FcConfig*       (*m_pFcInitLoadConfigAndFonts)();
    FcObjectSet*    (*m_pFcObjectSetVaBuild)(const char*,va_list);
    void            (*m_pFcObjectSetDestroy)(FcObjectSet* pSet);
    FcPattern*      (*m_pFcPatternCreate)();
    void            (*m_pFcPatternDestroy)(FcPattern*);
    FcFontSet*      (*m_pFcFontList)(FcConfig*,FcPattern*,FcObjectSet*);
    void            (*m_pFcFontSetDestroy)(FcFontSet*);
    FcResult        (*m_pFcPatternGetString)(const FcPattern*,const char*,int,FcChar8**);
    FcResult        (*m_pFcPatternGetInteger)(const FcPattern*,const char*,int,int*);
    FcResult        (*m_pFcPatternGetDouble)(const FcPattern*,const char*,int,double*);
    FcResult        (*m_pFcPatternGetBool)(const FcPattern*,const char*,int,FcBool*);

    void* loadSymbol( const char* );
public:
    FontCfgWrapper();
    ~FontCfgWrapper();

    bool isValid() const
    { return m_pLib != NULL;}

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

    void FcFontSetDestroy( FcFontSet* pSet )
    { m_pFcFontSetDestroy( pSet );}

    FcResult FcPatternGetString( const FcPattern* pPattern, const char* object, int n, FcChar8** s )
    { return m_pFcPatternGetString( pPattern, object, n, s ); }

    FcResult FcPatternGetInteger( const FcPattern* pPattern, const char* object, int n, int* s )
    { return m_pFcPatternGetInteger( pPattern, object, n, s ); }

    FcResult FcPatternGetDouble( const FcPattern* pPattern, const char* object, int n, double* s )
    { return m_pFcPatternGetDouble( pPattern, object, n, s ); }

    FcResult FcPatternGetBool( const FcPattern* pPattern, const char* object, int n, FcBool* s )
    { return m_pFcPatternGetBool( pPattern, object, n, s ); }
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
        : m_pLib( NULL )
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
    m_pFcFontSetDestroy = (void(*)(FcFontSet*))
        loadSymbol( "FcFontSetDestroy" );
    m_pFcPatternGetString = (FcResult(*)(const FcPattern*,const char*,int,FcChar8**))
        loadSymbol( "FcPatternGetString" );
    m_pFcPatternGetInteger = (FcResult(*)(const FcPattern*,const char*,int,int*))
        loadSymbol( "FcPatternGetInteger" );
    m_pFcPatternGetDouble = (FcResult(*)(const FcPattern*,const char*,int,double*))
        loadSymbol( "FcPatternGetDouble" );
    m_pFcPatternGetBool = (FcResult(*)(const FcPattern*,const char*,int,FcBool*))
        loadSymbol( "FcPatternGetBool" );

    if( ! (
            m_pFcInitLoadConfigAndFonts     &&
            m_pFcObjectSetVaBuild           &&
            m_pFcObjectSetDestroy           &&
            m_pFcPatternCreate              &&
            m_pFcPatternDestroy             &&
            m_pFcFontList                   &&
            m_pFcFontSetDestroy             &&
            m_pFcPatternGetString           &&
            m_pFcPatternGetInteger          &&
            m_pFcPatternGetDouble           &&
            m_pFcPatternGetBool
            ) )
     {
         osl_unloadModule( m_pLib );
         m_pLib = NULL;
#if OSL_DEBUG_LEVEL > 1
         fprintf( stderr, "not all needed symbols were found in libfontconfig\n" );
#endif
     }
}

FontCfgWrapper::~FontCfgWrapper()
{
    if( m_pLib )
        osl_unloadModule( m_pLib );
}

/*
 * PrintFontManager::initFontconfig
 */
bool PrintFontManager::initFontconfig()
{
    int nFonts = 0;

#ifdef ENABLE_FONTCONFIG
    FontCfgWrapper aWrapper;
    if( ! aWrapper.isValid() )
        return false;

    FcConfig* pConfig = aWrapper.FcInitLoadConfigAndFonts();
    if( ! pConfig )
        return false;

    FcObjectSet* pOSet = aWrapper.FcObjectSetBuild( FC_FAMILY,
                                                    FC_STYLE,
                                                    FC_SLANT,
                                                    FC_WEIGHT,
                                                    FC_SPACING,
                                                    FC_FILE,
                                                    FC_OUTLINE,
                                                    NULL );
    FcPattern* pPattern = aWrapper.FcPatternCreate();
    FcFontSet* pFSet = aWrapper.FcFontList( pConfig, pPattern, pOSet );

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
            FcBool outline = false;

            FcResult eFileRes   = aWrapper.FcPatternGetString( pFSet->fonts[i], FC_FILE, 0, &file );
            FcResult eFamilyRes = aWrapper.FcPatternGetString( pFSet->fonts[i], FC_FAMILY, 0, &family );
            FcResult eStyleRes  = aWrapper.FcPatternGetString( pFSet->fonts[i], FC_STYLE, 0, &style );
            FcResult eSlantRes  = aWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SLANT, 0, &slant );
            FcResult eWeightRes = aWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_WEIGHT, 0, &weight );
            FcResult eSpacRes   = aWrapper.FcPatternGetInteger( pFSet->fonts[i], FC_SPACING, 0, &spacing );
            FcResult eOutRes    = aWrapper.FcPatternGetBool( pFSet->fonts[i], FC_OUTLINE, 0, &outline );


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
            }
#if OSL_DEBUG_LEVEL > 1
            if( aFonts.size() > 1 )
                fprintf( stderr, "Warning: file contains more than one font, updating all to fontconfig data\n" );
            else if( aFonts.empty() )
                fprintf( stderr, "Warning: file is unusable to psprint\n" );
#endif
            int nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( OString( (sal_Char*)family ), osl_getThreadTextEncoding() ), sal_True );

            for( std::list< PrintFont* >::iterator it = aFonts.begin();
                 it != aFonts.end(); ++it )
            {
                // set family name
                if( (*it)->m_nFamilyName != nFamilyName )
                {
                    (*it)->m_aAliases.remove( (*it)->m_nFamilyName );
                    (*it)->m_aAliases.push_back( (*it)->m_nFamilyName );
                    (*it)->m_aAliases.remove( nFamilyName );
                    (*it)->m_nFamilyName = nFamilyName;
                }
                if( eWeightRes == FcResultMatch )
                {
                    // set weight
                    if( weight == FC_WEIGHT_LIGHT )
                        (*it)->m_eWeight = weight::Light;
                    else if( weight == FC_WEIGHT_MEDIUM )
                        (*it)->m_eWeight = weight::Medium;
                    else if( weight == FC_WEIGHT_DEMIBOLD )
                        (*it)->m_eWeight = weight::SemiBold;
                    else if( weight == FC_WEIGHT_BOLD )
                        (*it)->m_eWeight = weight::Bold;
                    else if( weight == FC_WEIGHT_BLACK )
                        (*it)->m_eWeight = weight::Black;
                }
                if( eSpacRes == FcResultMatch )
                {
                    // set pitch
                    if( spacing == FC_PROPORTIONAL )
                        (*it)->m_ePitch = pitch::Variable;
                    else if( spacing == FC_MONO || spacing == FC_CHARCELL )
                        (*it)->m_ePitch = pitch::Fixed;
                }
                if( eSlantRes == FcResultMatch )
                {
                    // set italic
                    if( slant == FC_SLANT_ROMAN )
                        (*it)->m_eItalic = italic::Upright;
                    else if( slant == FC_SLANT_ITALIC )
                        (*it)->m_eItalic = italic::Italic;
                    else if( slant == FC_SLANT_OBLIQUE )
                        (*it)->m_eItalic = italic::Oblique;
                }

                // update font cache
                m_pFontCache->updateFontCacheEntry( *it, false );
                // sort into known fonts
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = *it;
                m_aFontFileToFontID[ aBase ].insert( aFont );
                nFonts++;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "inserted font %s as fontID %d\n", family, aFont );
#endif
            }
        }
    }


    // cleanup
    if( pPattern )
        aWrapper.FcPatternDestroy( pPattern );
    if( pFSet )
        aWrapper.FcFontSetDestroy( pFSet );
    if( pOSet )
        aWrapper.FcObjectSetDestroy( pOSet );

    // how does one get rid of the config ?
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "inserted %d fonts from fontconfig\n", nFonts );
#endif
#endif // ENABLE_FONTCONFIG
    return nFonts != 0;
}
