/*************************************************************************
 *
 *  $RCSfile: substitutepathvars.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:53 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_
#include "services/substitutepathvars.hxx"
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_NETWORKDOMAIN_HXX_
#include <helper/networkdomain.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_BOOTSTRAP_HXX_
#include <unotools/bootstrap.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif

#ifndef _OSL_SOCKET_HXX_
#include <osl/socket.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

#define STRPOS_NOTFOUND                     (sal_Int32)-1

#define ASCII_STR( val )                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( val ))

#define SEARCHPATH_DELIMITER                ';'

// Variable start/end characters
#define SIGN_STARTVARIABLE                  ASCII_STR("$(")
#define SIGN_ENDVARIABLE                    ASCII_STR(")")

// Strings to replace $(vlang)
#define REPLACEMENT_ARABIC                  ASCII_STR("arabic")
#define REPLACEMENT_CZECH                   ASCII_STR("czech")
#define REPLACEMENT_DANISH                  ASCII_STR("danish")
#define REPLACEMENT_DUTCH                   ASCII_STR("dutch")
#define REPLACEMENT_ENGLISH                 ASCII_STR("english")
#define REPLACEMENT_ENGLISH_UK              ASCII_STR("english_uk")
#define REPLACEMENT_FINNISH                 ASCII_STR("finnish")
#define REPLACEMENT_FRENCH                  ASCII_STR("french")
#define REPLACEMENT_GERMAN                  ASCII_STR("german")
#define REPLACEMENT_GREEK                   ASCII_STR("greek")
#define REPLACEMENT_HEBREW                  ASCII_STR("hebrew")
#define REPLACEMENT_ITALIAN                 ASCII_STR("italian")
#define REPLACEMENT_JAPANESE                ASCII_STR("japanese")
#define REPLACEMENT_KOREAN                  ASCII_STR("korean")
#define REPLACEMENT_POLISH                  ASCII_STR("polish")
#define REPLACEMENT_RUSSIAN                 ASCII_STR("russian")
#define REPLACEMENT_SLOVAK                  ASCII_STR("slovak")
#define REPLACEMENT_SPANISH                 ASCII_STR("spanish")
#define REPLACEMENT_SWEDISH                 ASCII_STR("swedish")
#define REPLACEMENT_TURKISH                 ASCII_STR("turkish")
#define REPLACEMENT_NORWEGIAN               ASCII_STR("norwegian")
#define REPLACEMENT_HUNGARIAN               ASCII_STR("hungarian")
//#define   REPLACEMENT_BULGARIAN           ASCII_STR("bulgarian")
#define REPLACEMENT_CHINESE_TRADITIONAL     ASCII_STR("chinese_traditional")
#define REPLACEMENT_CHINESE_SIMPLIFIED      ASCII_STR("chinese_simplified")
#define REPLACEMENT_PORTUGUESE              ASCII_STR("portuguese")
#define REPLACEMENT_PORTUGUESE_BRAZILIAN    ASCII_STR("portuguese_brazilian")
#define REPLACEMENT_THAI                    ASCII_STR("thai")
#define REPLACEMENT_CATALAN                 ASCII_STR("catalan")

// Length of SUBSTITUTE_... to replace it with real values.
#define REPLACELENGTH_INST              7
#define REPLACELENGTH_PROG              7
#define REPLACELENGTH_USER              7
#define REPLACELENGTH_WORK              7
#define REPLACELENGTH_HOME              7
#define REPLACELENGTH_TEMP              7
#define REPLACELENGTH_INSTPATH          11
#define REPLACELENGTH_PROGPATH          11
#define REPLACELENGTH_USERPATH          11
#define REPLACELENGTH_INSTURL           10
#define REPLACELENGTH_PROGURL           10
#define REPLACELENGTH_USERURL           10
#define REPLACELENGTH_PATH              7
#define REPLACELENGTH_LANG              7
#define REPLACELENGTH_LANGID            9
#define REPLACELENGTH_VLANG             8
#define REPLACELENGTH_WORKDIRURL        13

// Name of the pre defined path variables
#define VARIABLE_INST                   "$(inst)"
#define VARIABLE_PROG                   "$(prog)"
#define VARIABLE_USER                   "$(user)"
#define VARIABLE_WORK                   "$(work)"
#define VARIABLE_HOME                   "$(home)"
#define VARIABLE_TEMP                   "$(temp)"
#define VARIABLE_PATH                   "$(path)"
#define VARIABLE_LANG                   "$(lang)"
#define VARIABLE_LANGID                 "$(langid)"
#define VARIABLE_VLANG                  "$(vlang)"
#define VARIABLE_INSTPATH               "$(instpath)"
#define VARIABLE_PROGPATH               "$(progpath)"
#define VARIABLE_USERPATH               "$(userpath)"
#define VARIABLE_INSTURL                "$(insturl)"
#define VARIABLE_PROGURL                "$(progurl)"
#define VARIABLE_USERURL                "$(userurl)"
#define VARIABLE_WORKDIRURL             "$(workdirurl)"


using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{

struct FixedVariable
{
    const char*     pVarName;
    PreDefVariable  nEnumValue;
    int             nStrLen;
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
    { "WINDOWS" ,   7   },
    { "UNIX"    ,   4   },
    { "SOLARIS" ,   7   },
    { "LINUX"   ,   5   },
    { ""        ,   0   }   // unknown
};

// Table with valid environment variables
// Name of the environment type as a char* and
// the length of the string.
static TableEntry aEnvTable[ET_COUNT] =
{
    { "HOST"        ,   4   },
    { "YPDOMAIN"    ,   8   },
    { "DNSDOMAIN"   ,   9   },
    { "NTDOMAIN"    ,   8   },
    { "OS"          ,   2   },
    { ""            ,   0   } // unknown
};

// Priority table for the environment types. Lower numbers define
// a higher priority. Equal numbers has the same priority that means
// that the first match wins!!
static sal_Int16 aEnvPrioTable[ET_COUNT] =
{
    1,  // ET_HOST
    2,  // ET_IPDOMAIN
    2,  // ET_DNSDOMAIN
    2,  // ET_NTDOMAIN
    3,  // ET_OS
    99, // ET_UNKNOWN
};

// Table with all fixed/predefined variables supported.
static FixedVariable aFixedVarTable[] =
{
    { VARIABLE_INST,        PREDEFVAR_INST,         REPLACELENGTH_INST          },
    { VARIABLE_PROG,        PREDEFVAR_PROG,         REPLACELENGTH_PROG          },
    { VARIABLE_USER,        PREDEFVAR_USER,         REPLACELENGTH_USER          },
    { VARIABLE_WORK,        PREDEFVAR_WORK,         REPLACELENGTH_WORK          },  // Special variable (transient)!
    { VARIABLE_HOME,        PREDEFVAR_HOME,         REPLACELENGTH_HOME          },
    { VARIABLE_TEMP,        PREDEFVAR_TEMP,         REPLACELENGTH_TEMP          },
    { VARIABLE_LANG,        PREDEFVAR_LANG,         REPLACELENGTH_LANG          },
    { VARIABLE_LANGID,      PREDEFVAR_LANGID,       REPLACELENGTH_LANGID        },
    { VARIABLE_VLANG,       PREDEFVAR_VLANG,        REPLACELENGTH_VLANG         },
    { VARIABLE_INSTPATH,    PREDEFVAR_INSTPATH,     REPLACELENGTH_INSTPATH      },
    { VARIABLE_PROGPATH,    PREDEFVAR_PROGPATH,     REPLACELENGTH_PROGPATH      },
    { VARIABLE_USERPATH,    PREDEFVAR_USERPATH,     REPLACELENGTH_USERPATH      },
    { VARIABLE_INSTURL,     PREDEFVAR_INSTURL,      REPLACELENGTH_INSTURL       },
    { VARIABLE_PROGURL,     PREDEFVAR_PROGURL,      REPLACELENGTH_PROGURL       },
    { VARIABLE_USERURL,     PREDEFVAR_USERURL,      REPLACELENGTH_USERURL       },
    { VARIABLE_WORKDIRURL,  PREDEFVAR_WORKDIRURL,   REPLACELENGTH_WORKDIRURL    }   // Special variable (transient) and don't use for resubstitution!
};

//_________________________________________________________________________________________________________________
//  Implementation helper classes
//_________________________________________________________________________________________________________________
//

WorkPathHelper_Impl::WorkPathHelper_Impl() :
    utl::ConfigItem( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Path/Current" ))),
    m_aPathNameSeq( 1 )
{
    m_aPathNameSeq[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Work" ));
}

rtl::OUString WorkPathHelper_Impl::GetWorkPath()
{
    rtl::OUString aWorkPath;
    Sequence< Any > aValues = GetProperties( m_aPathNameSeq );

    if ( aValues.getLength() == 1 && aValues[0].hasValue() )
         aValues[0] >>= aWorkPath;

    return aWorkPath;
}


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
    m_aListenerNotify( aNotifyLink ),
    m_aSharePointsNodeName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SharePoints" ))),
    m_aDirPropertyName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/Directory" ))),
    m_aEnvPropertyName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/Environment" ))),
    m_aLevelSep( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ))),
    m_bYPDomainRetrieved( sal_False ),
    m_bDNSDomainRetrieved( sal_False ),
    m_bNTDomainRetrieved( sal_False ),
    m_bHostRetrieved( sal_False ),
    m_bOSRetrieved( sal_False )
{
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
            aSharePointNodeName += rtl::OUString::createFromAscii( "/" );
            aSharePointNodeName += aSharePointNames[ nSharePoints ];

            SubstituteRuleVector aRuleSet;
            ReadSharePointRuleSetFromConfiguration( aSharePointNames[ nSharePoints ], aSharePointNodeName, aRuleSet );
            if ( aRuleSet.size() > 0 )
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

void SubstitutePathVariables_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames )
{
    // NOT implemented yet!
}

//_________________________________________________________________________________________________________________
//  private methods
//_________________________________________________________________________________________________________________
//

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

sal_Bool SubstitutePathVariables_Impl::FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule )
{
    sal_Bool bResult = sal_False;

    if ( aRuleSet.size() >= 1 )
    {
        Wildcard    aPatternMatch;
        sal_Int16   nPrioCurrentRule = aEnvPrioTable[ ET_UNKNOWN ];
        for ( sal_Int32 nIndex = 0; nIndex < (sal_Int32)aRuleSet.size(); nIndex++ )
        {
            const SubstituteRule& aRule = aRuleSet[nIndex];
            EnvironmentType eEnvType    = aRule.aEnvType;

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
                        sal_Bool bMatch = aPatternMatch.match( aHost, aHostStr );
                        if ( bMatch )
                        {
                            aActiveRule         = aRule;
                            bResult             = sal_True;
                            nPrioCurrentRule    = aEnvPrioTable[eEnvType];
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
                        sal_Bool bMatch = aPatternMatch.match( aDomain, aDomainStr );
                        if ( bMatch )
                        {
                            aActiveRule         = aRule;
                            bResult             = sal_True;
                            nPrioCurrentRule    = aEnvPrioTable[eEnvType];
                        }
                    }
                    break;

                    case ET_OS:
                    {
                        // No pattern matching for OS type
                        OperatingSystem eOSType = GetOperatingSystem();

                        sal_Int16 nValue;
                        aRule.aEnvValue >>= nValue;

                        sal_Bool        bUnix = ( eOSType == OS_LINUX ) || ( eOSType == OS_SOLARIS );
                        OperatingSystem eRuleOSType = (OperatingSystem)nValue;

                        // Match if OS identical or rule is set to UNIX and OS is LINUX/SOLARIS!
                        if (( eRuleOSType == eOSType ) || ( eRuleOSType == OS_UNIX && bUnix ))
                        {
                            aActiveRule         = aRule;
                            bResult             = sal_True;
                            nPrioCurrentRule    = aEnvPrioTable[eEnvType];
                        }
                    }
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
        Any             aEnvValue;
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
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                    (   SubstitutePathVariables                                  ,
                                            OWeakObject                                             ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider              ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo               ),
                                            DIRECT_INTERFACE( css::util::XStringSubstitution        )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   SubstitutePathVariables             ,
                                            css::lang::XTypeProvider            ,
                                            css::lang::XServiceInfo             ,
                                            css::util::XStringSubstitution
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   SubstitutePathVariables                     ,
                                            ::cppu::OWeakObject                         ,
                                            SERVICENAME_SUBSTITUTEPATHVARIABLES         ,
                                            IMPLEMENTATIONNAME_SUBSTITUTEPATHVARIABLES
                                        )

DEFINE_INIT_SERVICE                     (   SubstitutePathVariables, {} )


SubstitutePathVariables::SubstitutePathVariables( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_aImpl( LINK( this, SubstitutePathVariables, implts_ConfigurationNotify )),
    m_aVarStart( SIGN_STARTVARIABLE ),
    m_aVarEnd( SIGN_ENDVARIABLE ),
    m_xServiceManager( xServiceManager )
{
    SetPredefinedPathVariables( m_aPreDefVars );
    m_aImpl.GetSharePointsRules( m_aSubstVarMap );

    // Init the predefined/fixed variable to index hash map
    for ( int i = 0; i < PREDEFVAR_COUNT; i++ )
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
        if ( i != PREDEFVAR_WORKDIRURL )
        {
            // Special path variables, don't include into automatic resubstituion search!
            // $(workdirurl) is not allowed to resubstitute! This variable is the value of path settings entry
            // and it could be possible that it will be resubstituted by itself!!
            // Example: WORK_PATH=c:\test, $(workdirurl)=WORK_PATH => WORK_PATH=$(workdirurl) and this cannot be substituted!
            ReSubstFixedVarOrder aFixedVar;
            aFixedVar.eVariable         = aFixedVarTable[i].nEnumValue;
            aFixedVar.nVarValueLength   = m_aPreDefVars.m_FixedVar[(sal_Int32)aFixedVar.eVariable].getLength();
            m_aReSubstFixedVarOrder.push_back( aFixedVar );
        }
    }
    m_aReSubstFixedVarOrder.sort();

    // Sort user variables to path length
    SubstituteVariables::const_iterator pIter;
    for ( pIter = m_aSubstVarMap.begin(); pIter != m_aSubstVarMap.end(); pIter++ )
    {
        ReSubstUserVarOrder aUserOrderVar;
        rtl::OUStringBuffer aStrBuffer( pIter->second.aSubstVariable.getLength() );
        aStrBuffer.append( m_aVarStart );
        aStrBuffer.append( pIter->second.aSubstVariable );
        aStrBuffer.append( m_aVarEnd );
        aUserOrderVar.aVarName          = aStrBuffer.makeStringAndClear();
        aUserOrderVar.nVarValueLength   = pIter->second.aSubstVariable.getLength();
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
    ResetableGuard aLock( m_aLock );
    return impl_substituteVariable( aText, bSubstRequired );
}

rtl::OUString SAL_CALL SubstitutePathVariables::reSubstituteVariables( const ::rtl::OUString& aText )
throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    return impl_reSubstituteVariables( aText );
}

rtl::OUString SAL_CALL SubstitutePathVariables::getSubstituteVariableValue( const ::rtl::OUString& aVariable )
throw ( NoSuchElementException, RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    return impl_getSubstituteVariableValue( aVariable );
}

//_________________________________________________________________________________________________________________
//  protected methods
//_________________________________________________________________________________________________________________
//

IMPL_LINK( SubstitutePathVariables, implts_ConfigurationNotify, SubstitutePathNotify*, pSubstVarNotify )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    return 0;
}

rtl::OUString SubstitutePathVariables::ConvertOSLtoUCBURL( const rtl::OUString& aOSLCompliantURL ) const
{
    String          aResult;
    rtl::OUString   aTemp;

    osl::FileBase::getSystemPathFromFileURL( aOSLCompliantURL, aTemp );
    utl::LocalFileHelper::ConvertPhysicalNameToURL( aTemp, aResult );

    // Not all OSL URL's can be mapped to UCB URL's!
    if ( aResult.Len() == 0 )
        return aOSLCompliantURL;
    else
        return rtl::OUString( aResult );
}

rtl::OUString SubstitutePathVariables::GetWorkVariableValue() const
{
    osl::Security   aSecurity;
    rtl::OUString   aHomePath;

    aSecurity.getHomeDir( aHomePath );
    return ConvertOSLtoUCBURL( aHomePath );
}

rtl::OUString SubstitutePathVariables::GetHomeVariableValue() const
{
    osl::Security   aSecurity;
    rtl::OUString   aConfigPath;
    aSecurity.getConfigDir( aConfigPath );
    return ConvertOSLtoUCBURL( aConfigPath );
}

rtl::OUString SubstitutePathVariables::GetPathVariableValue() const
{
    return rtl::OUString::createFromAscii( getenv( "path" ) );
}

rtl::OUString SubstitutePathVariables::GetLanguageString( LanguageType aLanguageType ) const
{
    rtl::OUString aLangStr;

    switch ( aLanguageType )
    {
        case LANGUAGE_ARABIC                :
        case LANGUAGE_ARABIC_IRAQ           :
        case LANGUAGE_ARABIC_EGYPT          :
        case LANGUAGE_ARABIC_LIBYA          :
        case LANGUAGE_ARABIC_ALGERIA        :
        case LANGUAGE_ARABIC_MOROCCO        :
        case LANGUAGE_ARABIC_TUNISIA        :
        case LANGUAGE_ARABIC_OMAN           :
        case LANGUAGE_ARABIC_YEMEN          :
        case LANGUAGE_ARABIC_SYRIA          :
        case LANGUAGE_ARABIC_JORDAN         :
        case LANGUAGE_ARABIC_LEBANON        :
        case LANGUAGE_ARABIC_KUWAIT         :
        case LANGUAGE_ARABIC_UAE            :
        case LANGUAGE_ARABIC_BAHRAIN        :
        case LANGUAGE_ARABIC_QATAR          :   aLangStr = REPLACEMENT_ARABIC;
                                                break ;

        case LANGUAGE_CZECH                 :   aLangStr = REPLACEMENT_CZECH;
                                                break ;

        case LANGUAGE_DANISH                :   aLangStr = REPLACEMENT_DANISH;
                                                break ;

        case LANGUAGE_DUTCH                 :
        case LANGUAGE_DUTCH_BELGIAN         :   aLangStr = REPLACEMENT_DUTCH;
                                                break ;

        case LANGUAGE_ENGLISH               :
        case LANGUAGE_ENGLISH_AUS           :
        case LANGUAGE_ENGLISH_CAN           :
        case LANGUAGE_ENGLISH_NZ            :
        case LANGUAGE_ENGLISH_EIRE          :
        case LANGUAGE_ENGLISH_SAFRICA       :
        case LANGUAGE_ENGLISH_JAMAICA       :
        case LANGUAGE_ENGLISH_CARRIBEAN     :
        case LANGUAGE_ENGLISH_BELIZE        :
        case LANGUAGE_ENGLISH_TRINIDAD      :
        case LANGUAGE_ENGLISH_ZIMBABWE      :
        case LANGUAGE_ENGLISH_PHILIPPINES   :
        case LANGUAGE_ENGLISH_US            :   aLangStr = REPLACEMENT_ENGLISH;
                                                break ;

        case LANGUAGE_ENGLISH_UK            :   aLangStr = REPLACEMENT_ENGLISH_UK;
                                                break ;

        case LANGUAGE_FINNISH               :   aLangStr = REPLACEMENT_FINNISH;
                                                break ;

        case LANGUAGE_FRENCH                :
        case LANGUAGE_FRENCH_BELGIAN        :
        case LANGUAGE_FRENCH_CANADIAN       :
        case LANGUAGE_FRENCH_SWISS          :
        case LANGUAGE_FRENCH_LUXEMBOURG     :
        case LANGUAGE_FRENCH_MONACO         :   aLangStr = REPLACEMENT_FRENCH;
                                                break ;

        case LANGUAGE_GERMAN                :
        case LANGUAGE_GERMAN_SWISS          :
        case LANGUAGE_GERMAN_AUSTRIAN       :
        case LANGUAGE_GERMAN_LUXEMBOURG     :
        case LANGUAGE_GERMAN_LIECHTENSTEIN  :   aLangStr = REPLACEMENT_GERMAN;
                                                break ;

        case LANGUAGE_GREEK                 :   aLangStr = REPLACEMENT_GREEK;
                                                break ;

        case LANGUAGE_HEBREW                :   aLangStr = REPLACEMENT_HEBREW;
                                                break ;

        case LANGUAGE_ITALIAN               :
        case LANGUAGE_ITALIAN_SWISS         :   aLangStr = REPLACEMENT_ITALIAN;
                                                break ;

        case LANGUAGE_JAPANESE              :   aLangStr = REPLACEMENT_JAPANESE;
                                                break ;

        case LANGUAGE_KOREAN                :
        case LANGUAGE_KOREAN_JOHAB          :   aLangStr = REPLACEMENT_KOREAN;
                                                break ;

        case LANGUAGE_POLISH                :   aLangStr = REPLACEMENT_POLISH;
                                                break ;

        case LANGUAGE_RUSSIAN               :   aLangStr = REPLACEMENT_RUSSIAN;
                                                break ;

        case LANGUAGE_SLOVAK                :   aLangStr = REPLACEMENT_SLOVAK;
                                                break ;

        case LANGUAGE_SPANISH               :
        case LANGUAGE_SPANISH_MEXICAN       :
        case LANGUAGE_SPANISH_MODERN        :
        case LANGUAGE_SPANISH_GUATEMALA     :
        case LANGUAGE_SPANISH_COSTARICA     :
        case LANGUAGE_SPANISH_PANAMA        :
        case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
        case LANGUAGE_SPANISH_VENEZUELA     :
        case LANGUAGE_SPANISH_COLOMBIA      :
        case LANGUAGE_SPANISH_PERU          :
        case LANGUAGE_SPANISH_ARGENTINA     :
        case LANGUAGE_SPANISH_ECUADOR       :
        case LANGUAGE_SPANISH_CHILE         :
        case LANGUAGE_SPANISH_URUGUAY       :
        case LANGUAGE_SPANISH_PARAGUAY      :
        case LANGUAGE_SPANISH_BOLIVIA       :
        case LANGUAGE_SPANISH_EL_SALVADOR   :
        case LANGUAGE_SPANISH_HONDURAS      :
        case LANGUAGE_SPANISH_NICARAGUA     :
        case LANGUAGE_SPANISH_PUERTO_RICO   :   aLangStr = REPLACEMENT_SPANISH;
                                                break ;

        case LANGUAGE_SWEDISH               :
        case LANGUAGE_SWEDISH_FINLAND       :   aLangStr = REPLACEMENT_SWEDISH;
                                                break ;

        case LANGUAGE_TURKISH               :   aLangStr = REPLACEMENT_TURKISH;
                                                break ;

        case LANGUAGE_NORWEGIAN             :
        case LANGUAGE_NORWEGIAN_BOKMAL      :
        case LANGUAGE_NORWEGIAN_NYNORSK     :   aLangStr = REPLACEMENT_NORWEGIAN;
                                                break ;

        case LANGUAGE_HUNGARIAN             :   aLangStr = REPLACEMENT_HUNGARIAN;
                                                break ;

    /*              case LANGUAGE_BULGARIAN             :   aLangStr = REPLACEMENT_BULGARIAN;
                                                break ; */

        case LANGUAGE_CHINESE_TRADITIONAL   :   aLangStr = REPLACEMENT_CHINESE_TRADITIONAL;
                                                break ;

        case LANGUAGE_CHINESE_SIMPLIFIED    :   aLangStr = REPLACEMENT_CHINESE_SIMPLIFIED;
                                                break ;

        case LANGUAGE_PORTUGUESE            :   aLangStr = REPLACEMENT_PORTUGUESE;
                                                break ;

        case LANGUAGE_PORTUGUESE_BRAZILIAN  :   aLangStr = REPLACEMENT_PORTUGUESE_BRAZILIAN;
                                                break ;

        case LANGUAGE_THAI                  :   aLangStr = REPLACEMENT_THAI;
                                                break ;

        case LANGUAGE_CATALAN               :   aLangStr = REPLACEMENT_CATALAN;
                                                break ;

        default                             :   // fallback for L10N-framework => ISO-Code
                                                {
                                                    String rLangStr;
                                                    String rCountry;
                                                    ConvertLanguageToIsoNames( aLanguageType, rLangStr, rCountry);
                                                    if ( rCountry.Len()) {
                                                        rLangStr += String::CreateFromAscii( "-" );
                                                        rLangStr += rCountry;
                                                    }
                                                    aLangStr = rLangStr;
                                                }
                                                break ;
    }

    return aLangStr;
}

