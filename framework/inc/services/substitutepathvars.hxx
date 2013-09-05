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

#ifndef __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_
#define __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <tools/link.hxx>
#include <i18nlangtag/lang.h>

namespace framework
{

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
    SubstituteRule() {}
    SubstituteRule( const OUString& aVarName,
                    const OUString& aValue,
                    const com::sun::star::uno::Any& aVal,
                    EnvironmentType aType ) :
        aSubstVariable( aVarName ), aSubstValue( aValue ), aEnvValue( aVal ), aEnvType( aType ) {}

    OUString            aSubstVariable;
    OUString            aSubstValue;
    com::sun::star::uno::Any aEnvValue;
    EnvironmentType          aEnvType;
};

struct SubstitutePathNotify
{
    SubstitutePathNotify() {};

    const com::sun::star::uno::Sequence<OUString> aPropertyNames;
};

class SubstituteVariables : public ::boost::unordered_map< OUString,
                                                    SubstituteRule,
                                                    OUStringHash,
                                                    ::std::equal_to< OUString > >
{
    public:
        inline void free()
        {
            SubstituteVariables().swap( *this );
        }
};

typedef std::vector< SubstituteRule > SubstituteRuleVector;
class SubstitutePathVariables_Impl : public utl::ConfigItem
{
    public:
        SubstitutePathVariables_Impl( const Link& aNotifyLink );
        virtual ~SubstitutePathVariables_Impl();

        static OperatingSystem GetOperatingSystemFromString( const OUString& );
        static EnvironmentType GetEnvTypeFromString( const OUString& );

        void                   GetSharePointsRules( SubstituteVariables& aSubstVarMap );

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. */
        virtual void Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );
        virtual void Commit();

        private:
            // Wrapper methods for low-level functions
            OperatingSystem         GetOperatingSystem();
            const OUString&    GetYPDomainName();
            const OUString&    GetDNSDomainName();
            const OUString&    GetNTDomainName();
            const OUString&    GetHostName();

            bool                    FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule );

            void                    ReadSharePointsFromConfiguration( com::sun::star::uno::Sequence< OUString >& aSharePointsSeq );
            void                    ReadSharePointRuleSetFromConfiguration( const OUString& aSharePointName,
                                                                                                                                const OUString& aSharePointNodeName,
                                                                                                                                SubstituteRuleVector& aRuleSet );

            // Stored values for domains and host
            bool                    m_bYPDomainRetrieved;
            OUString           m_aYPDomain;
            bool                    m_bDNSDomainRetrieved;
            OUString           m_aDNSDomain;
            bool                    m_bNTDomainRetrieved;
            OUString           m_aNTDomain;
            bool                    m_bHostRetrieved;
            OUString           m_aHost;
            bool                    m_bOSRetrieved;
            OperatingSystem         m_eOSType;

            Link                    m_aListenerNotify;
            const OUString     m_aSharePointsNodeName;
            const OUString     m_aDirPropertyName;
            const OUString     m_aEnvPropertyName;
            const OUString     m_aLevelSep;
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
    PREDEFVAR_LANG,
    PREDEFVAR_LANGID,
    PREDEFVAR_VLANG,
    PREDEFVAR_INSTPATH,
    PREDEFVAR_PROGPATH,
    PREDEFVAR_USERPATH,
    PREDEFVAR_INSTURL,
    PREDEFVAR_PROGURL,
    PREDEFVAR_USERURL,
    PREDEFVAR_WORKDIRURL,
    // New variable of hierachy service (#i32656#)
    PREDEFVAR_BASEINSTURL,
    PREDEFVAR_USERDATAURL,
    PREDEFVAR_BRANDBASEURL,
    PREDEFVAR_SHARE_SUBDIR_NAME,
    PREDEFVAR_COUNT
};

struct PredefinedPathVariables
{
    // Predefined variables supported by substitute variables
    LanguageType    m_eLanguageType;                    // Lanuage type of Office
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

class SubstitutePathVariables : private ThreadHelpBase, // Struct for right initalization of mutex member! Must be first of baseclasses.
                                public ::cppu::WeakImplHelper2< ::com::sun::star::util::XStringSubstitution, css::lang::XServiceInfo >
{
    friend class SubstitutePathVariables_Impl;

    public:
        SubstitutePathVariables( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
        virtual ~SubstitutePathVariables();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XStringSubstitution
        virtual OUString SAL_CALL substituteVariables( const OUString& aText, sal_Bool bSubstRequired )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL reSubstituteVariables( const OUString& aText )
            throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getSubstituteVariableValue( const OUString& variable )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        protected:
            DECL_LINK(implts_ConfigurationNotify, void *);

            void            SetPredefinedPathVariables( PredefinedPathVariables& );
            OUString   ConvertOSLtoUCBURL( const OUString& aOSLCompliantURL ) const;

            // Special case (transient) values can change during runtime!
            // Don't store them in the pre defined struct
            OUString   GetWorkPath() const;
            OUString   GetWorkVariableValue() const;
            OUString   GetPathVariableValue() const;

            OUString   GetHomeVariableValue() const;

            // XStringSubstitution implementation methods
            OUString impl_substituteVariable( const OUString& aText, bool bSustRequired )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            OUString impl_reSubstituteVariables( const OUString& aText )
                throw (::com::sun::star::uno::RuntimeException);
            OUString impl_getSubstituteVariableValue( const OUString& variable )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        private:
            class VarNameToIndexMap : public boost::unordered_map< OUString,
                                                            PreDefVariable,
                                                            OUStringHash,
                                                            ::std::equal_to< OUString > >
            {
                inline void free()
                {
                    VarNameToIndexMap().swap( *this );
                }
            };

            // heavy used string
            const OUString          m_aVarStart;
            const OUString          m_aVarEnd;

            VarNameToIndexMap            m_aPreDefVarMap;         // Mapping from pre-def variable names to enum for array access
            SubstituteVariables          m_aSubstVarMap;          // Active rule set map indexed by variable name!
            PredefinedPathVariables      m_aPreDefVars;           // All predefined variables
            SubstitutePathVariables_Impl m_aImpl;                 // Implementation class that access the configuration
            ReSubstFixedVarOrderVector   m_aReSubstFixedVarOrder; // To speed up resubstitution fixed variables (order for lookup)
            ReSubstUserVarOrderVector    m_aReSubstUserVarOrder;  // To speed up resubstitution user variables
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
};

}

#endif // __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
