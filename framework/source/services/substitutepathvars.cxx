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

#include <helper/networkdomain.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>

#include <unotools/bootstrap.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/socket.hxx>
#include <osl/process.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <tools/link.hxx>
#include <tools/urlobj.hxx>
#include <tools/resmgr.hxx>
#include <tools/wldcrd.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Paths.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include <unordered_map>
#include <string.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace framework;

namespace {

// Must be zero value based
enum EnvironmentType
{
        ET_HOST = 0             ,
        ET_YPDOMAIN             ,
        ET_DNSDOMAIN    ,
        ET_NTDOMAIN             ,
        ET_OS                   ,
        ET_UNKNOWN              ,
        ET_COUNT
};

// Must be zero value based
enum OperatingSystem
{
        OS_WINDOWS = 0,
        OS_UNIX         ,
        OS_SOLARIS      ,
        OS_LINUX        ,
        OS_UNKNOWN      ,
        OS_COUNT
};

struct SubstituteRule
{
    SubstituteRule()
        : aEnvType(ET_UNKNOWN)
    {}

    SubstituteRule( const OUString& aVarName,
                    const OUString& aValue,
                    const css::uno::Any& aVal,
                    EnvironmentType aType )
        : aSubstVariable(aVarName)
        , aSubstValue(aValue)
        , aEnvValue(aVal)
        , aEnvType(aType)
    {}

    OUString            aSubstVariable;
    OUString            aSubstValue;
    css::uno::Any       aEnvValue;
    EnvironmentType     aEnvType;
};

typedef std::unordered_map<OUString, SubstituteRule, OUStringHash>
    SubstituteVariables;

typedef std::vector< SubstituteRule > SubstituteRuleVector;
class SubstitutePathVariables_Impl : public utl::ConfigItem
{
    public:
        SubstitutePathVariables_Impl();
        virtual ~SubstitutePathVariables_Impl();

        static OperatingSystem GetOperatingSystemFromString( const OUString& );
        static EnvironmentType GetEnvTypeFromString( const OUString& );

        void                   GetSharePointsRules( SubstituteVariables& aSubstVarMap );

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. */
        virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    private:

        virtual void ImplCommit() override;

        // Wrapper methods for low-level functions
        const OUString&    GetYPDomainName();
        const OUString&    GetDNSDomainName();
        const OUString&    GetNTDomainName();
        const OUString&    GetHostName();

        bool  FilterRuleSet(const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule);

        void  ReadSharePointsFromConfiguration(css::uno::Sequence< OUString >& aSharePointsSeq);
        void  ReadSharePointRuleSetFromConfiguration(const OUString& aSharePointName,
                  const OUString& aSharePointNodeName,
                  SubstituteRuleVector& aRuleSet);

        // Stored values for domains and host
        bool      m_bYPDomainRetrieved;
        OUString  m_aYPDomain;
        bool      m_bDNSDomainRetrieved;
        OUString  m_aDNSDomain;
        bool      m_bNTDomainRetrieved;
        OUString  m_aNTDomain;
        bool      m_bHostRetrieved;
        OUString  m_aHost;

        const OUString    m_aSharePointsNodeName;
        const OUString    m_aDirPropertyName;
        const OUString    m_aEnvPropertyName;
        const OUString    m_aLevelSep;
};

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

struct ReSubstUserVarOrder
{
    sal_Int32       nVarValueLength;
    OUString   aVarName;

    bool operator< ( const ReSubstUserVarOrder& aUserVarOrder ) const
    {
        // Reverse operator< to have high to low ordering
        return ( nVarValueLength > aUserVarOrder.nVarValueLength );
    }
};

typedef std::list< ReSubstFixedVarOrder > ReSubstFixedVarOrderVector;
typedef std::list< ReSubstUserVarOrder > ReSubstUserVarOrderVector;
typedef ::cppu::WeakComponentImplHelper<
    css::util::XStringSubstitution,
    css::lang::XServiceInfo > SubstitutePathVariables_BASE;

class SubstitutePathVariables : private cppu::BaseMutex,
                                public SubstitutePathVariables_BASE
{
friend class SubstitutePathVariables_Impl;

public:
    explicit SubstitutePathVariables(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~SubstitutePathVariables();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.PathSubstitution");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.util.PathSubstitution" };
        return aSeq;
    }