rtl::OUString SubstitutePathVariables::impl_substituteVariable( const ::rtl::OUString& rText, sal_Bool bSubstRequired )
throw ( NoSuchElementException, RuntimeException )
{
    // This is maximal recursive depth supported!
    const sal_Int32 nMaxRecursiveDepth = 8;

    rtl::OUString   aWorkText = rText;
    rtl::OUString   aResult;

    // Use vector with strings to detect endless recursions!
    std::vector< rtl::OUString > aEndlessRecursiveDetector;

    // Search for first occure of "$(...".
    sal_Int32   nDepth = 0;
    sal_Int32   bSubstitutionCompleted = sal_False;
    sal_Int32   nPosition   = aWorkText.indexOf( m_aVarStart ); // = first position of "$(" in string
    sal_Int32   nLength = 0; // = count of letters from "$(" to ")" in string
    sal_Bool    bVarNotSubstituted = sal_False;

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
    sal_Bool bWorkRetrieved         = sal_False;
    sal_Bool bWorkDirURLRetrieved   = sal_False;
    while ( !bSubstitutionCompleted && nDepth < nMaxRecursiveDepth )
    {
        while ( ( nPosition != STRPOS_NOTFOUND ) && ( nLength > 3 ) ) // "$(" ")"
        {
            // YES; Get the next variable for replace.
            sal_Int32       nReplaceLength  = 0;
            rtl::OUString   aReplacement;
            rtl::OUString   aSubString      = aWorkText.copy( nPosition, nLength );
            rtl::OUString   aSubVarString;

            // Path variables are not case sensitive!
            aSubVarString = aSubString.toAsciiLowerCase();
            VarNameToIndexMap::const_iterator pIter = m_aPreDefVarMap.find( aSubVarString );
            if ( pIter != m_aPreDefVarMap.end() )
            {
                // Fixed/Predefined variable found
                PreDefVariable nIndex = (PreDefVariable)pIter->second;

                // Determine variable value and length from array/table
                if ( nIndex == PREDEFVAR_WORK && !bWorkRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ] = GetWorkVariableValue();
                    bWorkRetrieved = sal_True;
                }
                else if ( nIndex == PREDEFVAR_WORKDIRURL && !bWorkDirURLRetrieved )
                {
                    // Transient value, retrieve it again
                    m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ] = m_aWorkPathHelper.GetWorkPath();
                    bWorkDirURLRetrieved = sal_True;
                }

                aReplacement = m_aPreDefVars.m_FixedVar[ (PreDefVariable)nIndex ];
                nReplaceLength = nLength;
            }
            else
            {
                // Extract the variable name and try to find in the user defined variable set
                rtl::OUString aVarName = aSubString.copy( 2, nLength-3 );
                SubstituteVariables::const_iterator pIter = m_aSubstVarMap.find( aVarName );
                if ( pIter != m_aSubstVarMap.end() )
                {
                    // found!
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
                bVarNotSubstituted = sal_False;
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
            for ( sal_Int32 i=0; i < (sal_Int32)aEndlessRecursiveDetector.size(); i++ )
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
            // variable in text but unknwon!
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
            if ( aTemp.getLength() )
                aURL = INetURLObject( aTemp ).GetMainURL( INetURLObject::NO_DECODE );
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
    sal_Bool        bResubstitutionCompleted    = sal_False;
    sal_Bool        bVariableFound              = sal_False;
    sal_Int32       nURLLen                     = aURL.getLength();

    // Get transient predefined path variable $(work) value before starting resubstitution
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();

    while ( !bResubstitutionCompleted )
    {
        ReSubstFixedVarOrderVector::const_iterator pIterFixed;
        for ( pIterFixed = m_aReSubstFixedVarOrder.begin(); pIterFixed != m_aReSubstFixedVarOrder.end(); pIterFixed++ )
        {
            rtl::OUString aValue = m_aPreDefVars.m_FixedVar[ (sal_Int32)pIterFixed->eVariable ];
            sal_Int32 nPos = aURL.indexOf( aValue );
            if ( nPos >= 0 )
            {
                sal_Bool bMatch = sal_True;
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
                    bVariableFound = sal_True; // Resubstitution not finished yet!
                    break;
                }
            }
        }

        // This part can be iteratered more than one time as variables can contain variables again!
        ReSubstUserVarOrderVector::const_iterator pIterUser;
        for ( pIterUser = m_aReSubstUserVarOrder.begin(); pIterUser != m_aReSubstUserVarOrder.end(); pIterUser++ )
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
                bVariableFound = sal_True;  // Resubstitution not finished yet!
            }
        }

        if ( !bVariableFound )
            bResubstitutionCompleted = sal_True;
        else
            bVariableFound = sal_False; // Next resubstitution
    }

    return aURL;
}

