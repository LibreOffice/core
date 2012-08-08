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

#include "services/substitutepathvars.hxx"
#include <threadhelp/resetableguard.hxx>
#include <helper/networkdomain.hxx>
#include "services.h"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <unotools/configitem.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/configmgr.hxx>

#include <unotools/bootstrap.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/socket.hxx>
#include <osl/process.h>
#include <i18npool/mslangid.hxx>
#include <tools/urlobj.hxx>
#include <tools/resmgr.hxx>
#include <tools/wldcrd.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <comphelper/configurationhelper.hxx>

#include <string.h>

//_________________________________________________________________________________________________________________
//      Defines
//_________________________________________________________________________________________________________________

#define STRPOS_NOTFOUND                  (sal_Int32)-1

#define ASCII_STR( val )                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( val ))

#define SEARCHPATH_DELIMITER             ';'

// Variable start/end characters
#define SIGN_STARTVARIABLE               ASCII_STR("$(")
#define SIGN_ENDVARIABLE                 ASCII_STR(")")

// Length of SUBSTITUTE_... to replace it with real values.
#define REPLACELENGTH_INST               7
#define REPLACELENGTH_PROG               7
#define REPLACELENGTH_USER               7
#define REPLACELENGTH_WORK               7
#define REPLACELENGTH_HOME               7
#define REPLACELENGTH_TEMP               7
#define REPLACELENGTH_PATH               7
#define REPLACELENGTH_INSTPATH          11
#define REPLACELENGTH_PROGPATH          11
#define REPLACELENGTH_USERPATH          11
#define REPLACELENGTH_INSTURL           10
#define REPLACELENGTH_PROGURL           10
#define REPLACELENGTH_USERURL           10
#define REPLACELENGTH_PATH               7
#define REPLACELENGTH_LANG               7
#define REPLACELENGTH_LANGID             9
#define REPLACELENGTH_VLANG              8
#define REPLACELENGTH_WORKDIRURL        13
// New variable of hierachy service (#i32656#)
#define REPLACELENGTH_BASEINSTURL       14
#define REPLACELENGTH_USERDATAURL       14

// Name of the pre defined path variables
#define VARIABLE_INST                                   "$(inst)"
#define VARIABLE_PROG                                   "$(prog)"
#define VARIABLE_USER                                   "$(user)"
#define VARIABLE_WORK                                   "$(work)"
#define VARIABLE_HOME                                   "$(home)"
#define VARIABLE_TEMP                                   "$(temp)"
#define VARIABLE_PATH                                   "$(path)"
#define VARIABLE_LANG                                   "$(lang)"
#define VARIABLE_LANGID                                 "$(langid)"
#define VARIABLE_VLANG                                  "$(vlang)"
#define VARIABLE_INSTPATH                               "$(instpath)"
#define VARIABLE_PROGPATH                               "$(progpath)"
#define VARIABLE_USERPATH                               "$(userpath)"
#define VARIABLE_INSTURL                                "$(insturl)"
#define VARIABLE_PROGURL                                "$(progurl)"
#define VARIABLE_USERURL                                "$(userurl)"
#define VARIABLE_WORKDIRURL                             "$(workdirurl)"
// New variable of hierachy service (#i32656#)
#define VARIABLE_BASEINSTURL                            "$(baseinsturl)"
#define VARIABLE_USERDATAURL                            "$(userdataurl)"
#define VARIABLE_BRANDBASEURL                           "$(brandbaseurl)"

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

//_________________________________________________________________________________________________________________
//      Namespace
//_________________________________________________________________________________________________________________

