/*************************************************************************
 *
 *  $RCSfile: pathsettings.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2002-08-20 10:10:29 $
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

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif


namespace framework
{

struct PathSettingsNotify
{
    com::sun::star::uno::Sequence< rtl::OUString > aPathSettingsChanged;
};

struct PathSettingsData
{
    // ID's for the supported pathes. Must be sorted by name!!
    enum PathSettingsId
    {
        PS_ADDINPATH,
        PS_AUTOCORRECTPATH,
        PS_AUTOTEXTPATH,
        PS_BACKUPPATH,
        PS_BASICPATH,
        PS_BITMAPPATH,
        PS_CONFIGPATH,
        PS_DICTIONARYPATH,
        PS_FAVORITESPATH,
        PS_FILTERPATH,
        PS_GALLERYPATH,
        PS_GRAPHICPATH,
        PS_HELPPATH,
        PS_LINGUISTICPATH,
        PS_MODULEPATH,
        PS_PALETTEPATH,
        PS_PLUGINPATH,
        PS_STORAGEPATH,
        PS_TEMPPATH,
        PS_TEMPLATEPATH,
        PS_UICONFIGPATH,
        PS_USERCONFIGPATH,
        PS_USERDICTIONARYPATH,
        PS_WORKPATH,
        PS_COUNT
    };

    void SetValue( PathSettingsId nId, rtl::OUString& aValue );
    const rtl::OUString& GetValue( PathSettingsId nId ) const;

    // Strings for all supported pathes
    ::rtl::OUString         m_aAddinPath;
    ::rtl::OUString         m_aAutoCorrectPath;
    ::rtl::OUString         m_aAutoTextPath;
    ::rtl::OUString         m_aBackupPath;
    ::rtl::OUString         m_aBasicPath;
    ::rtl::OUString         m_aBitmapPath;
    ::rtl::OUString         m_aConfigPath;
    ::rtl::OUString         m_aDictionaryPath;
    ::rtl::OUString         m_aFavoritesPath;
    ::rtl::OUString         m_aFilterPath;
    ::rtl::OUString         m_aGalleryPath;
    ::rtl::OUString         m_aGraphicPath;
    ::rtl::OUString         m_aHelpPath;
    ::rtl::OUString         m_aLinguisticPath;
    ::rtl::OUString         m_aModulePath;
    ::rtl::OUString         m_aPalettePath;
    ::rtl::OUString         m_aPluginPath;
    ::rtl::OUString         m_aStoragePath;
    ::rtl::OUString         m_aTempPath;
    ::rtl::OUString         m_aTemplatePath;
    ::rtl::OUString         m_aUIConfigPath;
    ::rtl::OUString         m_aUserConfigPath;
    ::rtl::OUString         m_aUserDictionaryPath;
    ::rtl::OUString         m_aWorkPath;

    ::rtl::OUString         m_aEmptyStr;
};

class PathSettings;
class PathSettings_Impl : public utl::ConfigItem
{
    friend class PathSettings;

    public:
        PathSettings_Impl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xSMgr, const Link& aNotifyLink );
        virtual ~PathSettings_Impl();

        /** is called from the ConfigManager before application ends or from the
            PropertyChangeListener if the sub tree broadcasts changes. */
        virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

        sal_Bool        ReadConfigurationData( PathSettingsData& rSettingsData, const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        sal_Bool        WriteConfigurationData( const PathSettingsData& rSettingsData, const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        sal_Bool        CheckAndReplaceNewPathValue( PathSettingsData::PathSettingsId nId, rtl::OUString& aNewPathValue );

        int             GetHandleFromPropertyName( const rtl::OUString& aPropertyName );

    private:
        sal_Bool        CheckPath( PathSettingsData::PathSettingsId nId, const rtl::OUString& aNewPathValue );

        class PropToHandleHashMap : public ::std::hash_map< ::rtl::OUString,
                                                            PathSettingsData::PathSettingsId,
                                                            OUStringHashCode,
                                                            ::std::equal_to< ::rtl::OUString > >
        {
            public:
                inline void free()
                {
                    PropToHandleHashMap().swap( *this );
                }
        };

        Link                                                                            m_aListenerNotify ;
        PropToHandleHashMap                                                             m_aPropNameToHandleMap ;
        com::sun::star::uno::Reference< com::sun::star::util::XStringSubstitution >     m_xPathVariableSubstitution ;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    m_xFactory ;
};


class PathSettings  :   // interfaces
                    public  css::lang::XTypeProvider             ,
                    public  css::lang::XServiceInfo              ,
                    // base classes
                    // Order is neccessary for right initialization!
                    private ThreadHelpBase                       ,
                    public  ::cppu::OBroadcastHelper             ,
                    public  ::cppu::OPropertySetHelper           ,
                    public  ::cppu::OWeakObject
{
    friend class PathSettings_Impl;

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
                 PathSettings( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~PathSettings(                                                                        );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO


    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        (       css::uno::Any&  aConvertedValue ,
                                                                                                                     css::uno::Any&  aOldValue       ,
                                                                                                                     sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          )
            throw( css::lang::IllegalArgumentException );

        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast(       sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          )
            throw( css::uno::Exception                 );

        virtual void                                                SAL_CALL getFastPropertyValue            (       css::uno::Any&  aValue          ,
                                                                                                                     sal_Int32       nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                   (                                       );

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
            throw (::com::sun::star::uno::RuntimeException);

        DECL_LINK( implts_ConfigurationNotify, PathSettingsNotify* );


    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        sal_Bool                                                impl_tryToChangeProperty        (       rtl::OUString   sCurrentValue   ,
                                                                                                  const css::uno::Any&  aNewValue       ,
                                                                                                        css::uno::Any&  aOldValue       ,
                                                                                                        css::uno::Any& aConvertedValue )
            throw( css::lang::IllegalArgumentException  );

        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        // Members
        css::uno::Reference< css::lang::XMultiServiceFactory >                      m_xFactory                  ;   /// reference to factory, which has create this instance
        PathSettings_Impl                                                           m_aImpl                     ;
        PathSettingsData                                                            m_aPathSettingsData         ;
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