    // XStringSubstitution
    virtual OUString SAL_CALL substituteVariables( const OUString& aText, sal_Bool bSubstRequired )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL reSubstituteVariables( const OUString& aText )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSubstituteVariableValue( const OUString& variable )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

protected:
    void            SetPredefinedPathVariables();

    // Special case (transient) values can change during runtime!
    // Don't store them in the pre defined struct
    OUString   GetWorkPath() const;
    OUString   GetWorkVariableValue() const;
    OUString   GetPathVariableValue() const;

    OUString   GetHomeVariableValue() const;

    // XStringSubstitution implementation methods
    OUString impl_substituteVariable( const OUString& aText, bool bSustRequired )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException);
    OUString impl_reSubstituteVariables( const OUString& aText )
        throw (css::uno::RuntimeException);
    OUString const & impl_getSubstituteVariableValue( const OUString& variable )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException);

private:
    typedef std::unordered_map<OUString, PreDefVariable, OUStringHash>
        VarNameToIndexMap;

    VarNameToIndexMap            m_aPreDefVarMap;         // Mapping from pre-def variable names to enum for array access
    SubstituteVariables          m_aSubstVarMap;          // Active rule set map indexed by variable name!
    PredefinedPathVariables      m_aPreDefVars;           // All predefined variables
    SubstitutePathVariables_Impl m_aImpl;                 // Implementation class that access the configuration
    ReSubstFixedVarOrderVector   m_aReSubstFixedVarOrder; // To speed up resubstitution fixed variables (order for lookup)
    ReSubstUserVarOrderVector    m_aReSubstUserVarOrder;  // To speed up resubstitution user variables
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

struct FixedVariable
{
    const char*     pVarName;
    PreDefVariable  nEnumValue;
    bool            bAbsPath;
};

struct TableEntry
{
    const char* pOSString;
    sal_Int32   nStrLen;
};

// Table with valid operating system strings
// Name of the os as char* and the length
// of the string
static const TableEntry aOSTable[OS_COUNT] =
{
    { RTL_CONSTASCII_STRINGPARAM("WINDOWS") },
    { RTL_CONSTASCII_STRINGPARAM("UNIX") },
    { RTL_CONSTASCII_STRINGPARAM("SOLARIS") },
    { RTL_CONSTASCII_STRINGPARAM("LINUX") },
    { RTL_CONSTASCII_STRINGPARAM("") } // unknown
};

// Table with valid environment variables
// Name of the environment type as a char* and
// the length of the string.
static const TableEntry aEnvTable[ET_COUNT] =
{
    { RTL_CONSTASCII_STRINGPARAM("HOST") },
    { RTL_CONSTASCII_STRINGPARAM("YPDOMAIN") },
    { RTL_CONSTASCII_STRINGPARAM("DNSDOMAIN") },
    { RTL_CONSTASCII_STRINGPARAM("NTDOMAIN") },
    { RTL_CONSTASCII_STRINGPARAM("OS") },
    { RTL_CONSTASCII_STRINGPARAM("") } // unknown
};

// Priority table for the environment types. Lower numbers define
// a higher priority. Equal numbers has the same priority that means
// that the first match wins!!
static const sal_Int16 aEnvPrioTable[ET_COUNT] =
{
    1,      // ET_HOST
    2,      // ET_IPDOMAIN
    2,      // ET_DNSDOMAIN
    2,      // ET_NTDOMAIN
    3,      // ET_OS
    99,     // ET_UNKNOWN
};