namespace framework
{

struct FixedVariable
{
    const char*     pVarName;
    PreDefVariable  nEnumValue;
    int             nStrLen;
    bool            bAbsPath;
};

struct TableEntry
{
    const char* pOSString;
    int         nStrLen;
};

// Table with valid operating system strings
// Name of the os as char* and the length
// of the string
static TableEntry aOSTable[OS_COUNT] =
{
    { "WINDOWS"     ,       7       },
    { "UNIX"        ,       4       },
    { "SOLARIS"     ,       7       },
    { "LINUX"       ,       5       },
    { ""            ,       0       }       // unknown
};

// Table with valid environment variables
// Name of the environment type as a char* and
// the length of the string.
static TableEntry aEnvTable[ET_COUNT] =
{
    { "HOST"        ,       4       },
    { "YPDOMAIN"    ,       8       },
    { "DNSDOMAIN"   ,       9       },
    { "NTDOMAIN"    ,       8       },
    { "OS"          ,       2       },
    { ""            ,       0       } // unknown
};

// Priority table for the environment types. Lower numbers define
// a higher priority. Equal numbers has the same priority that means
// that the first match wins!!
static sal_Int16 aEnvPrioTable[ET_COUNT] =
{
    1,      // ET_HOST
    2,      // ET_IPDOMAIN
    2,      // ET_DNSDOMAIN
    2,      // ET_NTDOMAIN
    3,      // ET_OS
    99,     // ET_UNKNOWN
};

// Table with all fixed/predefined variables supported.
static FixedVariable aFixedVarTable[] =
{
    { VARIABLE_INST,        PREDEFVAR_INST,         REPLACELENGTH_INST,     true                       },
    { VARIABLE_PROG,        PREDEFVAR_PROG,         REPLACELENGTH_PROG,     true                       },
    { VARIABLE_USER,        PREDEFVAR_USER,         REPLACELENGTH_USER,     true                       },
    { VARIABLE_WORK,        PREDEFVAR_WORK,         REPLACELENGTH_WORK,     true                       },      // Special variable (transient)!
    { VARIABLE_HOME,        PREDEFVAR_HOME,         REPLACELENGTH_HOME,     true                       },
    { VARIABLE_TEMP,        PREDEFVAR_TEMP,         REPLACELENGTH_TEMP,     true                       },
    { VARIABLE_PATH,        PREDEFVAR_PATH,         REPLACELENGTH_PATH,     true                       },
    { VARIABLE_LANG,        PREDEFVAR_LANG,         REPLACELENGTH_LANG,     false                      },
    { VARIABLE_LANGID,      PREDEFVAR_LANGID,       REPLACELENGTH_LANGID,   false                      },
    { VARIABLE_VLANG,       PREDEFVAR_VLANG,        REPLACELENGTH_VLANG,    false                      },
    { VARIABLE_INSTPATH,    PREDEFVAR_INSTPATH,     REPLACELENGTH_INSTPATH, true                       },
    { VARIABLE_PROGPATH,    PREDEFVAR_PROGPATH,     REPLACELENGTH_PROGPATH, true                       },
    { VARIABLE_USERPATH,    PREDEFVAR_USERPATH,     REPLACELENGTH_USERPATH, true                       },
    { VARIABLE_INSTURL,     PREDEFVAR_INSTURL,      REPLACELENGTH_INSTURL,  true                       },
    { VARIABLE_PROGURL,     PREDEFVAR_PROGURL,      REPLACELENGTH_PROGURL,  true                       },
    { VARIABLE_USERURL,     PREDEFVAR_USERURL,      REPLACELENGTH_USERURL,  true                       },
    { VARIABLE_WORKDIRURL,  PREDEFVAR_WORKDIRURL,   REPLACELENGTH_WORKDIRURL,true                      },  // Special variable (transient) and don't use for resubstitution!
    // New variable of hierachy service (#i32656#)
    { VARIABLE_BASEINSTURL, PREDEFVAR_BASEINSTURL,  REPLACELENGTH_BASEINSTURL,true                     },
    { VARIABLE_USERDATAURL, PREDEFVAR_USERDATAURL,  REPLACELENGTH_USERDATAURL,true                     },
    { VARIABLE_BRANDBASEURL,PREDEFVAR_BRANDBASEURL, RTL_CONSTASCII_LENGTH(VARIABLE_BRANDBASEURL), true }
};

//_________________________________________________________________________________________________________________
//      Implementation helper classes
//_________________________________________________________________________________________________________________

OperatingSystem SubstitutePathVariables_Impl::GetOperatingSystemFromString( const rtl::OUString& aOSString )
{
    for ( int i = 0; i < OS_COUNT; i++ )
    {
        if ( aOSString.equalsIgnoreAsciiCaseAsciiL( aOSTable[i].pOSString, aOSTable[i].nStrLen ))
            return (OperatingSystem)i;
    }

    return OS_UNKNOWN;
}

EnvironmentType SubstitutePathVariables_Impl::GetEnvTypeFromString( const rtl::OUString& aEnvTypeString )
{
    for ( int i = 0; i < ET_COUNT; i++ )
    {
        if ( aEnvTypeString.equalsIgnoreAsciiCaseAsciiL( aEnvTable[i].pOSString, aEnvTable[i].nStrLen ))
            return (EnvironmentType)i;
    }

    return ET_UNKNOWN;
}

SubstitutePathVariables_Impl::SubstitutePathVariables_Impl( const Link& aNotifyLink ) :
    utl::ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Substitution" ))),
    m_bYPDomainRetrieved( false ),
    m_bDNSDomainRetrieved( false ),
    m_bNTDomainRetrieved( false ),
    m_bHostRetrieved( false ),
    m_bOSRetrieved( false ),
    m_aListenerNotify( aNotifyLink ),
    m_aSharePointsNodeName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SharePoints" ))),
    m_aDirPropertyName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/Directory" ))),
    m_aEnvPropertyName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/Environment" ))),
    m_aLevelSep( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" )))
{
    // Enable notification mechanism
    // We need it to get information about changes outside these class on our configuration branch
    Sequence< rtl::OUString > aNotifySeq( 1 );
    aNotifySeq[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SharePoints" ));
    EnableNotification( aNotifySeq, sal_True );
}

SubstitutePathVariables_Impl::~SubstitutePathVariables_Impl()
{
}

void SubstitutePathVariables_Impl::GetSharePointsRules( SubstituteVariables& aSubstVarMap )
{
    Sequence< rtl::OUString > aSharePointNames;
    ReadSharePointsFromConfiguration( aSharePointNames );

    if ( aSharePointNames.getLength() > 0 )
    {
        sal_Int32 nSharePoints = 0;

        // Read SharePoints container from configuration
        while ( nSharePoints < aSharePointNames.getLength() )
        {
            rtl::OUString aSharePointNodeName( m_aSharePointsNodeName );
            aSharePointNodeName += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
            aSharePointNodeName += aSharePointNames[ nSharePoints ];

            SubstituteRuleVector aRuleSet;
            ReadSharePointRuleSetFromConfiguration( aSharePointNames[ nSharePoints ], aSharePointNodeName, aRuleSet );
            if ( !aRuleSet.empty() )
            {
                // We have at minimum one rule. Filter the correct rule out of the rule set
                // and put into our SubstituteVariable map
                SubstituteRule aActiveRule;
                if ( FilterRuleSet( aRuleSet, aActiveRule ))
                {
                    // We have found an active rule
                    aActiveRule.aSubstVariable = aSharePointNames[ nSharePoints ];
                    aSubstVarMap.insert( SubstituteVariables::value_type(
                    aActiveRule.aSubstVariable, aActiveRule ));
                }
            }
            ++nSharePoints;
        }
    }
}

void SubstitutePathVariables_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& /*aPropertyNames*/ )
{
    // NOT implemented yet!
}

void SubstitutePathVariables_Impl::Commit()
{
}


//_________________________________________________________________________________________________________________
//      private methods
//_________________________________________________________________________________________________________________

OperatingSystem SubstitutePathVariables_Impl::GetOperatingSystem()
{
    if ( !m_bOSRetrieved )
    {
#ifdef SOLARIS
        m_eOSType = OS_SOLARIS;
#elif defined LINUX
        m_eOSType = OS_LINUX;
#elif defined WIN32
        m_eOSType = OS_WINDOWS;
#elif defined UNIX
        m_eOSType = OS_UNIX;
#else
        m_eOSType = OS_UNKNOWN;
#endif
        m_bOSRetrieved = sal_True;
    }

    return m_eOSType;
}

const rtl::OUString& SubstitutePathVariables_Impl::GetYPDomainName()
{
    if ( !m_bYPDomainRetrieved )
    {
        m_aYPDomain = NetworkDomain::GetYPDomainName().toAsciiLowerCase();
        m_bYPDomainRetrieved = sal_True;
    }

    return m_aYPDomain;
}

const rtl::OUString& SubstitutePathVariables_Impl::GetDNSDomainName()
{
    if ( !m_bDNSDomainRetrieved )
    {
        rtl::OUString   aTemp;
        osl::SocketAddr aSockAddr;
        oslSocketResult aResult;

        rtl::OUString aHostName = GetHostName();
        osl::SocketAddr::resolveHostname( aHostName, aSockAddr );
        aTemp = aSockAddr.getHostname( &aResult );

        // DNS domain name begins after the first "."
        sal_Int32 nIndex = aTemp.indexOf( '.' );
        if ( nIndex >= 0 && aTemp.getLength() > nIndex+1 )
            m_aDNSDomain = aTemp.copy( nIndex+1 ).toAsciiLowerCase();
        else
            m_aDNSDomain = rtl::OUString();

        m_bDNSDomainRetrieved = sal_True;
    }

    return m_aDNSDomain;
}

const rtl::OUString& SubstitutePathVariables_Impl::GetNTDomainName()
{
    if ( !m_bNTDomainRetrieved )
    {
        m_aNTDomain = NetworkDomain::GetNTDomainName().toAsciiLowerCase();
        m_bNTDomainRetrieved = sal_True;
    }

    return m_aNTDomain;
}

const rtl::OUString& SubstitutePathVariables_Impl::GetHostName()
{
    if ( !m_bHostRetrieved )
    {
        rtl::OUString   aHostName;
        oslSocketResult aSocketResult;

        m_aHost = osl::SocketAddr::getLocalHostname( &aSocketResult ).toAsciiLowerCase();
    }

    return m_aHost;
}

bool SubstitutePathVariables_Impl::FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule )
{
    bool bResult = sal_False;

    if ( !aRuleSet.empty() )
    {
        const sal_uInt32 nCount = aRuleSet.size();

        sal_Int16 nPrioCurrentRule = aEnvPrioTable[ ET_UNKNOWN ];
        for ( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const SubstituteRule& aRule = aRuleSet[nIndex];
            EnvironmentType eEnvType        = aRule.aEnvType;

            // Check if environment type has a higher priority than current one!
            if ( nPrioCurrentRule > aEnvPrioTable[eEnvType] )
            {
                switch ( eEnvType )
                {
                    case ET_HOST:
                    {
                        rtl::OUString aHost = GetHostName();
                        rtl::OUString aHostStr;
                        aRule.aEnvValue >>= aHostStr;
                        aHostStr = aHostStr.toAsciiLowerCase();

                        // Pattern match if domain environment match
                        WildCard aPattern(aHostStr);
                        bool bMatch = aPattern.Matches(aHost);
                        if ( bMatch )
                        {
                            aActiveRule      = aRule;
                            bResult          = true;
                            nPrioCurrentRule = aEnvPrioTable[eEnvType];
                        }
                    }
                    break;

                    case ET_YPDOMAIN:
                    case ET_DNSDOMAIN:
                    case ET_NTDOMAIN:
                    {
                        rtl::OUString   aDomain;
                        rtl::OUString   aDomainStr;
                        aRule.aEnvValue >>= aDomainStr;
                        aDomainStr = aDomainStr.toAsciiLowerCase();

                        // Retrieve the correct domain value
                        if ( eEnvType == ET_YPDOMAIN )
                            aDomain = GetYPDomainName();
                        else if ( eEnvType == ET_DNSDOMAIN )
                            aDomain = GetDNSDomainName();
                        else
                            aDomain = GetNTDomainName();

                        // Pattern match if domain environment match
                        WildCard aPattern(aDomainStr);
                        bool bMatch = aPattern.Matches(aDomain);
                        if ( bMatch )
                        {
                            aActiveRule      = aRule;
                            bResult          = true;
                            nPrioCurrentRule = aEnvPrioTable[eEnvType];
                        }
                    }
                    break;

                    case ET_OS:
                    {
                        // No pattern matching for OS type
                        OperatingSystem eOSType = GetOperatingSystem();

                        sal_Int16 nValue = 0;
                        aRule.aEnvValue >>= nValue;

                        bool            bUnix = ( eOSType == OS_LINUX ) || ( eOSType == OS_SOLARIS );
                        OperatingSystem eRuleOSType = (OperatingSystem)nValue;

                        // Match if OS identical or rule is set to UNIX and OS is LINUX/SOLARIS!
                        if (( eRuleOSType == eOSType ) || ( eRuleOSType == OS_UNIX && bUnix ))
                        {
                            aActiveRule      = aRule;
                            bResult          = true;
                            nPrioCurrentRule = aEnvPrioTable[eEnvType];
                        }
                    }
                    break;

                    case ET_UNKNOWN: // nothing to do
                        break;

                    default:
                        break;
                }
            }
        }
    }

    return bResult;
}

void SubstitutePathVariables_Impl::ReadSharePointsFromConfiguration( Sequence< rtl::OUString >& aSharePointsSeq )
{
    //returns all the names of all share point nodes
    aSharePointsSeq = GetNodeNames( m_aSharePointsNodeName );
}

void SubstitutePathVariables_Impl::ReadSharePointRuleSetFromConfiguration(
        const rtl::OUString& aSharePointName,
        const rtl::OUString& aSharePointNodeName,
        SubstituteRuleVector& rRuleSet )
{
    Sequence< rtl::OUString > aSharePointMappingsNodeNames = GetNodeNames( aSharePointNodeName, utl::CONFIG_NAME_LOCAL_PATH );

    sal_Int32 nSharePointMapping = 0;
    while ( nSharePointMapping < aSharePointMappingsNodeNames.getLength() )
    {
        rtl::OUString aSharePointMapping( aSharePointNodeName );
        aSharePointMapping += m_aLevelSep;
        aSharePointMapping += aSharePointMappingsNodeNames[ nSharePointMapping ];

        // Read SharePointMapping
        rtl::OUString aDirValue;
        rtl::OUString aDirProperty( aSharePointMapping );
        aDirProperty += m_aDirPropertyName;

        // Read only the directory property
        Sequence< rtl::OUString > aDirPropertySeq( 1 );
        aDirPropertySeq[0] = aDirProperty;

        Sequence< Any > aValueSeq = GetProperties( aDirPropertySeq );
        if ( aValueSeq.getLength() == 1 )
            aValueSeq[0] >>= aDirValue;

        // Read the environment setting
        rtl::OUString aEnvUsed;
        rtl::OUString aEnvProperty( aSharePointMapping );
        aEnvProperty += m_aEnvPropertyName;
        Sequence< rtl::OUString > aEnvironmentVariable = GetNodeNames( aEnvProperty );

        // Filter the property which has a value set
        Sequence< rtl::OUString > aEnvUsedPropertySeq( aEnvironmentVariable.getLength() );

        rtl::OUString aEnvUsePropNameTemplate( aEnvProperty );
        aEnvUsePropNameTemplate += m_aLevelSep;

        for ( sal_Int32 nProperty = 0; nProperty < aEnvironmentVariable.getLength(); nProperty++ )
            aEnvUsedPropertySeq[nProperty] = rtl::OUString( aEnvUsePropNameTemplate + aEnvironmentVariable[nProperty] );

        Sequence< Any > aEnvUsedValueSeq;
        aEnvUsedValueSeq = GetProperties( aEnvUsedPropertySeq );

        rtl::OUString aEnvUsedValue;
        for ( sal_Int32 nIndex = 0; nIndex < aEnvironmentVariable.getLength(); nIndex++ )
        {
            if ( aEnvUsedValueSeq[nIndex] >>= aEnvUsedValue )
            {
                aEnvUsed = aEnvironmentVariable[nIndex];
                break;
            }
        }

        // Decode the environment and optional the operatng system settings
        Any                             aEnvValue;
        EnvironmentType eEnvType = GetEnvTypeFromString( aEnvUsed );
        if ( eEnvType == ET_OS )
        {
            OperatingSystem eOSType = GetOperatingSystemFromString( aEnvUsedValue );
            aEnvValue <<= (sal_Int16)eOSType;
        }
        else
            aEnvValue <<= aEnvUsedValue;

        // Create rule struct and push it into the rule set
        SubstituteRule aRule( aSharePointName, aDirValue, aEnvValue, eEnvType );
        rRuleSet.push_back( aRule );

        ++nSharePointMapping;
    }
}

//*****************************************************************************************************************
//      XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  ( SubstitutePathVariables                     ,
                                          ::cppu::OWeakObject                         ,
                                          SERVICENAME_SUBSTITUTEPATHVARIABLES         ,
                                          IMPLEMENTATIONNAME_SUBSTITUTEPATHVARIABLES    )

