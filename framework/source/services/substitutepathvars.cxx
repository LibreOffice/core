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

#include <config_folders.h>

#include <comphelper/lok.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/thread.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Paths.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include <unordered_map>

using namespace com::sun::star::uno;
using namespace com::sun::star::container;

namespace {

enum PreDefVariable
{
    PREDEFVAR_INST,
    PREDEFVAR_PROG,
    PREDEFVAR_USER,
    PREDEFVAR_WORK,
    PREDEFVAR_HOME,
    PREDEFVAR_TEMP,
    PREDEFVAR_PATH,
    PREDEFVAR_USERNAME,
    PREDEFVAR_LANGID,
    PREDEFVAR_VLANG,
    PREDEFVAR_INSTPATH,
    PREDEFVAR_PROGPATH,
    PREDEFVAR_USERPATH,
    PREDEFVAR_INSTURL,
    PREDEFVAR_PROGURL,
    PREDEFVAR_USERURL,
    PREDEFVAR_WORKDIRURL,
    // New variable of hierarchy service (#i32656#)
    PREDEFVAR_BASEINSTURL,
    PREDEFVAR_USERDATAURL,
    PREDEFVAR_BRANDBASEURL,
    PREDEFVAR_COUNT
};

struct FixedVariable
{
    const char*     pVarName;
    bool            bAbsPath;
};

// Table with all fixed/predefined variables supported.
const FixedVariable aFixedVarTable[PREDEFVAR_COUNT] =
{
    { "$(inst)",         true  }, // PREDEFVAR_INST
    { "$(prog)",         true  }, // PREDEFVAR_PROG
    { "$(user)",         true  }, // PREDEFVAR_USER
    { "$(work)",         true  }, // PREDEFVAR_WORK, special variable
                                  //  (transient)
    { "$(home)",         true  }, // PREDEFVAR_HOME
    { "$(temp)",         true  }, // PREDEFVAR_TEMP
    { "$(path)",         true  }, // PREDEFVAR_PATH
    { "$(username)",     false }, // PREDEFVAR_USERNAME
    { "$(langid)",       false }, // PREDEFVAR_LANGID
    { "$(vlang)",        false }, // PREDEFVAR_VLANG
    { "$(instpath)",     true  }, // PREDEFVAR_INSTPATH
    { "$(progpath)",     true  }, // PREDEFVAR_PROGPATH
    { "$(userpath)",     true  }, // PREDEFVAR_USERPATH
    { "$(insturl)",      true  }, // PREDEFVAR_INSTURL
    { "$(progurl)",      true  }, // PREDEFVAR_PROGURL
    { "$(userurl)",      true  }, // PREDEFVAR_USERURL
    { "$(workdirurl)",   true  }, // PREDEFVAR_WORKDIRURL, special variable
                                  //  (transient) and don't use for
                                  //  resubstitution
    { "$(baseinsturl)",  true  }, // PREDEFVAR_BASEINSTURL
    { "$(userdataurl)",  true  }, // PREDEFVAR_USERDATAURL
    { "$(brandbaseurl)", true  }  // PREDEFVAR_BRANDBASEURL
};

struct PredefinedPathVariables
{
    // Predefined variables supported by substitute variables
    LanguageType    m_eLanguageType;               // Language type of Office
    OUString   m_FixedVar[ PREDEFVAR_COUNT ];      // Variable value access by PreDefVariable
    OUString   m_FixedVarNames[ PREDEFVAR_COUNT ]; // Variable name access by PreDefVariable
};

struct ReSubstFixedVarOrder
{
    sal_Int32       nVarValueLength;
    PreDefVariable  eVariable;

