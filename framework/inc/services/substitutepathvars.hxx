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
#include <i18npool/lang.h>

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
    SubstituteRule( const rtl::OUString& aVarName,
                    const rtl::OUString& aValue,
                    const com::sun::star::uno::Any& aVal,
                    EnvironmentType aType ) :
        aSubstVariable( aVarName ), aSubstValue( aValue ), aEnvValue( aVal ), aEnvType( aType ) {}

    rtl::OUString            aSubstVariable;
    rtl::OUString            aSubstValue;
    com::sun::star::uno::Any aEnvValue;
    EnvironmentType          aEnvType;
};

struct SubstitutePathNotify
{
    SubstitutePathNotify() {};

    const com::sun::star::uno::Sequence<rtl::OUString> aPropertyNames;
};

class SubstituteVariables : public ::boost::unordered_map< ::rtl::OUString,
                                                    SubstituteRule,
                                                    OUStringHashCode,
                                                    ::std::equal_to< ::rtl::OUString > >
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

        static OperatingSystem GetOperatingSystemFromString( const rtl::OUString& );
        static EnvironmentType GetEnvTypeFromString( const rtl::OUString& );

        void                   GetSharePointsRules( SubstituteVariables& aSubstVarMap );

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. */
        virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        virtual void Commit();

        private:
            // Wrapper methods for low-level functions
            OperatingSystem         GetOperatingSystem();
            const rtl::OUString&    GetYPDomainName();
            const rtl::OUString&    GetDNSDomainName();
            const rtl::OUString&    GetNTDomainName();
            const rtl::OUString&    GetHostName();

            bool                    FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule );

            void                    ReadSharePointsFromConfiguration( com::sun::star::uno::Sequence< rtl::OUString >& aSharePointsSeq );
            void                    ReadSharePointRuleSetFromConfiguration( const rtl::OUString& aSharePointName,
                                                                                                                                const rtl::OUString& aSharePointNodeName,
                                                                                                                                SubstituteRuleVector& aRuleSet );

            // Stored values for domains and host
            bool                    m_bYPDomainRetrieved;
            rtl::OUString           m_aYPDomain;
            bool                    m_bDNSDomainRetrieved;
            rtl::OUString           m_aDNSDomain;
            bool                    m_bNTDomainRetrieved;
            rtl::OUString           m_aNTDomain;
            bool                    m_bHostRetrieved;
            rtl::OUString           m_aHost;
            bool                    m_bOSRetrieved;
            OperatingSystem         m_eOSType;

            Link                    m_aListenerNotify;
            const rtl::OUString     m_aSharePointsNodeName;
            const rtl::OUString     m_aDirPropertyName;
            const rtl::OUString     m_aEnvPropertyName;
            const rtl::OUString     m_aLevelSep;
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
    PREDEFVAR_COUNT
};

struct PredefinedPathVariables
{
    // Predefined variables supported by substitute variables
    LanguageType    m_eLanguageType;                    // Lanuage type of Office
    rtl::OUString   m_FixedVar[ PREDEFVAR_COUNT ];      // Variable value access by PreDefVariable
    rtl::OUString   m_FixedVarNames[ PREDEFVAR_COUNT ]; // Variable name access by PreDefVariable
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
    rtl::OUString   aVarName;

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
        SubstitutePathVariables( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~SubstitutePathVariables();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XStringSubstitution
        virtual rtl::OUString SAL_CALL substituteVariables( const ::rtl::OUString& aText, sal_Bool bSubstRequired )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual rtl::OUString SAL_CALL reSubstituteVariables( const ::rtl::OUString& aText )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSubstituteVariableValue( const ::rtl::OUString& variable )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        protected:
            DECL_LINK(implts_ConfigurationNotify, void *);

            void            SetPredefinedPathVariables( PredefinedPathVariables& );
            rtl::OUString   ConvertOSLtoUCBURL( const rtl::OUString& aOSLCompliantURL ) const;

            // Special case (transient) values can change during runtime!
            // Don't store them in the pre defined struct
            rtl::OUString   GetWorkPath() const;
            rtl::OUString   GetWorkVariableValue() const;
            rtl::OUString   GetPathVariableValue() const;

            rtl::OUString   GetHomeVariableValue() const;

            // XStringSubstitution implementation methods
            rtl::OUString impl_substituteVariable( const ::rtl::OUString& aText, bool bSustRequired )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            rtl::OUString impl_reSubstituteVariables( const ::rtl::OUString& aText )
                throw (::com::sun::star::uno::RuntimeException);
            ::rtl::OUString impl_getSubstituteVariableValue( const ::rtl::OUString& variable )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        private:
            class VarNameToIndexMap : public boost::unordered_map< ::rtl::OUString,
                                                            PreDefVariable,
                                                            OUStringHashCode,
                                                            ::std::equal_to< ::rtl::OUString > >
            {
                inline void free()
                {
                    VarNameToIndexMap().swap( *this );
                }
            };

            // heavy used string
            const rtl::OUString          m_aVarStart;
            const rtl::OUString          m_aVarEnd;

            VarNameToIndexMap            m_aPreDefVarMap;         // Mapping from pre-def variable names to enum for array access
            SubstituteVariables          m_aSubstVarMap;          // Active rule set map indexed by variable name!
            PredefinedPathVariables      m_aPreDefVars;           // All predefined variables
            SubstitutePathVariables_Impl m_aImpl;                 // Implementation class that access the configuration
            ReSubstFixedVarOrderVector   m_aReSubstFixedVarOrder; // To speed up resubstitution fixed variables (order for lookup)
            ReSubstUserVarOrderVector    m_aReSubstUserVarOrder;  // To speed up resubstitution user variables
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
};

}

#endif // __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