DEFINE_INIT_SERVICE                     (   SubstitutePathVariables, {} )


SubstitutePathVariables::SubstitutePathVariables( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_aVarStart( SIGN_STARTVARIABLE ),
    m_aVarEnd( SIGN_ENDVARIABLE ),
    m_aImpl( LINK( this, SubstitutePathVariables, implts_ConfigurationNotify )),
    m_xServiceManager( xServiceManager )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::SubstitutePathVariables" );
    int i;

    SetPredefinedPathVariables( m_aPreDefVars );
    m_aImpl.GetSharePointsRules( m_aSubstVarMap );

    // Init the predefined/fixed variable to index hash map
    for ( i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        // Store variable name into struct of predefined/fixed variables
        m_aPreDefVars.m_FixedVarNames[i] = rtl::OUString::createFromAscii( aFixedVarTable[i].pVarName );

        // Create hash map entry
        m_aPreDefVarMap.insert( VarNameToIndexMap::value_type(
            m_aPreDefVars.m_FixedVarNames[i], aFixedVarTable[i].nEnumValue ) );
    }

    // Sort predefined/fixed variable to path length
    for ( i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        if (( i != PREDEFVAR_WORKDIRURL ) && ( i != PREDEFVAR_PATH ))
        {
            // Special path variables, don't include into automatic resubstituion search!
            // $(workdirurl) is not allowed to resubstitute! This variable is the value of path settings entry
            // and it could be possible that it will be resubstituted by itself!!
            // Example: WORK_PATH=c:\test, $(workdirurl)=WORK_PATH => WORK_PATH=$(workdirurl) and this cannot be substituted!
            ReSubstFixedVarOrder aFixedVar;
            aFixedVar.eVariable       = aFixedVarTable[i].nEnumValue;
            aFixedVar.nVarValueLength = m_aPreDefVars.m_FixedVar[(sal_Int32)aFixedVar.eVariable].getLength();
            m_aReSubstFixedVarOrder.push_back( aFixedVar );
        }
    }
    m_aReSubstFixedVarOrder.sort();

    // Sort user variables to path length
    SubstituteVariables::const_iterator pIter;
    for ( pIter = m_aSubstVarMap.begin(); pIter != m_aSubstVarMap.end(); ++pIter )
    {
        ReSubstUserVarOrder aUserOrderVar;
        rtl::OUStringBuffer aStrBuffer( pIter->second.aSubstVariable.getLength() );
        aStrBuffer.append( m_aVarStart );
        aStrBuffer.append( pIter->second.aSubstVariable );
        aStrBuffer.append( m_aVarEnd );
        aUserOrderVar.aVarName        = aStrBuffer.makeStringAndClear();
        aUserOrderVar.nVarValueLength = pIter->second.aSubstVariable.getLength();
        m_aReSubstUserVarOrder.push_back( aUserOrderVar );
    }
    m_aReSubstUserVarOrder.sort();
}

