/*************************************************************************
 *
 *  $RCSfile: pathsettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:53 $
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

#ifndef __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
#define __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_

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

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
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

#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

namespace framework
{

struct PathInfo
{
    ::rtl::OUString sPath       ;
    ::rtl::OUString sValue      ;
    sal_Bool        bReadOnly   ;
    sal_Bool        bMultiPath  ;
};

/** implements the data container for the path settings service.
    It can be used to read/write the right config items; (re-)substitute
    her path values; check her readonly states and to provide an easy and fast
    access by using an ID instead of a name. */
class PathSettingsCfg : protected ThreadHelpBase
                      , public    utl::ConfigItem
{
    // ______________________________________
    // const

    protected:

        enum EPropHandle
        {
            E_ADDIN         = PATHSETTINGS_PROPHANDLE_ADDIN         ,
            E_AUTOCORRECT   = PATHSETTINGS_PROPHANDLE_AUTOCORRECT   ,
            E_AUTOTEXT      = PATHSETTINGS_PROPHANDLE_AUTOTEXT      ,
            E_BACKUP        = PATHSETTINGS_PROPHANDLE_BACKUP        ,
            E_BASIC         = PATHSETTINGS_PROPHANDLE_BASIC         ,
            E_BITMAP        = PATHSETTINGS_PROPHANDLE_BITMAP        ,
            E_CONFIG        = PATHSETTINGS_PROPHANDLE_CONFIG        ,
            E_DICTIONARY    = PATHSETTINGS_PROPHANDLE_DICTIONARY    ,
            E_FAVORITE      = PATHSETTINGS_PROPHANDLE_FAVORITE      ,
            E_FILTER        = PATHSETTINGS_PROPHANDLE_FILTER        ,
            E_GALLERY       = PATHSETTINGS_PROPHANDLE_GALLERY       ,
            E_GRAPHIC       = PATHSETTINGS_PROPHANDLE_GRAPHIC       ,
            E_HELP          = PATHSETTINGS_PROPHANDLE_HELP          ,
            E_LINGUISTIC    = PATHSETTINGS_PROPHANDLE_LINGUISTIC    ,
            E_MODULE        = PATHSETTINGS_PROPHANDLE_MODULE        ,
            E_PALETTE       = PATHSETTINGS_PROPHANDLE_PALETTE       ,
            E_PLUGIN        = PATHSETTINGS_PROPHANDLE_PLUGIN        ,
            E_STORAGE       = PATHSETTINGS_PROPHANDLE_STORAGE       ,
            E_TEMP          = PATHSETTINGS_PROPHANDLE_TEMP          ,
            E_TEMPLATE      = PATHSETTINGS_PROPHANDLE_TEMPLATE      ,
            E_UICONFIG      = PATHSETTINGS_PROPHANDLE_UICONFIG      ,
            E_USERCONFIG    = PATHSETTINGS_PROPHANDLE_USERCONFIG    ,
            E_USERDICTIONARY= PATHSETTINGS_PROPHANDLE_USERDICTIONARY,
            E_WORK          = PATHSETTINGS_PROPHANDLE_WORK
        };

        static const ::rtl::OUString PropNames[];
        static const css::beans::Property Properties[];

    // ______________________________________
    // member

    private:

        /** list of all supported path variables.
            Correspond to the defined lists of names/handles of static class PathSettingsPropHelp.
            see file "properties.h" for further informations. */
        PathInfo m_lPathes[PATHSETTINGS_PROPCOUNT];

        /** is used to map path names faster to her corresponding ID. */
        NameToHandleHash m_lIDMap;

        /** helper needed to (re-)substitute all internal save path values. */
        css::uno::Reference< css::util::XStringSubstitution > m_xSubstitution;

    // ______________________________________
    // interface

    public:

        /** initialize this instance and read all needed config values immediatly.
            The given uno manager is used to create own needed helper services. */
        PathSettingsCfg( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

        /** destroy this instance and free all used memory.
            Further it write all changed items back to the configguration, if it was not already done. */
        virtual ~PathSettingsCfg();

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes.
            We have to update the specified config items and actualize our cache. */
        virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& lPropertyNames );

        /** returns the path value for the given path handle.
            It returns an empty string for invalid calls. */
        ::rtl::OUString getPath( EPropHandle nID ) const;

        /** set the new path value for the given path handle.
            Invalid calls will be ignored and do nothing. */
        void setPath(       EPropHandle      nID    ,
                      const ::rtl::OUString& sValue );

        /** returns the readonly state for theiven path handle.
            It returns false as default and answer for invalid calls! */
        sal_Bool isReadOnly( EPropHandle nID ) const;

        /** some of our pathes are multi pathes.
            They contain a list of values instead of one string only.
            This method returns true, if the given handle match to a
            multi path (they are well known and fix!); false if not. */
        sal_Bool isMultiPath( EPropHandle nID ) const;

        /** returns a descriptor for all path properties.
            It contains against some fix informations some dynamic ones too (e.g. readonly states).
            But note: This method can return valid results only, if all neccessary data was readed
            before from the configuration! */
        const css::uno::Sequence< css::beans::Property > getPropertyDescriptor() const;

        /** map given path name to it's corresponding ID.
            But the out parameter is set to a valid enum value only in case
            mapping was successfully. If method returns false - rID will be undefined!
            Using of such "invalid" ID can produce crashes ... */
        sal_Bool mapName2Handle( const ::rtl::OUString& sName ,
                                       EPropHandle&     rID   ) const;

        /** it checks, if the given path value seams to be a valid URL or system path.
            To do it right it must be called with an information about the path type (single/multi path)! */
        sal_Bool isValidValue( const ::rtl::OUString& sValue     ,
                                     sal_Bool         bMultiPath ) const;

        /** it prepares the given path for saving.
            Doing so the path value must be checked and substituted.
            Further this method must know the path type (single/multi path) */
        sal_Bool checkAndSubstituteValue( ::rtl::OUString& sValue     ,
                                          sal_Bool         bMultiPath ) const;

    // ______________________________________
    // helper

    private:

        /** read the given list of path entries from the configuration and update
            all internal structures. And additional parameter "bSearchID" can be used
            to optimize this method and disable mapping of path names to her corresponding ID.
            If it's set to "FALSE" ... the method require that the array position of a path name
            entry inside "lNames" is equal to it's property handle! */
        void impl_read( const css::uno::Sequence< ::rtl::OUString >& lNames    ,
                              sal_Bool                               bSearchID );
};