    bool operator< ( const ReSubstFixedVarOrder& aFixedVarOrder ) const
    {
        // Reverse operator< to have high to low ordering
        return ( nVarValueLength > aFixedVarOrder.nVarValueLength );
    }
};

typedef ::cppu::WeakComponentImplHelper<
    css::util::XStringSubstitution,
    css::lang::XServiceInfo > SubstitutePathVariables_BASE;

class SubstitutePathVariables : private cppu::BaseMutex,
                                public SubstitutePathVariables_BASE
{
public:
    explicit SubstitutePathVariables(const css::uno::Reference< css::uno::XComponentContext >& xContext);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.PathSubstitution";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.util.PathSubstitution"};
    }

    // XStringSubstitution
    virtual OUString SAL_CALL substituteVariables( const OUString& aText, sal_Bool bSubstRequired ) override;
    virtual OUString SAL_CALL reSubstituteVariables( const OUString& aText ) override;
    virtual OUString SAL_CALL getSubstituteVariableValue( const OUString& variable ) override;

protected:
    void            SetPredefinedPathVariables();

    // Special case (transient) values can change during runtime!
    // Don't store them in the pre defined struct
    OUString   GetWorkPath() const;
    OUString   GetWorkVariableValue() const;
    OUString   GetPathVariableValue() const;

    OUString   GetHomeVariableValue() const;

    // XStringSubstitution implementation methods
    /// @throws css::container::NoSuchElementException
    /// @throws css::uno::RuntimeException
    OUString impl_substituteVariable( const OUString& aText, bool bSustRequired );
    /// @throws css::uno::RuntimeException
    OUString impl_reSubstituteVariables( const OUString& aText );
    /// @throws css::container::NoSuchElementException
    /// @throws css::uno::RuntimeException
    OUString const & impl_getSubstituteVariableValue( const OUString& variable );

private:
    typedef std::unordered_map<OUString, PreDefVariable>
        VarNameToIndexMap;

    VarNameToIndexMap            m_aPreDefVarMap;         // Mapping from pre-def variable names to enum for array access
    PredefinedPathVariables      m_aPreDefVars;           // All predefined variables
    std::vector<ReSubstFixedVarOrder> m_aReSubstFixedVarOrder; // To speed up resubstitution fixed variables (order for lookup)
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

SubstitutePathVariables::SubstitutePathVariables( const Reference< XComponentContext >& xContext ) :
    SubstitutePathVariables_BASE(m_aMutex),
    m_xContext( xContext )
{
    SetPredefinedPathVariables();

    // Init the predefined/fixed variable to index hash map
    for ( int i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        // Store variable name into struct of predefined/fixed variables
        m_aPreDefVars.m_FixedVarNames[i] = OUString::createFromAscii( aFixedVarTable[i].pVarName );

        // Create hash map entry
        m_aPreDefVarMap.emplace( m_aPreDefVars.m_FixedVarNames[i], PreDefVariable(i) );
    }

    // Sort predefined/fixed variable to path length
    for ( int i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        if (( i != PREDEFVAR_WORKDIRURL ) && ( i != PREDEFVAR_PATH ))
        {
            // Special path variables, don't include into automatic resubstitution search!
            // $(workdirurl) is not allowed to resubstitute! This variable is the value of path settings entry
            // and it could be possible that it will be resubstituted by itself!!
            // Example: WORK_PATH=c:\test, $(workdirurl)=WORK_PATH => WORK_PATH=$(workdirurl) and this cannot be substituted!
            ReSubstFixedVarOrder aFixedVar;
            aFixedVar.eVariable       = PreDefVariable(i);
            aFixedVar.nVarValueLength = m_aPreDefVars.m_FixedVar[static_cast<sal_Int32>(aFixedVar.eVariable)].getLength();
            m_aReSubstFixedVarOrder.push_back( aFixedVar );
        }
    }
    sort(m_aReSubstFixedVarOrder.begin(),m_aReSubstFixedVarOrder.end());
}

// XStringSubstitution
OUString SAL_CALL SubstitutePathVariables::substituteVariables( const OUString& aText, sal_Bool bSubstRequired )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_substituteVariable( aText, bSubstRequired );
}

OUString SAL_CALL SubstitutePathVariables::reSubstituteVariables( const OUString& aText )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_reSubstituteVariables( aText );
}

OUString SAL_CALL SubstitutePathVariables::getSubstituteVariableValue( const OUString& aVariable )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_getSubstituteVariableValue( aVariable );
}