SubstitutePathVariables::~SubstitutePathVariables()
{
}

// XStringSubstitution
rtl::OUString SAL_CALL SubstitutePathVariables::substituteVariables( const ::rtl::OUString& aText, sal_Bool bSubstRequired )
throw ( NoSuchElementException, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::substituteVariables" );
    ResetableGuard aLock( m_aLock );
    return impl_substituteVariable( aText, bSubstRequired );
}

rtl::OUString SAL_CALL SubstitutePathVariables::reSubstituteVariables( const ::rtl::OUString& aText )
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::reSubstituteVariables" );
    ResetableGuard aLock( m_aLock );
    return impl_reSubstituteVariables( aText );
}

rtl::OUString SAL_CALL SubstitutePathVariables::getSubstituteVariableValue( const ::rtl::OUString& aVariable )
throw ( NoSuchElementException, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::getSubstituteVariableValue" );
    ResetableGuard aLock( m_aLock );
    return impl_getSubstituteVariableValue( aVariable );
}

//_________________________________________________________________________________________________________________
//      protected methods
//_________________________________________________________________________________________________________________

IMPL_LINK_NOARG(SubstitutePathVariables, implts_ConfigurationNotify)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    return 0;
}

rtl::OUString SubstitutePathVariables::ConvertOSLtoUCBURL( const rtl::OUString& aOSLCompliantURL ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::ConvertOSLtoUCBURL" );
    rtl::OUString aResult;
    rtl::OUString   aTemp;

    osl::FileBase::getSystemPathFromFileURL( aOSLCompliantURL, aTemp );
    utl::LocalFileHelper::ConvertPhysicalNameToURL( aTemp, aResult );

    // Not all OSL URL's can be mapped to UCB URL's!
    if ( aResult.isEmpty() )
        return aOSLCompliantURL;
    else
        return aResult;
}