// Table with all fixed/predefined variables supported.
static const FixedVariable aFixedVarTable[] =
{
    { "$(inst)",        PREDEFVAR_INST,         true  },
    { "$(prog)",        PREDEFVAR_PROG,         true  },
    { "$(user)",        PREDEFVAR_USER,         true  },
    { "$(work)",        PREDEFVAR_WORK,         true  },      // Special variable (transient)!
    { "$(home)",        PREDEFVAR_HOME,         true  },
    { "$(temp)",        PREDEFVAR_TEMP,         true  },
    { "$(path)",        PREDEFVAR_PATH,         true  },
    { "$(username)",    PREDEFVAR_USERNAME,     false },
    { "$(langid)",      PREDEFVAR_LANGID,       false },
    { "$(vlang)",       PREDEFVAR_VLANG,        false },
    { "$(instpath)",    PREDEFVAR_INSTPATH,     true  },
    { "$(progpath)",    PREDEFVAR_PROGPATH,     true  },
    { "$(userpath)",    PREDEFVAR_USERPATH,     true  },
    { "$(insturl)",     PREDEFVAR_INSTURL,      true  },
    { "$(progurl)",     PREDEFVAR_PROGURL,      true  },
    { "$(userurl)",     PREDEFVAR_USERURL,      true  },
    { "$(workdirurl)",  PREDEFVAR_WORKDIRURL,   true  },  // Special variable (transient) and don't use for resubstitution!
    { "$(baseinsturl)", PREDEFVAR_BASEINSTURL,  true  },
    { "$(userdataurl)", PREDEFVAR_USERDATAURL,  true  },
    { "$(brandbaseurl)",PREDEFVAR_BRANDBASEURL, true  }
};

//      Implementation helper classes

OperatingSystem SubstitutePathVariables_Impl::GetOperatingSystemFromString( const OUString& aOSString )
{
    for ( int i = 0; i < OS_COUNT; i++ )
    {
        if ( aOSString.equalsIgnoreAsciiCaseAsciiL( aOSTable[i].pOSString, aOSTable[i].nStrLen ))
            return (OperatingSystem)i;
    }

    return OS_UNKNOWN;
}

EnvironmentType SubstitutePathVariables_Impl::GetEnvTypeFromString( const OUString& aEnvTypeString )
{
    for ( int i = 0; i < ET_COUNT; i++ )
    {
        if ( aEnvTypeString.equalsIgnoreAsciiCaseAsciiL( aEnvTable[i].pOSString, aEnvTable[i].nStrLen ))
            return (EnvironmentType)i;
    }

    return ET_UNKNOWN;
}

SubstitutePathVariables_Impl::SubstitutePathVariables_Impl() :
    utl::ConfigItem( OUString( "Office.Substitution" )),
    m_bYPDomainRetrieved( false ),
    m_bDNSDomainRetrieved( false ),
    m_bNTDomainRetrieved( false ),
    m_bHostRetrieved( false ),
    m_aSharePointsNodeName( OUString( "SharePoints" )),
    m_aDirPropertyName( OUString( "/Directory" )),
    m_aEnvPropertyName( OUString( "/Environment" )),
    m_aLevelSep( OUString(  "/" ))
{
    // Enable notification mechanism
    // We need it to get information about changes outside these class on our configuration branch
    Sequence<OUString> aNotifySeq { "SharePoints" };
    EnableNotification( aNotifySeq, true );
}

SubstitutePathVariables_Impl::~SubstitutePathVariables_Impl()
{
}