OUString SubstitutePathVariables::GetWorkPath() const
{
    OUString aWorkPath;
    css::uno::Reference< css::container::XHierarchicalNameAccess > xPaths(officecfg::Office::Paths::Paths::get(m_xContext), css::uno::UNO_QUERY_THROW);
    if (!(xPaths->getByHierarchicalName("['Work']/WritePath") >>= aWorkPath))
        // fallback in case config layer does not return a usable work dir value.
        aWorkPath = GetWorkVariableValue();

    return aWorkPath;
}

OUString SubstitutePathVariables::GetWorkVariableValue() const
{
    OUString aWorkPath;
    std::optional<OUString> x(officecfg::Office::Paths::Variables::Work::get(m_xContext));
    if (!x)
    {
        // fallback to $HOME in case platform dependent config layer does not return
        // a usable work dir value.
        osl::Security aSecurity;
        aSecurity.getHomeDir( aWorkPath );
    }
    else
        aWorkPath = *x;
    return aWorkPath;
}

OUString SubstitutePathVariables::GetHomeVariableValue() const
{
    osl::Security   aSecurity;
    OUString   aHomePath;

    aSecurity.getHomeDir( aHomePath );
    return aHomePath;
}

OUString SubstitutePathVariables::GetPathVariableValue() const
{
    OUString aRetStr;
    const char* pEnv = getenv( "PATH" );

    if ( pEnv )
    {
        const int PATH_EXTEND_FACTOR = 200;
        OUString       aTmp;
        OUString       aPathList( pEnv, strlen( pEnv ), osl_getThreadTextEncoding() );
        OUStringBuffer aPathStrBuffer( aPathList.getLength() * PATH_EXTEND_FACTOR / 100 );

        bool      bAppendSep = false;
        sal_Int32 nToken = 0;
        do
        {
            OUString sToken = aPathList.getToken(0, SAL_PATHSEPARATOR, nToken);
            if (!sToken.isEmpty() &&
                osl::FileBase::getFileURLFromSystemPath( sToken, aTmp ) ==
                osl::FileBase::RC::E_None )
            {
                if ( bAppendSep )
                    aPathStrBuffer.append( ";" ); // Office uses ';' as path separator
                aPathStrBuffer.append( aTmp );
                bAppendSep = true;
            }
        }
        while(nToken>=0);

        aRetStr = aPathStrBuffer.makeStringAndClear();
    }

    return aRetStr;
}