rtl::OUString SubstitutePathVariables::GetWorkPath() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::GetWorkPath" );
        rtl::OUString aWorkPath;

    try
    {
        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xServiceManager,
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Paths")),
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Paths/Work")),
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WritePath")),
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= aWorkPath;
    }
    catch(const RuntimeException &)
    {
    }

    // fallback in case config layer does not return an useable work dir value.
    if (aWorkPath.isEmpty())
        aWorkPath = GetWorkVariableValue();

    return aWorkPath;
}

rtl::OUString SubstitutePathVariables::GetWorkVariableValue() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::GetWorkVariableValue" );
    ::rtl::OUString aWorkPath;

    try
    {
        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xServiceManager,
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Paths")),
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Variables")),
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Work")),
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= aWorkPath;
    }
    catch(const RuntimeException &)
    {
    }

    // fallback to $HOME in case platform dependend config layer does not return
    // an usuable work dir value.
    if (aWorkPath.isEmpty())
    {
        osl::Security aSecurity;
        aSecurity.getHomeDir( aWorkPath );
    }
    return ConvertOSLtoUCBURL( aWorkPath );
}

rtl::OUString SubstitutePathVariables::GetHomeVariableValue() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::GetHomeVariableValue" );
    osl::Security   aSecurity;
    rtl::OUString   aHomePath;

    aSecurity.getHomeDir( aHomePath );
    return ConvertOSLtoUCBURL( aHomePath );
}