void SubstitutePathVariables_Impl::GetSharePointsRules( SubstituteVariables& aSubstVarMap )
{
    Sequence< OUString > aSharePointNames;
    ReadSharePointsFromConfiguration( aSharePointNames );

    if ( aSharePointNames.getLength() > 0 )
    {
        sal_Int32 nSharePoints = 0;

        // Read SharePoints container from configuration
        while ( nSharePoints < aSharePointNames.getLength() )
        {
            OUString aSharePointNodeName( m_aSharePointsNodeName );
            aSharePointNodeName += "/";
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

void SubstitutePathVariables_Impl::Notify( const css::uno::Sequence< OUString >& /*aPropertyNames*/ )
{
    // NOT implemented yet!
}

void SubstitutePathVariables_Impl::ImplCommit()
{
}

inline OperatingSystem GetOperatingSystem()
{
#ifdef SOLARIS
    return OS_SOLARIS;
#elif defined LINUX
    return OS_LINUX;
#elif defined WIN32
    return OS_WINDOWS;
#elif defined UNIX
    return OS_UNIX;
#else
    return OS_UNKNOWN;
#endif
}

const OUString& SubstitutePathVariables_Impl::GetYPDomainName()
{
    if ( !m_bYPDomainRetrieved )
    {
        m_aYPDomain = NetworkDomain::GetYPDomainName().toAsciiLowerCase();
        m_bYPDomainRetrieved = true;
    }

    return m_aYPDomain;
}

const OUString& SubstitutePathVariables_Impl::GetDNSDomainName()
{
    if ( !m_bDNSDomainRetrieved )
    {
        OUString   aTemp;
        osl::SocketAddr aSockAddr;
        oslSocketResult aResult;

        OUString aHostName = GetHostName();
        osl::SocketAddr::resolveHostname( aHostName, aSockAddr );
        aTemp = aSockAddr.getHostname( &aResult );

        // DNS domain name begins after the first "."
        sal_Int32 nIndex = aTemp.indexOf( '.' );
        if ( nIndex >= 0 && aTemp.getLength() > nIndex+1 )
            m_aDNSDomain = aTemp.copy( nIndex+1 ).toAsciiLowerCase();
        else
            m_aDNSDomain.clear();

        m_bDNSDomainRetrieved = true;
    }

    return m_aDNSDomain;
}

const OUString& SubstitutePathVariables_Impl::GetNTDomainName()
{
    if ( !m_bNTDomainRetrieved )
    {
        m_aNTDomain = NetworkDomain::GetNTDomainName().toAsciiLowerCase();
        m_bNTDomainRetrieved = true;
    }

    return m_aNTDomain;
}

const OUString& SubstitutePathVariables_Impl::GetHostName()
{
    if (!m_bHostRetrieved)
    {
        oslSocketResult aSocketResult;
        m_aHost = osl::SocketAddr::getLocalHostname( &aSocketResult ).toAsciiLowerCase();
        m_bHostRetrieved = true;
    }

    return m_aHost;
}

bool SubstitutePathVariables_Impl::FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule )
{
    bool bResult = false;

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
                        OUString aHost = GetHostName();
                        OUString aHostStr;
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
                        OUString   aDomain;
                        OUString   aDomainStr;
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

void SubstitutePathVariables_Impl::ReadSharePointsFromConfiguration( Sequence< OUString >& aSharePointsSeq )
{
    //returns all the names of all share point nodes
    aSharePointsSeq = GetNodeNames( m_aSharePointsNodeName );
}

void SubstitutePathVariables_Impl::ReadSharePointRuleSetFromConfiguration(
        const OUString& aSharePointName,
        const OUString& aSharePointNodeName,
        SubstituteRuleVector& rRuleSet )
{
    Sequence< OUString > aSharePointMappingsNodeNames = GetNodeNames( aSharePointNodeName, utl::CONFIG_NAME_LOCAL_PATH );

    sal_Int32 nSharePointMapping = 0;
    while ( nSharePointMapping < aSharePointMappingsNodeNames.getLength() )
    {
        OUString aSharePointMapping( aSharePointNodeName );
        aSharePointMapping += m_aLevelSep;
        aSharePointMapping += aSharePointMappingsNodeNames[ nSharePointMapping ];

        // Read SharePointMapping
        OUString aDirValue;
        OUString aDirProperty( aSharePointMapping );
        aDirProperty += m_aDirPropertyName;

        // Read only the directory property
        Sequence<OUString> aDirPropertySeq { aDirProperty };

        Sequence< Any > aValueSeq = GetProperties( aDirPropertySeq );
        if ( aValueSeq.getLength() == 1 )
            aValueSeq[0] >>= aDirValue;

        // Read the environment setting
        OUString aEnvUsed;
        OUString aEnvProperty( aSharePointMapping );
        aEnvProperty += m_aEnvPropertyName;
        Sequence< OUString > aEnvironmentVariable = GetNodeNames( aEnvProperty );

        // Filter the property which has a value set
        Sequence< OUString > aEnvUsedPropertySeq( aEnvironmentVariable.getLength() );

        OUString aEnvUsePropNameTemplate( aEnvProperty );
        aEnvUsePropNameTemplate += m_aLevelSep;

        for ( sal_Int32 nProperty = 0; nProperty < aEnvironmentVariable.getLength(); nProperty++ )
            aEnvUsedPropertySeq[nProperty] = aEnvUsePropNameTemplate + aEnvironmentVariable[nProperty];

        Sequence< Any > aEnvUsedValueSeq;
        aEnvUsedValueSeq = GetProperties( aEnvUsedPropertySeq );

        OUString aEnvUsedValue;
        for ( sal_Int32 nIndex = 0; nIndex < aEnvironmentVariable.getLength(); nIndex++ )
        {
            if ( aEnvUsedValueSeq[nIndex] >>= aEnvUsedValue )
            {
                aEnvUsed = aEnvironmentVariable[nIndex];
                break;
            }
        }

        // Decode the environment and optional the operating system settings
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

SubstitutePathVariables::SubstitutePathVariables( const Reference< XComponentContext >& xContext ) :
    SubstitutePathVariables_BASE(m_aMutex),
    m_xContext( xContext )
{
    int i;

    SetPredefinedPathVariables();
    m_aImpl.GetSharePointsRules( m_aSubstVarMap );

    // Init the predefined/fixed variable to index hash map
    for ( i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        // Store variable name into struct of predefined/fixed variables
        m_aPreDefVars.m_FixedVarNames[i] = OUString::createFromAscii( aFixedVarTable[i].pVarName );

        // Create hash map entry
        m_aPreDefVarMap.insert( VarNameToIndexMap::value_type(
            m_aPreDefVars.m_FixedVarNames[i], aFixedVarTable[i].nEnumValue ) );
    }

    // Sort predefined/fixed variable to path length
    for ( i = 0; i < PREDEFVAR_COUNT; i++ )
    {
        if (( i != PREDEFVAR_WORKDIRURL ) && ( i != PREDEFVAR_PATH ))
        {
            // Special path variables, don't include into automatic resubstitution search!
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
        aUserOrderVar.aVarName = "$(" + pIter->second.aSubstVariable + ")";
        aUserOrderVar.nVarValueLength = pIter->second.aSubstVariable.getLength();
        m_aReSubstUserVarOrder.push_back( aUserOrderVar );
    }
    m_aReSubstUserVarOrder.sort();
}

SubstitutePathVariables::~SubstitutePathVariables()
{
}

// XStringSubstitution
OUString SAL_CALL SubstitutePathVariables::substituteVariables( const OUString& aText, sal_Bool bSubstRequired )
throw ( NoSuchElementException, RuntimeException, std::exception )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_substituteVariable( aText, bSubstRequired );
}

OUString SAL_CALL SubstitutePathVariables::reSubstituteVariables( const OUString& aText )
throw ( RuntimeException, std::exception )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_reSubstituteVariables( aText );
}

OUString SAL_CALL SubstitutePathVariables::getSubstituteVariableValue( const OUString& aVariable )
throw ( NoSuchElementException, RuntimeException, std::exception )
{
    osl::MutexGuard g(rBHelper.rMutex);
    return impl_getSubstituteVariableValue( aVariable );
}

OUString SubstitutePathVariables::GetWorkPath() const
{
    OUString aWorkPath;
    css::uno::Reference< css::container::XHierarchicalNameAccess > xPaths(officecfg::Office::Paths::Paths::get(m_xContext), css::uno::UNO_QUERY_THROW);
    if (!(xPaths->getByHierarchicalName("['Work']/WritePath") >>= aWorkPath))
        // fallback in case config layer does not return an useable work dir value.
        aWorkPath = GetWorkVariableValue();

    return aWorkPath;
}

OUString SubstitutePathVariables::GetWorkVariableValue() const
{
    OUString aWorkPath;
    boost::optional<OUString> x(officecfg::Office::Paths::Variables::Work::get(m_xContext));
    if (!x)
    {
        // fallback to $HOME in case platform dependent config layer does not return
        // an usable work dir value.
        osl::Security aSecurity;
        aSecurity.getHomeDir( aWorkPath );
    }
    else
        aWorkPath = x.get();
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
        const int PATH_EXTEND_FACTOR = 120;
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
throw ( NoSuchElementException, RuntimeException )
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
    while ( !bSubstitutionCompleted && nDepth < nMaxRecursiveDepth )
    {
        while ( ( nPosition != -1 ) && ( nLength > 3 ) ) // "$(" ")"
        {
            // YES; Get the next variable for replace.
            sal_Int32     nReplaceLength  = 0;
            OUString aReplacement;
            OUString aSubString      = aWorkText.copy( nPosition, nLength );
            OUString aSubVarString;

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

                // Check preconditions to substitute path variables.
                // 1. A path variable can only be substituted if it follows a ';'!
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
                OUString aVarName = aSubString.copy( 2, nLength-3 );
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
                OUString aMsg( "Endless recursion detected. Cannot substitute variables!" );
                throw NoSuchElementException( aMsg, static_cast<cppu::OWeakObject *>(this) );
            }
            else
                aResult = rText;
        }
        else
        {
            // variable in text but unknown!
            if ( bSubstRequired )
            {
                OUString aMsg( "Unknown variable found!" );
                throw NoSuchElementException( aMsg, static_cast<cppu::OWeakObject *>(this) );
            }
            else
                aResult = aWorkText;
        }
    }

    return aResult;
}

OUString SubstitutePathVariables::impl_reSubstituteVariables( const OUString& rURL )
throw ( RuntimeException )
{
    OUString aURL;

    INetURLObject aUrl( rURL );
    if ( !aUrl.HasError() )
        aURL = aUrl.GetMainURL( INetURLObject::NO_DECODE );
    else
    {
        // Convert a system path to a UCB compliant URL before resubstitution
        OUString aTemp;
        if ( osl::FileBase::getFileURLFromSystemPath( rURL, aTemp ) == osl::FileBase::E_None )
        {
            aURL = INetURLObject( aTemp ).GetMainURL( INetURLObject::NO_DECODE );
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

        for (ReSubstFixedVarOrderVector::const_iterator i(
                 m_aReSubstFixedVarOrder.begin());
             i != m_aReSubstFixedVarOrder.end(); ++i)
        {
            OUString aValue = m_aPreDefVars.m_FixedVar[i->eVariable];
            sal_Int32 nPos = aURL.indexOf( aValue );
            if ( nPos >= 0 )
            {
                bool bMatch = true;
                if ( i->eVariable == PREDEFVAR_LANGID ||
                     i->eVariable == PREDEFVAR_VLANG )
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
                    aURL = aURL.replaceAt(
                        nPos, aValue.getLength(),
                        m_aPreDefVars.m_FixedVarNames[i->eVariable]);
                    bVariableFound = true; // Resubstitution not finished yet!
                    break;
                }
            }
        }

        // This part can be iterated more than one time as variables can contain variables again!
        for (ReSubstUserVarOrderVector::const_iterator i(
                 m_aReSubstUserVarOrder.begin());
             i != m_aReSubstUserVarOrder.end(); ++i)
        {
            OUString aVarValue = i->aVarName;
            sal_Int32 nPos = aURL.indexOf( aVarValue );
            if ( nPos >= 0 )
            {
                aURL = aURL.replaceAt(
                    nPos, aVarValue.getLength(), "$(" + aVarValue + ")");
                bVariableFound = true;  // Resubstitution not finished yet!
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
throw ( NoSuchElementException, RuntimeException )
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
                OUString aExceptionText("Unknown variable!");
                throw NoSuchElementException(aExceptionText, static_cast<cppu::OWeakObject *>(this));
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

        OUString aExceptionText("Unknown variable!");
        throw NoSuchElementException(aExceptionText, static_cast<cppu::OWeakObject *>(this));
    }
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
    if (aState == ::utl::Bootstrap::PATH_EXISTS || getenv("SRC_ROOT")) {
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
        m_aPreDefVars.m_FixedVar[ PREDEFVAR_PROGPATH ] = aProgObj.GetMainURL(INetURLObject::NO_DECODE);
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
    OUString aLocaleStr( utl::ConfigManager::getLocale() );
    m_aPreDefVars.m_eLanguageType = LanguageTag::convertToLanguageTypeWithFallback( aLocaleStr );
    // We used to have an else branch here with a SAL_WARN, but that
    // always fired in some unit tests when this code was built with
    // debug=t, so it seems fairly pointless, especially as
    // m_aPreDefVars.m_eLanguageType has been initialized to a
    // default value above anyway.

    // Set $(vlang)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_VLANG ] = aLocaleStr;

    // Set $(langid)
    m_aPreDefVars.m_FixedVar[ PREDEFVAR_LANGID ] = OUString::number( m_aPreDefVars.m_eLanguageType );

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

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(
            static_cast<cppu::OWeakObject *>(new SubstitutePathVariables(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_PathSubstitution_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
