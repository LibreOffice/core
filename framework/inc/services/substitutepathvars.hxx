/*************************************************************************
 *
 *  $RCSfile: substitutepathvars.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:54 $
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

#ifndef __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_
#define __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <hash_map>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#include <classes/wildcard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

namespace framework
{

// Must be zero value based
enum EnvironmentType
{
    ET_HOST = 0     ,
    ET_YPDOMAIN     ,
    ET_DNSDOMAIN    ,
    ET_NTDOMAIN     ,
    ET_OS           ,
    ET_UNKNOWN      ,
    ET_COUNT
};

// Must be zero value based
enum OperatingSystem
{
    OS_WINDOWS = 0,
    OS_UNIX     ,
    OS_SOLARIS  ,
    OS_LINUX    ,
    OS_UNKNOWN  ,
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

    rtl::OUString               aSubstVariable;
    rtl::OUString               aSubstValue;
    com::sun::star::uno::Any    aEnvValue;
    EnvironmentType             aEnvType;
};

struct SubstitutePathNotify
{
    const com::sun::star::uno::Sequence<rtl::OUString> aPropertyNames;
};

class SubstituteVariables : public ::std::hash_map< ::rtl::OUString,
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

        void            GetSharePointsRules( SubstituteVariables& aSubstVarMap );

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. */
        virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    private:
        // Wrapper methods for low-level functions
        OperatingSystem         GetOperatingSystem();
        const rtl::OUString&    GetYPDomainName();
        const rtl::OUString&    GetDNSDomainName();
        const rtl::OUString&    GetNTDomainName();
        const rtl::OUString&    GetHostName();

        sal_Bool        FilterRuleSet( const SubstituteRuleVector& aRuleSet, SubstituteRule& aActiveRule );

        void            ReadSharePointsFromConfiguration( com::sun::star::uno::Sequence< rtl::OUString >& aSharePointsSeq );
        void            ReadSharePointRuleSetFromConfiguration( const rtl::OUString& aSharePointName,
                                                                const rtl::OUString& aSharePointNodeName,
                                                                SubstituteRuleVector& aRuleSet );

        // Stored values for domains and host
        sal_Bool            m_bYPDomainRetrieved;
        rtl::OUString       m_aYPDomain;
        sal_Bool            m_bDNSDomainRetrieved;
        rtl::OUString       m_aDNSDomain;
        sal_Bool            m_bNTDomainRetrieved;
        rtl::OUString       m_aNTDomain;
        sal_Bool            m_bHostRetrieved;
        rtl::OUString       m_aHost;
        sal_Bool            m_bOSRetrieved;
        OperatingSystem     m_eOSType;

        Link                m_aListenerNotify;
        const rtl::OUString m_aSharePointsNodeName;
        const rtl::OUString m_aDirPropertyName;
        const rtl::OUString m_aEnvPropertyName;
        const rtl::OUString m_aLevelSep;
};

enum PreDefVariable
{
    PREDEFVAR_INST,
    PREDEFVAR_PROG,
    PREDEFVAR_USER,
    PREDEFVAR_WORK,
    PREDEFVAR_HOME,
    PREDEFVAR_TEMP,
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

    bool ReSubstFixedVarOrder::operator< ( const ReSubstFixedVarOrder& aFixedVarOrder ) const
    {
        // Reverse operator< to have high to low ordering
        return ( nVarValueLength > aFixedVarOrder.nVarValueLength );
    }
};

struct ReSubstUserVarOrder
{
    sal_Int32       nVarValueLength;
    rtl::OUString   aVarName;

    bool ReSubstUserVarOrder::operator< ( const ReSubstUserVarOrder& aUserVarOrder ) const
    {
        // Reverse operator< to have high to low ordering
        return ( nVarValueLength > aUserVarOrder.nVarValueLength );
    }
};

typedef std::list< ReSubstFixedVarOrder > ReSubstFixedVarOrderVector;
typedef std::list< ReSubstUserVarOrder > ReSubstUserVarOrderVector;

class WorkPathHelper_Impl : public utl::ConfigItem
{
    public:
        WorkPathHelper_Impl();

        rtl::OUString GetWorkPath();

    private:
        com::sun::star::uno::Sequence< rtl::OUString > m_aPathNameSeq;
};

class SubstitutePathVariables :     public com::sun::star::lang::XTypeProvider      ,
                                    public com::sun::star::lang::XServiceInfo       ,
                                    public com::sun::star::util::XStringSubstitution,
                                    private ThreadHelpBase                          ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                    public ::cppu::OWeakObject
{
    friend class SubstitutePathVariables_Impl;

    public:
        SubstitutePathVariables( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~SubstitutePathVariables();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XStringSubstitution
        virtual rtl::OUString SAL_CALL substituteVariables( const ::rtl::OUString& aText, sal_Bool bSubstRequired )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual rtl::OUString SAL_CALL reSubstituteVariables( const ::rtl::OUString& aText )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSubstituteVariableValue( const ::rtl::OUString& variable )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    protected:
        DECL_LINK( implts_ConfigurationNotify, SubstitutePathNotify* );

        void            SetPredefinedPathVariables( PredefinedPathVariables& );
        rtl::OUString   ConvertOSLtoUCBURL( const rtl::OUString& aOSLCompliantURL ) const;

        // Special case (transient) values can change during runtime!
        // Don't store them in the pre defined struct
         rtl::OUString  GetWorkVariableValue() const;
         rtl::OUString  GetPathVariableValue() const;

        rtl::OUString   GetHomeVariableValue() const;
        rtl::OUString   GetLanguageString( LanguageType eLanguageType ) const;

        // XStringSubstitution implementation methods
        rtl::OUString impl_substituteVariable( const ::rtl::OUString& aText, sal_Bool bSustRequired )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        rtl::OUString impl_reSubstituteVariables( const ::rtl::OUString& aText )
            throw (::com::sun::star::uno::RuntimeException);
        ::rtl::OUString impl_getSubstituteVariableValue( const ::rtl::OUString& variable )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    private:
        class VarNameToIndexMap : public std::hash_map< ::rtl::OUString,
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
        const rtl::OUString             m_aVarStart;
        const rtl::OUString             m_aVarEnd;

        VarNameToIndexMap               m_aPreDefVarMap;            // Mapping from pre-def variable names to enum for array access
        SubstituteVariables             m_aSubstVarMap;             // Active rule set map indexed by variable name!
        PredefinedPathVariables         m_aPreDefVars;              // All predefined variables
        SubstitutePathVariables_Impl    m_aImpl;                    // Implementation class that access the configuration
        ReSubstFixedVarOrderVector      m_aReSubstFixedVarOrder;    // To speed up resubstitution fixed variables (order for lookup)
        ReSubstUserVarOrderVector       m_aReSubstUserVarOrder;     // To speed up resubstitution user variables
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
        WorkPathHelper_Impl             m_aWorkPathHelper;          // Helper class to get a substitution for $(workdirurl)
};

}

#endif // __FRAMEWORK_SERVICES_SUBSTPATHVARS_HXX_