rtl::OUString SubstitutePathVariables::GetPathVariableValue() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::GetPathVariableValue" );
    const int PATH_EXTEND_FACTOR = 120;

    rtl::OUString aRetStr;
    const char*   pEnv = getenv( "PATH" );

    if ( pEnv )
    {
        rtl::OUString       aTmp;
        rtl::OUString       aPathList( pEnv, strlen( pEnv ), osl_getThreadTextEncoding() );
        rtl::OUStringBuffer aPathStrBuffer( aPathList.getLength() * PATH_EXTEND_FACTOR / 100 );

        bool      bAppendSep = false;
        sal_Int32 nToken = 0;
        do
        {
            ::rtl::OUString sToken = aPathList.getToken(0, SAL_PATHSEPARATOR, nToken);
            if (!sToken.isEmpty())
            {
                osl::FileBase::getFileURLFromSystemPath( sToken, aTmp );
                if ( bAppendSep )
                    aPathStrBuffer.appendAscii( ";" ); // Office uses ';' as path separator
                aPathStrBuffer.append( aTmp );
                bAppendSep = true;
            }
        }
        while(nToken>=0);

        aRetStr = aPathStrBuffer.makeStringAndClear();
    }

    return aRetStr;
}

rtl::OUString SubstitutePathVariables::impl_substituteVariable( const ::rtl::OUString& rText, bool bSubstRequired )
throw ( NoSuchElementException, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::impl_substituteVariable" );
    // This is maximal recursive depth supported!
    const sal_Int32 nMaxRecursiveDepth = 8;

    rtl::OUString   aWorkText = rText;
    rtl::OUString   aResult;

    // Use vector with strings to detect endless recursions!
    std::vector< rtl::OUString > aEndlessRecursiveDetector;

    // Search for first occure of "$(...".
    sal_Int32   nDepth = 0;
    sal_Int32   bSubstitutionCompleted = sal_False;
    sal_Int32   nPosition       = aWorkText.indexOf( m_aVarStart );     // = first position of "$(" in string
    sal_Int32   nLength = 0; // = count of letters from "$(" to ")" in string
    bool        bVarNotSubstituted = false;

    // Have we found any variable like "$(...)"?
    if ( nPosition != STRPOS_NOTFOUND )
    {
        // Yes; Get length of found variable.
        // If no ")" was found - nLength is set to 0 by default! see before.
        sal_Int32 nEndPosition = aWorkText.indexOf( m_aVarEnd, nPosition );
        if ( nEndPosition != STRPOS_NOTFOUND )
            nLength = nEndPosition - nPosition + 1;
    }

    // Is there something to replace ?
    bool bWorkRetrieved       = false;
    bool bWorkDirURLRetrieved = false;
    while ( !bSubstitutionCompleted && nDepth < nMaxRecursiveDepth )
    {
        while ( ( nPosition != STRPOS_NOTFOUND ) && ( nLength > 3 ) ) // "$(" ")"
        {
            // YES; Get the next variable for replace.
            sal_Int32     nReplaceLength  = 0;
            rtl::OUString aReplacement;
            rtl::OUString aSubString      = aWorkText.copy( nPosition, nLength );
            rtl::OUString aSubVarString;

            // Path variables are not case sensitive!
            aSubVarString = aSubString.toAsciiLowerCase();
            VarNameToIndexMap::const_iterator pNTOIIter = m_aPreDefVarMap.find( aSubVarString );
            if ( pNTOIIter != m_aPreDefVarMap.end() )
            {
                // Fixed/Predefined variable found
                PreDefVariable nIndex = (PreDefVariable)pNTOIIter->second;

                // Determine variable value and length from array/table
                if ( nIndex == PREDEFVAR_WORK && !bWorkRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ] = GetWorkVariableValue();
                    bWorkRetrieved = true;
                }
                else if ( nIndex == PREDEFVAR_WORKDIRURL && !bWorkDirURLRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ] = GetWorkPath();
                    bWorkDirURLRetrieved = true;
                }

                // Check preconditions to substitue path variables.
                // 1. A path variable can only be substituted if it follows a SEARCHPATH_DELIMITER ';'!
                // 2. It's located exactly at the start of the string being substituted!
                if (( aFixedVarTable[ int( nIndex ) ].bAbsPath && (( nPosition == 0 ) || (( nPosition > 0 ) && ( aWorkText[nPosition-1] == ';')))) ||
            ( !aFixedVarTable[ int( nIndex ) ].bAbsPath ))
        {
                    aReplacement = m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ];
                    nReplaceLength = nLength;
                }
            }
            else
            {
                // Extract the variable name and try to find in the user defined variable set
                rtl::OUString aVarName = aSubString.copy( 2, nLength-3 );
                SubstituteVariables::const_iterator pIter = m_aSubstVarMap.find( aVarName );
                if ( pIter != m_aSubstVarMap.end() )
                {
                    // Found.
                    aReplacement = pIter->second.aSubstValue;
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
                bVarNotSubstituted = false;
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
                nPosition = STRPOS_NOTFOUND;
                nLength = 0;
            }
            else
            {
                // Else; Position is valid. Search for next variable to replace.
                nPosition = aWorkText.indexOf( m_aVarStart, nPosition );
                // Have we found any variable like "$(...)"?
                if ( nPosition != STRPOS_NOTFOUND )
                {
                    // Yes; Get length of found variable. If no ")" was found - nLength must set to 0!
                    nLength = 0;
                    sal_Int32 nEndPosition = aWorkText.indexOf( m_aVarEnd, nPosition );
                    if ( nEndPosition != STRPOS_NOTFOUND )
                        nLength = nEndPosition - nPosition + 1;
                }
            }
        }

        nPosition = aWorkText.indexOf( m_aVarStart );
        if ( nPosition == -1 )
        {
            bSubstitutionCompleted = sal_True;
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
            sal_Int32 nEndPosition = aWorkText.indexOf( m_aVarEnd, nPosition );
            if ( nEndPosition != STRPOS_NOTFOUND )
                nLength = nEndPosition - nPosition + 1;
            bVarNotSubstituted = sal_False;
            ++nDepth;
        }
    }

    // Fill return value with result
    if ( bSubstitutionCompleted )
    {
        // Substitution successfull!
        aResult = aWorkText;
    }
    else
    {
        // Substitution not successfull!
        if ( nDepth == nMaxRecursiveDepth )
        {
            // recursion depth reached!
            if ( bSubstRequired )
            {
                rtl::OUString aMsg( RTL_CONSTASCII_USTRINGPARAM( "Endless recursion detected. Cannot substitute variables!" ));
                throw NoSuchElementException( aMsg, (cppu::OWeakObject *)this );
            }
            else
                aResult = rText;
        }
        else
        {
            // variable in text but unknown!
            if ( bSubstRequired )
            {
                rtl::OUString aMsg( RTL_CONSTASCII_USTRINGPARAM( "Unknown variable found!" ));
                throw NoSuchElementException( aMsg, (cppu::OWeakObject *)this );
            }
            else
                aResult = aWorkText;
        }
    }

    return aResult;
}