class PathSettings : public  css::lang::XTypeProvider             ,
                     public  css::lang::XServiceInfo              ,
                     // base classes
                     // Order is neccessary for right initialization!
                     private PathSettingsCfg                      ,
                     public  ::cppu::OBroadcastHelper             ,
                     public  ::cppu::OPropertySetHelper           , // => XPropertySet / XFastPropertySet / XMultiPropertySet
                     public  ::cppu::OWeakObject                    // => XWeak, XInterface
{
    // ___________________________________________
    // member

    private:

        /** reference to factory, which has create this instance. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

    // ___________________________________________
    // interface

    public:

        /** initialize a new instance of this class.
            Attention: It's neccessary for right function of this class, that the order of base
            classes is the right one. Because we transfer information from one base to another
            during this ctor runs! */
        PathSettings( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

        /** free all used ressources ... if it was not already done. */
        virtual ~PathSettings();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

    // ___________________________________________
    // helper

    private:

        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        (       css::uno::Any&  aConvertedValue ,
                                                                                                                     css::uno::Any&  aOldValue       ,
                                                                                                                     sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw(css::lang::IllegalArgumentException);
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast(       sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw(css::uno::Exception);
        virtual void                                                SAL_CALL getFastPropertyValue            (       css::uno::Any&  aValue          ,
                                                                                                                     sal_Int32       nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                   (                                       );
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo              (                                       ) throw(::com::sun::star::uno::RuntimeException);

};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