OUString SubstitutePathVariables::impl_substituteVariable( const OUString& rText, bool bSubstRequired )
{
    // This is maximal recursive depth supported!
    const sal_Int32 nMaxRecursiveDepth = 8;

    OUString   aWorkText = rText;
    OUString   aResult;

    // Use vector with strings to detect endless recursions!
    std::vector< OUString > aEndlessRecursiveDetector;

    // Search for first occurrence of "$(...".
    sal_Int32   nDepth = 0;
    bool        bSubstitutionCompleted = false;
    sal_Int32   nPosition = aWorkText.indexOf( "$(" );
    sal_Int32   nLength = 0; // = count of letters from "$(" to ")" in string
    bool        bVarNotSubstituted = false;

    // Have we found any variable like "$(...)"?
    if ( nPosition != -1 )
    {
        // Yes; Get length of found variable.
        // If no ")" was found - nLength is set to 0 by default! see before.
        sal_Int32 nEndPosition = aWorkText.indexOf( ')', nPosition );
        if ( nEndPosition != -1 )
            nLength = nEndPosition - nPosition + 1;
    }

    // Is there something to replace ?
    bool bWorkRetrieved       = false;
    bool bWorkDirURLRetrieved = false;
    while (nDepth < nMaxRecursiveDepth)
    {
        while ( ( nPosition != -1 ) && ( nLength > 3 ) ) // "$(" ")"
        {
            // YES; Get the next variable for replace.
            sal_Int32     nReplaceLength  = 0;
            OUString aReplacement;
            OUString aSubString      = aWorkText.copy( nPosition, nLength );

            // Path variables are not case sensitive!
            OUString aSubVarString = aSubString.toAsciiLowerCase();
            VarNameToIndexMap::const_iterator pNTOIIter = m_aPreDefVarMap.find( aSubVarString );
            if ( pNTOIIter != m_aPreDefVarMap.end() )
            {
                // Fixed/Predefined variable found
                PreDefVariable nIndex = pNTOIIter->second;

                // Determine variable value and length from array/table
                if ( nIndex == PREDEFVAR_WORK && !bWorkRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ nIndex ] = GetWorkVariableValue();
                    bWorkRetrieved = true;
                }
                else if ( nIndex == PREDEFVAR_WORKDIRURL && !bWorkDirURLRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ nIndex ] = GetWorkPath();
                    bWorkDirURLRetrieved = true;
                }

                // Check preconditions to substitute path variables.
                // 1. A path variable can only be substituted if it follows a ';'!
                // 2. It's located exactly at the start of the string being substituted!
                if (( aFixedVarTable[ int( nIndex ) ].bAbsPath && (( nPosition == 0 ) || (( nPosition > 0 ) && ( aWorkText[nPosition-1] == ';')))) ||
                    ( !aFixedVarTable[ int( nIndex ) ].bAbsPath ))
                {
                    aReplacement = m_aPreDefVars.m_FixedVar[ nIndex ];
                    nReplaceLength = nLength;
                }
            }

            // Have we found something to replace?
            if ( nReplaceLength > 0 )
            {
                // Yes ... then do it.
                aWorkText = aWorkText.replaceAt( nPosition, nReplaceLength, aReplacement );
            }
            else
            {
                // Variable not known
                bVarNotSubstituted = true;
                nPosition += nLength;
            }

            // Step after replaced text! If no text was replaced (unknown variable!),
            // length of aReplacement is 0 ... and we don't step then.
            nPosition += aReplacement.getLength();

            // We must control index in string before call something at OUString!
            // The OUString-implementation don't do it for us :-( but the result is not defined otherwise.
            if ( nPosition + 1 > aWorkText.getLength() )
            {
                // Position is out of range. Break loop!
                nPosition = -1;
                nLength = 0;
            }
            else
            {
                // Else; Position is valid. Search for next variable to replace.
                nPosition = aWorkText.indexOf( "$(", nPosition );
                // Have we found any variable like "$(...)"?
                if ( nPosition != -1 )
                {
                    // Yes; Get length of found variable. If no ")" was found - nLength must set to 0!
                    nLength = 0;
                    sal_Int32 nEndPosition = aWorkText.indexOf( ')', nPosition );
                    if ( nEndPosition != -1 )
                        nLength = nEndPosition - nPosition + 1;
                }
            }
        }

        nPosition = aWorkText.indexOf( "$(" );
        if ( nPosition == -1 )
        {
            bSubstitutionCompleted = true;
            break; // All variables are substituted
        }
        else
        {
            // Check for recursion
            const sal_uInt32 nCount = aEndlessRecursiveDetector.size();
            for ( sal_uInt32 i=0; i < nCount; i++ )
            {
                if ( aEndlessRecursiveDetector[i] == aWorkText )
                {
                    if ( bVarNotSubstituted )
                        break; // Not all variables could be substituted!
                    else
                    {
                        nDepth = nMaxRecursiveDepth;
                        break; // Recursion detected!
                    }
                }
            }

            aEndlessRecursiveDetector.push_back( aWorkText );

            // Initialize values for next
            sal_Int32 nEndPosition = aWorkText.indexOf( ')', nPosition );
            if ( nEndPosition != -1 )
                nLength = nEndPosition - nPosition + 1;
            bVarNotSubstituted = false;
            ++nDepth;
        }
    }

    // Fill return value with result
    if ( bSubstitutionCompleted )
    {
        // Substitution successful!
        aResult = aWorkText;
    }
    else
    {
        // Substitution not successful!
        if ( nDepth == nMaxRecursiveDepth )
        {
            // recursion depth reached!
            if ( bSubstRequired )
            {
                throw NoSuchElementException( "Endless recursion detected. Cannot substitute variables!", static_cast<cppu::OWeakObject *>(this) );
            }
            aResult = rText;
        }
        else
        {
            // variable in text but unknown!
            if ( bSubstRequired )
            {
                throw NoSuchElementException( "Unknown variable found!", static_cast<cppu::OWeakObject *>(this) );
            }
            aResult = aWorkText;
        }
    }

    return aResult;
}