rtl::OUString SubstitutePathVariables::impl_reSubstituteVariables( const ::rtl::OUString& rURL )
throw ( RuntimeException )
{
    rtl::OUString aURL;

    INetURLObject aUrl( rURL );
    if ( !aUrl.HasError() )
        aURL = aUrl.GetMainURL( INetURLObject::NO_DECODE );
    else
    {
        // Convert a system path to a UCB compliant URL before resubstitution
        rtl::OUString aTemp;
        if ( osl::FileBase::getFileURLFromSystemPath( rURL, aTemp ) == osl::FileBase::E_None )
        {
            aTemp = ConvertOSLtoUCBURL( aTemp );
            if ( !aTemp.isEmpty() )
            {
                aURL = INetURLObject( aTemp ).GetMainURL( INetURLObject::NO_DECODE );
                if( aURL.isEmpty() )
                    return rURL;
            }
            else
                return rURL;
        }
        else
        {
            // rURL is not a valid URL nor a osl system path. Give up and return error!
            return rURL;
        }
    }

    // Due to a recursive definition this code must exchange variables with variables!
    bool bResubstitutionCompleted = false;
    bool bVariableFound           = false;

    // Get transient predefined path variable $(work) value before starting resubstitution
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();

    while ( !bResubstitutionCompleted )
    {
        ReSubstFixedVarOrderVector::const_iterator pIterFixed;
        for ( pIterFixed = m_aReSubstFixedVarOrder.begin(); pIterFixed != m_aReSubstFixedVarOrder.end(); ++pIterFixed )
        {
            rtl::OUString aValue = m_aPreDefVars.m_FixedVar[ (sal_Int32)pIterFixed->eVariable ];
            sal_Int32 nPos = aURL.indexOf( aValue );
            if ( nPos >= 0 )
            {
                bool bMatch = true;
                if ( pIterFixed->eVariable == PREDEFVAR_LANG ||
                     pIterFixed->eVariable == PREDEFVAR_LANGID ||
                     pIterFixed->eVariable == PREDEFVAR_VLANG )
                {
                    // Special path variables as they can occur in the middle of a path. Only match if they
                    // describe a whole directory and not only a substring of a directory!
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
                    rtl::OUStringBuffer aStrBuffer( aURL.getLength() );
                    aStrBuffer.append( aURL.copy( 0, nPos ) );
                    aStrBuffer.append( m_aPreDefVars.m_FixedVarNames[ (sal_Int32)pIterFixed->eVariable ] ); // Get the variable name for struct var name array!
                    aStrBuffer.append( aURL.copy( nPos + aValue.getLength(), ( aURL.getLength() - ( nPos + aValue.getLength() )) ));
                    aURL = aStrBuffer.makeStringAndClear();
                    bVariableFound = true; // Resubstitution not finished yet!
                    break;
                }
            }
        }

        // This part can be iteratered more than one time as variables can contain variables again!
        ReSubstUserVarOrderVector::const_iterator pIterUser;
        for ( pIterUser = m_aReSubstUserVarOrder.begin(); pIterUser != m_aReSubstUserVarOrder.end(); ++pIterUser )
        {
            rtl::OUString aVarValue = pIterUser->aVarName;
            sal_Int32 nPos = aURL.indexOf( aVarValue );
            if ( nPos >= 0 )
            {
                rtl::OUStringBuffer aStrBuffer( aURL.getLength() );
                aStrBuffer.append( aURL.copy( 0, nPos ) );
                aStrBuffer.append( m_aVarStart );
                aStrBuffer.append( aVarValue );
                aStrBuffer.append( m_aVarEnd );
                aStrBuffer.append( aURL.copy( nPos +  aVarValue.getLength(), ( aURL.getLength() - ( nPos + aVarValue.getLength() )) ));
                aURL = aStrBuffer.makeStringAndClear();
                bVariableFound = true;  // Resubstitution not finished yet!
            }
        }

        if ( !bVariableFound )
            bResubstitutionCompleted = true;
        else
            bVariableFound = sal_False; // Next resubstitution
    }

    return aURL;
}