// This method support both request schemes "$("<varname>")" or "<varname>".
::rtl::OUString SubstitutePathVariables::impl_getSubstituteVariableValue( const ::rtl::OUString& rVariable )
throw ( NoSuchElementException, RuntimeException )
{
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

    VarNameToIndexMap::const_iterator pIter = m_aPreDefVarMap.find( ( nPos == -1 ) ? aVariable : rVariable );

    // Fixed/Predefined variable
    if ( pIter != m_aPreDefVarMap.end() )
    {
        PreDefVariable nIndex = (PreDefVariable)pIter->second;
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

    // SAFE-IMPOSSIBLE CASE!
    return rtl::OUString();
}

void SubstitutePathVariables::SetPredefinedPathVariables( PredefinedPathVariables& aPreDefPathVariables )
{
    utl::ConfigManager* pCfgMgr = utl::ConfigManager::GetConfigManager();

    Any aAny;
    ::rtl::OUString aOfficePath;
    ::rtl::OUString aUserPath;
    ::rtl::OUString aTmp;
    ::rtl::OUString aTmp2;
    String          aResult;

    // Get inspath and userpath from bootstrap mechanism in every case as file URL
    ::utl::Bootstrap::PathStatus aState;
    ::rtl::OUString              sVal  ;

    aState = utl::Bootstrap::locateBaseInstallation( sVal );
    if( aState==::utl::Bootstrap::PATH_EXISTS )
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ] = ConvertOSLtoUCBURL( sVal );
    else
        LOG_ERROR( "SubstitutePathVariables::SetPredefinedPathVariables", "Bootstrap code has no value for instpath!");

    aState = utl::Bootstrap::locateUserData( sVal );
    if( aState == ::utl::Bootstrap::PATH_EXISTS )
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ] = ConvertOSLtoUCBURL( sVal );
    else
        LOG_ERROR( "SubstitutePathVariables::SetPredefinedPathVariables", "Bootstrap code has no value for userpath");

    // Get insturl and userurl from configuration (xml files)
    // But if they doesn't exist (e.g. is true for fat office!) use instpath and userpath instead of this.
    // Set $(inst), $(instpath), $(insturl)
    aAny = pCfgMgr->GetDirectConfigProperty( utl::ConfigManager::OFFICEINSTALLURL );
    if ( !aAny.hasValue() || ( aAny >>= aOfficePath ) )
    {
        // "OFFICEINSTALLURL" is a UCB compatible URL for the office installation directory
        // in the Webtop this MUST be set, in FATOffice it is the osl URL of the instpath
        if ( aOfficePath.getLength() )
        {
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ]   = aOfficePath;
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTURL ]    = aOfficePath;
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INST ]       = aOfficePath;
        }
        else
        {
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTURL ]    = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ];
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INST ]       = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_INSTPATH ];
        }
    }
    else
        LOG_ERROR( "SubstitutePathVariables::SetPredefinedPathVariables", "Wrong any typ for office install URL!" );

    // Get the user installation directory
    // Set $(user), $(userpath), $(userurl)
    aAny = pCfgMgr->GetDirectConfigProperty( utl::ConfigManager::USERINSTALLURL );
    if ( !aAny.hasValue() || ( aAny >>= aUserPath ) )
    {
        if ( aUserPath.getLength() )
        {
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ]   = aUserPath;
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERURL ]    = aUserPath;
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USER ]       = aUserPath;
        }
        else
        {
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERURL ]    = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ];
            aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USER ]       = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_USERPATH ];
        }
    }
    else
        LOG_ERROR( "SubstitutePathVariables::SetPredefinedPathVariables", "Wrong any type for user install URL!" );

    // Detect the program directory
    // Set $(prog), $(progpath), $(progurl)
    rtl::OUString aProgName;
    ::vos::OStartupInfo aInfo;
    aInfo.getExecutableFile( aProgName );
    sal_Int32 lastIndex = aProgName.lastIndexOf('/');
    if ( lastIndex >= 0 )
    {
        ::rtl::OUString aTmpProgPath;
        aTmp = aProgName.copy( 0, lastIndex );
        INetURLObject aObj( aTmp );

        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ]   = ConvertOSLtoUCBURL( aObj.GetMainURL(INetURLObject::NO_DECODE) );
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGURL ]    = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ];
        aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROG ]       = aPreDefPathVariables.m_FixedVar[ PREDEFVAR_PROGPATH ];
    }

    // Detect the language type of the current office
    aPreDefPathVariables.m_eLanguageType = LANGUAGE_ENGLISH_US;
    Any aLocale = utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( utl::ConfigManager::LOCALE );
    rtl::OUString aLocaleStr;
    if ( aLocale >>= aLocaleStr )
        aPreDefPathVariables.m_eLanguageType = ConvertIsoStringToLanguage( aLocaleStr );
    else
    {
        LOG_ERROR( "SubstitutePathVariables::SetPredefinedPathVariables", "Wrong Any type for language!" );
    }

    // Set $(lang)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_LANG ] = ConvertOSLtoUCBURL(
        rtl::OUString::createFromAscii( ResMgr::GetLang( aPreDefPathVariables.m_eLanguageType, 0 ) ));

    // Set $(vlang)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_VLANG ] = GetLanguageString( aPreDefPathVariables.m_eLanguageType );

    // Set $(langid)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_LANGID ] = rtl::OUString::valueOf( (sal_Int32)aPreDefPathVariables.m_eLanguageType );

    // Set the other pre defined path variables
    // Set $(work)
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_WORK ] = GetWorkVariableValue();
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_HOME ] = GetHomeVariableValue();

    // Set $(workdirurl) this is the value of the path PATH_WORK which doesn't make sense
    // anymore because the path settings service has this value! It can deliver this value more
    // quickly than the substitution service!
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_WORKDIRURL ] = m_aWorkPathHelper.GetWorkPath();

    // Set $(temp)
    osl::FileBase::getTempDirURL( aTmp );
    aPreDefPathVariables.m_FixedVar[ PREDEFVAR_TEMP ] = ConvertOSLtoUCBURL( aTmp );
}

} // namespace framework