OUString SubstitutePathVariables::impl_reSubstituteVariables( const OUString& rURL )
{
    OUString aURL;

    INetURLObject aUrl( rURL );
    if ( !aUrl.HasError() )
        aURL = aUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    else
    {
        // Convert a system path to a UCB compliant URL before resubstitution
        OUString aTemp;
        if ( osl::FileBase::getFileURLFromSystemPath( rURL, aTemp ) == osl::FileBase::E_None )
        {
            aURL = INetURLObject( aTemp ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
            if( aURL.isEmpty() )
                return rURL;
        }
        else
        {
            // rURL is not a valid URL nor a osl system path. Give up and return error!
            return rURL;
        }
    }

    // Get transient predefined path variable $(work) value before starting resubstitution
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();

    for (;;)
    {
        bool bVariableFound = false;

        for (auto const & i: m_aReSubstFixedVarOrder)
        {
            OUString aValue = m_aPreDefVars.m_FixedVar[i.eVariable];
            sal_Int32 nPos = aURL.indexOf( aValue );
            if ( nPos >= 0 )
            {
                bool bMatch = true;
                if ( !aFixedVarTable[i.eVariable].bAbsPath )
                {
                    // Special path variables as they can occur in the middle of a path. Only match if they
                    // describe a whole directory and not only a substring of a directory!
                    // (Ideally, all substitutions should stick to syntactical
                    // boundaries within the given URL, like not covering only
                    // part of a URL path segment; however, at least when saving
                    // an Impress document, one URL that is passed in is of the
                    // form <file:///.../share/palette%3Bfile:///.../user/
                    // config/standard.sob>, re-substituted to
                    // <$(inst)/share/palette%3B$(user)/config/standard.sob>.)
                    const sal_Unicode* pStr = aURL.getStr();

                    if ( nPos > 0 )
                        bMatch = ( aURL[ nPos-1 ] == '/' );

                    if ( bMatch )
                    {
                        if ( nPos + aValue.getLength() < aURL.getLength() )
                            bMatch = ( pStr[ nPos + aValue.getLength() ] == '/' );
                    }
                }

                if ( bMatch )
                {
                    aURL = aURL.replaceAt(
                        nPos, aValue.getLength(),
                        m_aPreDefVars.m_FixedVarNames[i.eVariable]);
                    bVariableFound = true; // Resubstitution not finished yet!
                    break;
                }
            }
        }

        if ( !bVariableFound )
        {
            return aURL;
        }
    }
}

// This method support both request schemes "$("<varname>")" or "<varname>".
OUString const & SubstitutePathVariables::impl_getSubstituteVariableValue( const OUString& rVariable )
{
    OUString aVariable;

    sal_Int32 nPos = rVariable.indexOf( "$(" );
    if ( nPos == -1 )
    {
        // Prepare variable name before hash map access
        aVariable = "$(" + rVariable + ")";
    }

    VarNameToIndexMap::const_iterator pNTOIIter = m_aPreDefVarMap.find( ( nPos == -1 ) ? aVariable : rVariable );

    // Fixed/Predefined variable
    if ( pNTOIIter == m_aPreDefVarMap.end() )
    {
        throw NoSuchElementException("Unknown variable!", static_cast<cppu::OWeakObject *>(this));
    }
    PreDefVariable nIndex = pNTOIIter->second;
    return m_aPreDefVars.m_FixedVar[static_cast<sal_Int32>(nIndex)];
}

void SubstitutePathVariables::SetPredefinedPathVariables()
{

    m_aPreDefVars.m_FixedVar[PREDEFVAR_BRANDBASEURL] = "$BRAND_BASE_DIR";
    rtl::Bootstrap::expandMacros(
        m_aPreDefVars.m_FixedVar[PREDEFVAR_BRANDBASEURL]);

    // Get inspath and userpath from bootstrap mechanism in every case as file URL
    ::utl::Bootstrap::PathStatus aState;
    OUString              sVal;

    aState = utl::Bootstrap::locateUserData( sVal );
    //There can be the valid case that there is no user installation.
    //TODO: Is that still the case? (With OOo 3.4, "unopkg sync" was run as part
    // of the setup. Then no user installation was required.)
    //Therefore we do not assert here.
    // It's not possible to detect when an empty value would actually be used.
    // (note: getenv is a hack to detect if we're running in a unit test)
    // Also, it's okay to have an empty user installation path in case of LOK
    if (aState == ::utl::Bootstrap::PATH_EXISTS || getenv("SRC_ROOT") ||
        (comphelper::LibreOfficeKit::isActive() && aState == ::utl::Bootstrap::PATH_VALID))
    {
        m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERPATH ] = sVal;
    }

    // Set $(inst), $(instpath), $(insturl)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_INSTPATH ] = m_aPreDefVars.m_FixedVar[PREDEFVAR_BRANDBASEURL];
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_INSTURL ]    = m_aPreDefVars.m_FixedVar[ PREDEFVAR_INSTPATH ];
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_INST ]       = m_aPreDefVars.m_FixedVar[ PREDEFVAR_INSTPATH ];
    // New variable of hierarchy service (#i32656#)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_BASEINSTURL ]= m_aPreDefVars.m_FixedVar[ PREDEFVAR_INSTPATH ];

    // Set $(user), $(userpath), $(userurl)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERURL ]    = m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERPATH ];
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_USER ]       = m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERPATH ];
    // New variable of hierarchy service (#i32656#)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERDATAURL ]= m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERPATH ];

    // Detect the program directory
    // Set $(prog), $(progpath), $(progurl)
    INetURLObject aProgObj(
        m_aPreDefVars.m_FixedVar[PREDEFVAR_BRANDBASEURL] );
    if ( !aProgObj.HasError() && aProgObj.insertName( LIBO_BIN_FOLDER ) )
    {
        m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROGPATH ] = aProgObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROGURL ]  = m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROGPATH ];
        m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROG ]     = m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROGPATH ];
    }

    // Set $(username)
    OUString aSystemUser;
    ::osl::Security aSecurity;
    aSecurity.getUserName( aSystemUser, false );
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_USERNAME ]   = aSystemUser;

    // Detect the language type of the current office
    m_aPreDefVars.m_eLanguageType = LANGUAGE_ENGLISH_US;
    OUString aLocaleStr( utl::ConfigManager::getUILocale() );
    m_aPreDefVars.m_eLanguageType = LanguageTag::convertToLanguageTypeWithFallback( aLocaleStr );
    // We used to have an else branch here with a SAL_WARN, but that
    // always fired in some unit tests when this code was built with
    // debug=t, so it seems fairly pointless, especially as
    // m_aPreDefVars.m_eLanguageType has been initialized to a
    // default value above anyway.

    // Set $(vlang)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_VLANG ] = aLocaleStr;

    // Set $(langid)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_LANGID ] = OUString::number( static_cast<sal_uInt16>(m_aPreDefVars.m_eLanguageType) );

    // Set the other pre defined path variables
    // Set $(work)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_HOME ] = GetHomeVariableValue();

    // Set $(workdirurl) this is the value of the path PATH_WORK which doesn't make sense
    // anymore because the path settings service has this value! It can deliver this value more
    // quickly than the substitution service!
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_WORKDIRURL ] = GetWorkPath();

    // Set $(path) variable
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_PATH ] = GetPathVariableValue();

    // Set $(temp)
    OUString aTmp;
    osl::FileBase::getTempDirURL( aTmp );
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_TEMP ] = aTmp;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_PathSubstitution_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SubstitutePathVariables(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