// This method support both request schemes "$("<varname>")" or "<varname>".
::rtl::OUString SubstitutePathVariables::impl_getSubstituteVariableValue( const ::rtl::OUString& rVariable )
throw ( NoSuchElementException, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::impl_getSubstituteVariableValue" );
    rtl::OUString aVariable;

    sal_Int32 nPos = rVariable.indexOf( m_aVarStart );
    if ( nPos == -1 )
    {
        // Prepare variable name before hash map access
        rtl::OUStringBuffer aStrBuffer( rVariable.getLength() + m_aVarStart.getLength() + m_aVarEnd.getLength() );
        aStrBuffer.append( m_aVarStart );
        aStrBuffer.append( rVariable );
        aStrBuffer.append( m_aVarEnd );
        aVariable = aStrBuffer.makeStringAndClear();
    }

    VarNameToIndexMap::const_iterator pNTOIIter = m_aPreDefVarMap.find( ( nPos == -1 ) ? aVariable : rVariable );

    // Fixed/Predefined variable
    if ( pNTOIIter != m_aPreDefVarMap.end() )
    {
        PreDefVariable nIndex = (PreDefVariable)pNTOIIter->second;
        return m_aPreDefVars.m_FixedVar[(sal_Int32)nIndex];
    }
    else
    {
        // Prepare variable name before hash map access
        if ( nPos >= 0 )
        {
            if ( rVariable.getLength() > 3 )
                aVariable = rVariable.copy( 2, rVariable.getLength() - 3 );
            else
            {
                rtl::OUString aExceptionText( RTL_CONSTASCII_USTRINGPARAM( "Unknown variable!" ));
                throw NoSuchElementException();
            }
        }
        else
            aVariable = rVariable;

        // User defined variable
        SubstituteVariables::const_iterator pIter = m_aSubstVarMap.find( aVariable );
        if ( pIter != m_aSubstVarMap.end() )
        {
            // found!
            return pIter->second.aSubstValue;
        }

        rtl::OUString aExceptionText( RTL_CONSTASCII_USTRINGPARAM( "Unknown variable!" ));
        throw NoSuchElementException( aExceptionText, (cppu::OWeakObject *)this );
    }
}

void SubstitutePathVariables::SetPredefinedPathVariables( PredefinedPathVariables& aPreDefPathVariables )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "SubstitutePathVariables::SetPredefinedPathVariables" );

    aPreDefPathVariables.m_FixedVar[PREDEFVAR_BRANDBASEURL] = rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR"));
    rtl::Bootstrap::expandMacros(
        aPreDefPathVariables.m_FixedVar[PREDEFVAR_BRANDBASEURL]);

    Any             aAny;
    ::rtl::OUString aTmp;
    String          aResult;

    // Get inspath and userpath from bootstrap mechanism in every case as file URL
    ::utl::Bootstrap::PathStatus aState;
    ::rtl::OUString              sVal  ;

    aState = utl::Bootstrap::locateUserData( sVal );
    //There can be the valid case that there is no user installation. For example, "unopkg sync"
    //is currently (OOo3.4) run as part of the setup. Then no user installation is required.
    //Therefore we do not assert here.
    if( aState == ::utl::Bootstrap::PATH_EXISTS ) {
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ] = ConvertOSLtoUCBURL( sVal );
    }

    // Set $(inst), $(instpath), $(insturl)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ] = aPreDefPathVariables.m_FixedVar[PREDEFVAR_BRANDBASEURL];
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTURL ]    = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ];
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INST ]       = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ];
    // New variable of hierachy service (#i32656#)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_BASEINSTURL ]= aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ];

    // Set $(user), $(userpath), $(userurl)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERURL ]    = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ];
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USER ]       = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ];
    // New variable of hierachy service (#i32656#)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERDATAURL ]= aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ];

    // Detect the program directory
    // Set $(prog), $(progpath), $(progurl)
    INetURLObject aProgObj(
        aPreDefPathVariables.m_FixedVar[PREDEFVAR_BRANDBASEURL] );
    if ( !aProgObj.HasError() && aProgObj.insertName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("program")) ) )
    {
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ] = aProgObj.GetMainURL(INetURLObject::NO_DECODE);
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGURL ]  = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ];
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROG ]     = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ];
    }

    // Detect the language type of the current office
    aPreDefPathVariables.m_eLanguageType = LANGUAGE_ENGLISH_US;
    rtl::OUString aLocaleStr( utl::ConfigManager::getLocale() );
    aPreDefPathVariables.m_eLanguageType = MsLangId::convertIsoStringToLanguage( aLocaleStr );
    // We used to have an else branch here with a LOG_ERROR, but that
    // always fired in some unit tests when this code was built with
    // debug=t, so it seems fairly pointless, especially as
    // aPreDefPathVariables.m_eLanguageType has been initialized to a
    // default value above anyway.

    // Set $(lang)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_LANG ] = ConvertOSLtoUCBURL(
    rtl::OUString::createFromAscii( ResMgr::GetLang( aPreDefPathVariables.m_eLanguageType, 0 ) ));

    // Set $(vlang)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_VLANG ] = aLocaleStr;

    // Set $(langid)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_LANGID ] = rtl::OUString::valueOf( (sal_Int32)aPreDefPathVariables.m_eLanguageType );

    // Set the other pre defined path variables
    // Set $(work)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_HOME ] = GetHomeVariableValue();

    // Set $(workdirurl) this is the value of the path PATH_WORK which doesn't make sense
    // anymore because the path settings service has this value! It can deliver this value more
    // quickly than the substitution service!
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_WORKDIRURL ] = GetWorkPath();

    // Set $(path) variable
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PATH ] = GetPathVariableValue();

    // Set $(temp)
    osl::FileBase::getTempDirURL( aTmp );
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_TEMP ] = ConvertOSLtoUCBURL( aTmp );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
