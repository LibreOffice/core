/*************************************************************************
 *
 *  $RCSfile: pathsettings.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2002-08-20 10:13:34 $
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

#ifndef __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
#include <services/pathsettings.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#include <cppuhelper/proptypehlp.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <vector>

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

// ascii strings of the properties
#define STRING_ASCII_ADDIN              "Addin"
#define STRING_ASCII_AUTOCORRECT        "AutoCorrect"
#define STRING_ASCII_AUTOTEXT           "AutoText"
#define STRING_ASCII_BACKUP             "Backup"
#define STRING_ASCII_BASIC              "Basic"
#define STRING_ASCII_BITMAP             "Bitmap"
#define STRING_ASCII_CONFIG             "Config"
#define STRING_ASCII_DICTIONARY         "Dictionary"
#define STRING_ASCII_FAVORITES          "Favorite"
#define STRING_ASCII_FILTER             "Filter"
#define STRING_ASCII_GALLERY            "Gallery"
#define STRING_ASCII_GRAPHIC            "Graphic"
#define STRING_ASCII_HELP               "Help"
#define STRING_ASCII_LINGUISTIC         "Linguistic"
#define STRING_ASCII_MODULE             "Module"
#define STRING_ASCII_PALETTE            "Palette"
#define STRING_ASCII_PLUGIN             "Plugin"
#define STRING_ASCII_STORAGE            "Storage"
#define STRING_ASCII_TEMP               "Temp"
#define STRING_ASCII_TEMPLATE           "Template"
#define STRING_ASCII_UICONFIG           "UIConfig"
#define STRING_ASCII_USERCONFIG         "UserConfig"
#define STRING_ASCII_USERDICTIONARY     "UserDictionary"
#define STRING_ASCII_WORK               "Work"

// name of properties
#define PROPERTYNAME_ADDIN                      DECLARE_ASCII(STRING_ASCII_ADDIN)
#define PROPERTYNAME_AUTOCORRECT                DECLARE_ASCII(STRING_ASCII_AUTOCORRECT)
#define PROPERTYNAME_AUTOTEXT                   DECLARE_ASCII(STRING_ASCII_AUTOTEXT)
#define PROPERTYNAME_BACKUP                     DECLARE_ASCII(STRING_ASCII_BACKUP)
#define PROPERTYNAME_BASIC                      DECLARE_ASCII(STRING_ASCII_BASIC)
#define PROPERTYNAME_BITMAP                     DECLARE_ASCII(STRING_ASCII_BITMAP)
#define PROPERTYNAME_CONFIG                     DECLARE_ASCII(STRING_ASCII_CONFIG)
#define PROPERTYNAME_DICTIONARY                 DECLARE_ASCII(STRING_ASCII_DICTIONARY)
#define PROPERTYNAME_FAVORITES                  DECLARE_ASCII(STRING_ASCII_FAVORITES)
#define PROPERTYNAME_FILTER                     DECLARE_ASCII(STRING_ASCII_FILTER)
#define PROPERTYNAME_GALLERY                    DECLARE_ASCII(STRING_ASCII_GALLERY)
#define PROPERTYNAME_GRAPHIC                    DECLARE_ASCII(STRING_ASCII_GRAPHIC)
#define PROPERTYNAME_HELP                       DECLARE_ASCII(STRING_ASCII_HELP)
#define PROPERTYNAME_LINGUISTIC                 DECLARE_ASCII(STRING_ASCII_LINGUISTIC)
#define PROPERTYNAME_MODULE                     DECLARE_ASCII(STRING_ASCII_MODULE)
#define PROPERTYNAME_PALETTE                    DECLARE_ASCII(STRING_ASCII_PALETTE)
#define PROPERTYNAME_PLUGIN                     DECLARE_ASCII(STRING_ASCII_PLUGIN)
#define PROPERTYNAME_STORAGE                    DECLARE_ASCII(STRING_ASCII_STORAGE)
#define PROPERTYNAME_TEMP                       DECLARE_ASCII(STRING_ASCII_TEMP)
#define PROPERTYNAME_TEMPLATE                   DECLARE_ASCII(STRING_ASCII_TEMPLATE)
#define PROPERTYNAME_UICONFIG                   DECLARE_ASCII(STRING_ASCII_UICONFIG)
#define PROPERTYNAME_USERCONFIG                 DECLARE_ASCII(STRING_ASCII_USERCONFIG)
#define PROPERTYNAME_USERDICTIONARY             DECLARE_ASCII(STRING_ASCII_USERDICTIONARY)
#define PROPERTYNAME_WORK                       DECLARE_ASCII(STRING_ASCII_WORK)


// handle of properties
#define PROPERTYHANDLE_ADDIN                    PathSettingsData::PS_ADDINPATH
#define PROPERTYHANDLE_AUTOCORRECT              PathSettingsData::PS_AUTOCORRECTPATH
#define PROPERTYHANDLE_AUTOTEXT                 PathSettingsData::PS_AUTOTEXTPATH
#define PROPERTYHANDLE_BACKUP                   PathSettingsData::PS_BACKUPPATH
#define PROPERTYHANDLE_BASIC                    PathSettingsData::PS_BASICPATH
#define PROPERTYHANDLE_BITMAP                   PathSettingsData::PS_BITMAPPATH
#define PROPERTYHANDLE_CONFIG                   PathSettingsData::PS_CONFIGPATH
#define PROPERTYHANDLE_DICTIONARY               PathSettingsData::PS_DICTIONARYPATH
#define PROPERTYHANDLE_FAVORITES                PathSettingsData::PS_FAVORITESPATH
#define PROPERTYHANDLE_FILTER                   PathSettingsData::PS_FILTERPATH
#define PROPERTYHANDLE_GALLERY                  PathSettingsData::PS_GALLERYPATH
#define PROPERTYHANDLE_GRAPHIC                  PathSettingsData::PS_GRAPHICPATH
#define PROPERTYHANDLE_HELP                     PathSettingsData::PS_HELPPATH
#define PROPERTYHANDLE_LINGUISTIC               PathSettingsData::PS_LINGUISTICPATH
#define PROPERTYHANDLE_MODULE                   PathSettingsData::PS_MODULEPATH
#define PROPERTYHANDLE_PALETTE                  PathSettingsData::PS_PALETTEPATH
#define PROPERTYHANDLE_PLUGIN                   PathSettingsData::PS_PLUGINPATH
#define PROPERTYHANDLE_STORAGE                  PathSettingsData::PS_STORAGEPATH
#define PROPERTYHANDLE_TEMP                     PathSettingsData::PS_TEMPPATH
#define PROPERTYHANDLE_TEMPLATE                 PathSettingsData::PS_TEMPLATEPATH
#define PROPERTYHANDLE_UICONFIG                 PathSettingsData::PS_UICONFIGPATH
#define PROPERTYHANDLE_USERCONFIG               PathSettingsData::PS_USERCONFIGPATH
#define PROPERTYHANDLE_USERDICTIONARY           PathSettingsData::PS_USERDICTIONARYPATH
#define PROPERTYHANDLE_WORK                     PathSettingsData::PS_WORKPATH

// count of ALL properties
#define PROPERTYCOUNT                           PathSettingsData::PS_COUNT

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

static const char* aPropNames[] =
{
    STRING_ASCII_ADDIN,             // PS_ADDIN
    STRING_ASCII_AUTOCORRECT,       // PS_AUTOCORRECT
    STRING_ASCII_AUTOTEXT,          // PS_AUTOTEXT
    STRING_ASCII_BACKUP,            // PS_BACKUP
    STRING_ASCII_BASIC,             // PS_BASIC
    STRING_ASCII_BITMAP,            // PS_BITMAP
    STRING_ASCII_CONFIG,            // PS_CONFIG
    STRING_ASCII_DICTIONARY,        // PS_DICTIONARY
    STRING_ASCII_FAVORITES,         // PS_FAVORITES
    STRING_ASCII_FILTER,            // PS_FILTER
    STRING_ASCII_GALLERY,           // PS_GALLERY
    STRING_ASCII_GRAPHIC,           // PS_GRAPHIC
    STRING_ASCII_HELP,              // PS_HELP
    STRING_ASCII_LINGUISTIC,        // PS_LINGUISTIC
    STRING_ASCII_MODULE,            // PS_MODULE
    STRING_ASCII_PALETTE,           // PS_PALETTE
    STRING_ASCII_PLUGIN,            // PS_PLUGIN
    STRING_ASCII_STORAGE,           // PS_STORAGE
    STRING_ASCII_TEMP,              // PS_TEMP
    STRING_ASCII_TEMPLATE,          // PS_TEMPLATE
    STRING_ASCII_UICONFIG,          // PS_USERCONFIG
    STRING_ASCII_USERCONFIG,        // PS_USERDICTIONARY
    STRING_ASCII_USERDICTIONARY,    // PS_UICONFIG
    STRING_ASCII_WORK               // PS_WORK
};

Sequence< rtl::OUString > GetPathPropertyNames()
{
    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< rtl::OUString > aNames( nCount );
    rtl::OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = rtl::OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

//_________________________________________________________________________________________________________________
//  Implementation helper class
//_________________________________________________________________________________________________________________
//

void PathSettingsData::SetValue( PathSettingsId nId, rtl::OUString& aValue )
{
    switch ( nId )
    {
        case PathSettingsData::PS_ADDINPATH:            m_aAddinPath = aValue;          break;
        case PathSettingsData::PS_AUTOCORRECTPATH:      m_aAutoCorrectPath = aValue;    break;
        case PathSettingsData::PS_AUTOTEXTPATH:         m_aAutoTextPath = aValue;       break;
        case PathSettingsData::PS_BACKUPPATH:           m_aBackupPath = aValue;         break;
        case PathSettingsData::PS_BASICPATH:            m_aBasicPath = aValue;          break;
        case PathSettingsData::PS_BITMAPPATH:           m_aBitmapPath = aValue;         break;
        case PathSettingsData::PS_CONFIGPATH:           m_aConfigPath = aValue;         break;
        case PathSettingsData::PS_DICTIONARYPATH:       m_aDictionaryPath = aValue;     break;
        case PathSettingsData::PS_FAVORITESPATH:        m_aFavoritesPath = aValue;      break;
        case PathSettingsData::PS_FILTERPATH:           m_aFilterPath = aValue;         break;
        case PathSettingsData::PS_GALLERYPATH:          m_aGalleryPath = aValue;        break;
        case PathSettingsData::PS_GRAPHICPATH:          m_aGraphicPath = aValue;        break;
        case PathSettingsData::PS_HELPPATH:             m_aHelpPath = aValue;           break;
        case PathSettingsData::PS_LINGUISTICPATH:       m_aLinguisticPath = aValue;     break;
        case PathSettingsData::PS_MODULEPATH:           m_aModulePath = aValue;         break;
        case PathSettingsData::PS_PALETTEPATH:          m_aPalettePath = aValue;        break;
        case PathSettingsData::PS_PLUGINPATH:           m_aPluginPath = aValue;         break;
        case PathSettingsData::PS_STORAGEPATH:          m_aStoragePath = aValue;        break;
        case PathSettingsData::PS_TEMPPATH:             m_aTempPath = aValue;           break;
        case PathSettingsData::PS_TEMPLATEPATH:         m_aTemplatePath = aValue;       break;
        case PathSettingsData::PS_USERCONFIGPATH:       m_aUserConfigPath = aValue;     break;
        case PathSettingsData::PS_USERDICTIONARYPATH:   m_aUserDictionaryPath = aValue; break;
        case PathSettingsData::PS_WORKPATH:             m_aWorkPath = aValue;           break;
        case PathSettingsData::PS_UICONFIGPATH:         m_aUIConfigPath = aValue;       break;

        default:
            DBG_ERRORFILE( "invalid index to load a path" );
    }
}

const rtl::OUString& PathSettingsData::GetValue( PathSettingsId nId ) const
{
    switch ( nId )
    {
        case PathSettingsData::PS_ADDINPATH:            return m_aAddinPath;
        case PathSettingsData::PS_AUTOCORRECTPATH:      return m_aAutoCorrectPath;
        case PathSettingsData::PS_AUTOTEXTPATH:         return m_aAutoTextPath;
        case PathSettingsData::PS_BACKUPPATH:           return m_aBackupPath;
        case PathSettingsData::PS_BASICPATH:            return m_aBasicPath;
        case PathSettingsData::PS_BITMAPPATH:           return m_aBitmapPath;
        case PathSettingsData::PS_CONFIGPATH:           return m_aConfigPath;
        case PathSettingsData::PS_DICTIONARYPATH:       return m_aDictionaryPath;
        case PathSettingsData::PS_FAVORITESPATH:        return m_aFavoritesPath;
        case PathSettingsData::PS_FILTERPATH:           return m_aFilterPath;
        case PathSettingsData::PS_GALLERYPATH:          return m_aGalleryPath;
        case PathSettingsData::PS_GRAPHICPATH:          return m_aGraphicPath;
        case PathSettingsData::PS_HELPPATH:             return m_aHelpPath;
        case PathSettingsData::PS_LINGUISTICPATH:       return m_aLinguisticPath;
        case PathSettingsData::PS_MODULEPATH:           return m_aModulePath;
        case PathSettingsData::PS_PALETTEPATH:          return m_aPalettePath;
        case PathSettingsData::PS_PLUGINPATH:           return m_aPluginPath;
        case PathSettingsData::PS_STORAGEPATH:          return m_aStoragePath;
        case PathSettingsData::PS_TEMPPATH:             return m_aTempPath;
        case PathSettingsData::PS_TEMPLATEPATH:         return m_aTemplatePath;
        case PathSettingsData::PS_USERCONFIGPATH:       return m_aUserConfigPath;
        case PathSettingsData::PS_USERDICTIONARYPATH:   return m_aUserDictionaryPath;
        case PathSettingsData::PS_WORKPATH:             return m_aWorkPath;
        case PathSettingsData::PS_UICONFIGPATH:         return m_aUIConfigPath;

        default:
            DBG_ERRORFILE( "invalid index to load a path" );
            return m_aEmptyStr;
    }
}


PathSettings_Impl::PathSettings_Impl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xSMgr, const Link& aNotifyLink ) :
    utl::ConfigItem( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Path/Current" ))),
    m_aListenerNotify( aNotifyLink ),
    m_xFactory( xSMgr )
{
    m_xPathVariableSubstitution = Reference< XStringSubstitution >( m_xFactory->createInstance(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.PathSubstitution" ))), UNO_QUERY );

    // Create a hash map to have a quick way to find the handle of a property name!
    for ( sal_Int32 n = 0; n < PROPERTYCOUNT; n++ )
        m_aPropNameToHandleMap.insert( PropToHandleHashMap::value_type(
            rtl::OUString::createFromAscii( aPropNames[n] ), (PathSettingsData::PathSettingsId)n ) );
}

PathSettings_Impl::~PathSettings_Impl()
{
}

int PathSettings_Impl::GetHandleFromPropertyName( const rtl::OUString& aPropertyName )
{

    PropToHandleHashMap::const_iterator pIter = m_aPropNameToHandleMap.find( aPropertyName );
    if ( pIter != m_aPropNameToHandleMap.end() )
        return pIter->second;
    else
        return -1;
}

sal_Bool PathSettings_Impl::ReadConfigurationData( PathSettingsData& rSettingsData, const Sequence< rtl::OUString >& aNames )
{
    Sequence< Any >         aValues     = GetProperties( aNames );
    const rtl::OUString*    pNames      = aNames.getConstArray();
    const Any*              pValues     = aValues.getConstArray();

    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        rtl::OUString aTempStr, aFullPath;

        for ( int n = 0; n < aNames.getLength(); n++ )
        {
            if ( pValues[n].hasValue() )
            {
                switch ( pValues[n].getValueTypeClass() )
                {
                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        // single pathes
                        if ( pValues[n] >>= aTempStr )
                            aFullPath = m_xPathVariableSubstitution->substituteVariables( aTempStr, sal_False );
                        else
                        {
                            DBG_ERRORFILE( "any operator >>= failed" );
                        }
                        break;
                    }

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    {
                        // multi pathes
                        aFullPath = rtl::OUString();
                        Sequence < rtl::OUString > aList;
                        if ( pValues[n] >>= aList )
                        {
                            sal_Int32 nCount = aList.getLength();
                            for ( sal_Int32 nPosition = 0; nPosition < nCount; ++nPosition )
                            {
                                aTempStr = m_xPathVariableSubstitution->substituteVariables( aList[ nPosition ], sal_False  );
                                aFullPath += aTempStr;
                                if ( nPosition < nCount-1 )
                                    aFullPath += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(";") );
                            }
                        }
                        else
                        {
                            DBG_ERROR( "Unknown property name used!!" );
                        }
                        break;
                    }

                    default:
                    {
                        DBG_ERROR( "Unknown property name used!!" );
                    }
                }

                // Get property handle for the property name and set value
                int nProp = GetHandleFromPropertyName( pNames[n] );
                if ( nProp >= 0 )
                    rSettingsData.SetValue( (PathSettingsData::PathSettingsId)nProp, aFullPath );
                else
                    DBG_ERROR( "Unknown property name used!!" );
            }
        }
    }

    return sal_True;
}

sal_Bool PathSettings_Impl::WriteConfigurationData( const PathSettingsData& rSettingsData, const Sequence< rtl::OUString >& aNames )
{
    rtl::OUString           aTempStr;
    const rtl::OUString*    pNames  = aNames.getConstArray();
    Sequence< Any >         aValues( aNames.getLength() );
    Any*                    pValues = aValues.getArray();
    const Type&             rType   = ::getBooleanCppuType();

    for ( int n = 0; n < aNames.getLength(); n++ )
    {
        sal_Bool bList = sal_False;

        // Get property handle for the property name
        int nProp = GetHandleFromPropertyName( pNames[n] );
        if ( nProp >= 0 )
        {
            switch ( nProp )
            {
                // multi pathes
                case PathSettingsData::PS_AUTOCORRECTPATH:
                case PathSettingsData::PS_AUTOTEXTPATH:
                case PathSettingsData::PS_BASICPATH:
                case PathSettingsData::PS_GALLERYPATH:
                case PathSettingsData::PS_PLUGINPATH:
                case PathSettingsData::PS_TEMPLATEPATH:
                case PathSettingsData::PS_UICONFIGPATH:
                    bList = sal_True;
            }

            aTempStr = rSettingsData.GetValue( (PathSettingsData::PathSettingsId)nProp );

            if ( bList )
            {
                String aFullPath( aTempStr );
                USHORT nCount = aFullPath.GetTokenCount(), nIdx = 0;
                if ( nCount > 0  )
                {
                    sal_Int32 nPos = 0;
                    Sequence < rtl::OUString > aList( nCount );
                    while ( STRING_NOTFOUND != nIdx )
                        aList[nPos++] = m_xPathVariableSubstitution->reSubstituteVariables( aFullPath.GetToken( 0, ';', nIdx ) );
                    pValues[n] <<= aList;
                }
            }
            else
            {
                pValues[n] <<= m_xPathVariableSubstitution->reSubstituteVariables( aTempStr );
            }
        }
        else
        {
            DBG_ERROR( "Unknown property name used!!" );
            return sal_False;
        }
    }

    PutProperties( aNames, aValues );
    return sal_True;
}

sal_Bool PathSettings_Impl::CheckPath( PathSettingsData::PathSettingsId nId, const rtl::OUString& aNewPathValue )
{
    // Check the new path value
    sal_Bool bList      = sal_False;
    sal_Bool bValueOk   = sal_False;

    switch ( nId )
    {
        // multi pathes
        case PathSettingsData::PS_AUTOCORRECTPATH:
        case PathSettingsData::PS_AUTOTEXTPATH:
        case PathSettingsData::PS_BASICPATH:
        case PathSettingsData::PS_GALLERYPATH:
        case PathSettingsData::PS_PLUGINPATH:
        case PathSettingsData::PS_TEMPLATEPATH:
        case PathSettingsData::PS_UICONFIGPATH:
            bList = sal_True;
    }

    if ( bList )
    {
        String aFullPath( aNewPathValue );
        USHORT nCount = aFullPath.GetTokenCount(), nIdx = 0;
        if ( nCount > 0  )
        {
            bValueOk = sal_True;
            while ( STRING_NOTFOUND != nIdx && bValueOk )
                bValueOk = !( INetURLObject( aFullPath.GetToken( 0, ';', nIdx ) ).HasError() );
        }
    }
    else
        bValueOk = !( INetURLObject( aNewPathValue ).HasError() );

    return bValueOk;
}

// Checks if we have a valid URL otherwise it tries to substitute the value to support variables setting for a path setting!
sal_Bool PathSettings_Impl::CheckAndReplaceNewPathValue( PathSettingsData::PathSettingsId nId, rtl::OUString& aNewPathValue )
{
    sal_Bool bValueOk = CheckPath( nId, aNewPathValue );

    if ( !bValueOk )
    {
        rtl::OUString aResult;

        // Support the usage of path variables set through setPropertyValue
        try
        {
            aResult = m_xPathVariableSubstitution->substituteVariables( aNewPathValue, sal_True );
        }
        catch( com::sun::star::container::NoSuchElementException& )
        {
            // Substitution doesn't work => don't try further and give up
            bValueOk = sal_False;
        }

        if ( bValueOk )
        {
            bValueOk = CheckPath( nId, aResult );
            if ( bValueOk )
                aNewPathValue = aResult;
        }
    }

    return bValueOk;
}

void PathSettings_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames )
{
    if ( m_aListenerNotify.IsSet() )
        m_aListenerNotify.Call( this );
}


//_________________________________________________________________________________________________________________
//  Implementation service class
//_________________________________________________________________________________________________________________
//

DEFINE_XINTERFACE_5                     (   PathSettings                                             ,
                                            OWeakObject                                              ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider              ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo               ),
                                            DIRECT_INTERFACE( css::beans::XPropertySet              ),
                                            DIRECT_INTERFACE( css::beans::XFastPropertySet          ),
                                            DIRECT_INTERFACE( css::beans::XMultiPropertySet         )
                                        )

DEFINE_XTYPEPROVIDER_5                  (   PathSettings                                            ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::beans::XPropertySet                                ,
                                            css::beans::XFastPropertySet                            ,
                                            css::beans::XMultiPropertySet
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   PathSettings                                            ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_PATHSETTINGS                                ,
                                            IMPLEMENTATIONNAME_PATHSETTINGS
                                        )

DEFINE_INIT_SERVICE                     (   PathSettings,
                                            {
                                                /*Attention
                                                    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                                                    to create a new instance of this class by our own supported service factory.
                                                    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                                                */
                                            }
                                        )

/*-************************************************************************************************************//**
    @short      standard constructor to create instance by factory
    @descr      This constructor initialize a new instance of this class by valid factory,
                and will be set valid values on his member and baseclasses.

    @attention  a)  Don't use your own reference during an UNO-Service-ctor! There is no guarantee, that you
                    will get over this. (e.g. using of your reference as parameter to initialize some member)
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automaticly after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBrodcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      "xFactory" is the multi service manager, which create this instance.
                The value must be different from NULL!
    @return     -

    @onerror    We throw an ASSERT in debug version or do nothing in relaese version.
*//*-*************************************************************************************************************/
PathSettings::PathSettings( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention: Don't change order of initialization!
        //      ThreadHelpBase is a struct with a lock as member. We can't use a lock as direct member!
        //      We must garant right initialization and a valid value of this to initialize other baseclasses!
        :   ThreadHelpBase          (                                                                   )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >           ( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this))             )
        ,   ::cppu::OWeakObject     (                                                                   )
        // Init member
        ,   m_xFactory              ( xFactory                                                          )
        ,   m_aImpl                 ( xFactory, LINK( this, PathSettings, implts_ConfigurationNotify )  )
{
    // Read initialy the configuration data
    m_aImpl.ReadConfigurationData( m_aPathSettingsData, GetPathPropertyNames() );
}


/*-************************************************************************************************************//**
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
PathSettings::~PathSettings()
{
}

/*-************************************************************************************************************//**
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given propertyhandle and
                return results of this operation. This will be used to ask vetoable
                listener. If no listener has a veto, we will change value realy!
                ( in method setFastPropertyValue_NoBroadcast(...) )

    @attention  Methods of OPropertySethelper are safed by using our shared osl mutex! (see ctor!)
                So we must use different locks to make our implementation threadsafe.

    @seealso    class OPropertySetHelper
    @seealso    method setFastPropertyValue_NoBroadcast()
    @seealso    method impl_tryToChangeProperty()

    @param      "aConvertedValue"   new converted value of property
    @param      "aOldValue"         old value of property
    @param      "nHandle"           handle of property
    @param      "aValue"            new value of property
    @return     sal_True if value will be changed, sal_FALSE otherway

    @onerror    IllegalArgumentException, if you call this with an invalid argument
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL PathSettings::convertFastPropertyValue(   css::uno::Any&          aConvertedValue ,
                                                            css::uno::Any&          aOldValue       ,
                                                            sal_Int32               nHandle         ,
                                                            const css::uno::Any&    aValue          ) throw( css::lang::IllegalArgumentException )
{
    //  Attention: Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException for us !!!

    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case PROPERTYHANDLE_ADDIN       :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aAddinPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_AUTOCORRECT :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aAutoCorrectPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_AUTOTEXT    :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aAutoTextPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_BACKUP      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aBackupPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_BASIC       :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aBasicPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_BITMAP      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aBitmapPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_CONFIG      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aConfigPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_DICTIONARY  :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aDictionaryPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_FAVORITES   :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aFavoritesPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_FILTER      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aFilterPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_GALLERY     :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aGalleryPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_GRAPHIC     :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aGraphicPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_HELP        :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aHelpPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_LINGUISTIC  :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aLinguisticPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_MODULE      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aModulePath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_PALETTE     :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aPalettePath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_PLUGIN      :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aPluginPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_STORAGE     :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aStoragePath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_TEMP        :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aTempPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_TEMPLATE    :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aTemplatePath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_UICONFIG    :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aUIConfigPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_USERCONFIG  :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aUserConfigPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_USERDICTIONARY: bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aUserDictionaryPath, aValue, aOldValue, aConvertedValue );
                                            break;
        case PROPERTYHANDLE_WORK        :   bReturn = impl_tryToChangeProperty( m_aPathSettingsData.m_aWorkPath, aValue, aOldValue, aConvertedValue );
                                            break;
    }

    // Return state of operation.
    return bReturn ;
}

/*-************************************************************************************************************//**
    @short      set value of a bound property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                Handle and value are valid everyway! You must set the new value only.
                After this, baseclass send messages to all listener automaticly.

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    new value of property
    @return     -

    @onerror    An exception is thrown.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL PathSettings::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                const css::uno::Any&    aValue  )
throw( css::uno::Exception )
{
    sal_Bool        bValueValid = sal_False;
    rtl::OUString   aPropertyName;
    rtl::OUString   aNewValue;

    // Check value before setting it
    aValue >>= aNewValue;
    if ( !m_aImpl.CheckAndReplaceNewPathValue( (PathSettingsData::PathSettingsId)nHandle, aNewValue ))
    {
        rtl::OUString aInvalidURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is not a valid file URL: " )));
        aInvalidURL += aNewValue;
        throw css::lang::IllegalArgumentException( aInvalidURL, (cppu::OWeakObject *)this, 0 );
    }

    switch( nHandle )
    {
        case PROPERTYHANDLE_ADDIN       :   m_aPathSettingsData.m_aAddinPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_ADDIN] );
                                            break;
        case PROPERTYHANDLE_AUTOCORRECT :   m_aPathSettingsData.m_aAutoCorrectPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_AUTOCORRECT] );
                                            break;
        case PROPERTYHANDLE_AUTOTEXT    :   m_aPathSettingsData.m_aAutoTextPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_AUTOTEXT] );
                                            break;
        case PROPERTYHANDLE_BACKUP      :   m_aPathSettingsData.m_aBackupPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_BACKUP] );
                                            break;
        case PROPERTYHANDLE_BASIC       :   m_aPathSettingsData.m_aBasicPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_BASIC] );
                                            break;
        case PROPERTYHANDLE_BITMAP      :   m_aPathSettingsData.m_aBitmapPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_BITMAP] );
                                            break;
        case PROPERTYHANDLE_CONFIG      :   m_aPathSettingsData.m_aConfigPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_CONFIG] );
                                            break;
        case PROPERTYHANDLE_DICTIONARY  :   m_aPathSettingsData.m_aDictionaryPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_DICTIONARY] );
                                            break;
        case PROPERTYHANDLE_FAVORITES   :   m_aPathSettingsData.m_aFavoritesPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_FAVORITES] );
                                            break;
        case PROPERTYHANDLE_FILTER      :   m_aPathSettingsData.m_aFilterPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_FILTER] );
                                            break;
        case PROPERTYHANDLE_GALLERY     :   m_aPathSettingsData.m_aGalleryPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_GALLERY] );
                                            break;
        case PROPERTYHANDLE_GRAPHIC     :   m_aPathSettingsData.m_aGraphicPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_GRAPHIC] );
                                            break;
        case PROPERTYHANDLE_HELP        :   m_aPathSettingsData.m_aHelpPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_HELP] );
                                            break;
        case PROPERTYHANDLE_LINGUISTIC  :   m_aPathSettingsData.m_aLinguisticPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_LINGUISTIC] );
                                            break;
        case PROPERTYHANDLE_MODULE      :   m_aPathSettingsData.m_aModulePath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_MODULE] );
                                            break;
        case PROPERTYHANDLE_PALETTE     :   m_aPathSettingsData.m_aPalettePath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_PALETTE] );
                                            break;
        case PROPERTYHANDLE_PLUGIN      :   m_aPathSettingsData.m_aPluginPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_PLUGIN] );
                                            break;
        case PROPERTYHANDLE_STORAGE     :   m_aPathSettingsData.m_aStoragePath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_STORAGE] );
                                            break;
        case PROPERTYHANDLE_TEMP        :   m_aPathSettingsData.m_aTempPath = aNewValue;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_TEMP] );
                                            break;
        case PROPERTYHANDLE_TEMPLATE    :   aValue >>= m_aPathSettingsData.m_aTemplatePath;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_TEMPLATE] );
                                            break;
        case PROPERTYHANDLE_UICONFIG    :   aValue >>= m_aPathSettingsData.m_aUIConfigPath;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_UICONFIG] );
                                            break;
        case PROPERTYHANDLE_USERCONFIG  :   aValue >>= m_aPathSettingsData.m_aUserConfigPath;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_USERCONFIG] );
                                            break;
        case PROPERTYHANDLE_USERDICTIONARY: aValue >>= m_aPathSettingsData.m_aUserDictionaryPath;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_USERDICTIONARY] );
                                            break;
        case PROPERTYHANDLE_WORK        :   aValue >>= m_aPathSettingsData.m_aWorkPath;
                                            aPropertyName = rtl::OUString::createFromAscii( aPropNames[PROPERTYHANDLE_WORK] );
                                            break;
    }

    Sequence< rtl::OUString > aProperties( 1 );
    aProperties[0] = aPropertyName;
    m_aImpl.WriteConfigurationData( m_aPathSettingsData, aProperties );
}

/*-************************************************************************************************************//**
    @short      get value of a bound property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  We don't need any mutex or lock here ... We use threadsafe container or methods here only!

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    current value of property
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL PathSettings::getFastPropertyValue(   css::uno::Any& aValue  ,
                                                    sal_Int32      nHandle ) const
{
    switch( nHandle )
    {
        case PROPERTYHANDLE_ADDIN       :   aValue <<= m_aPathSettingsData.m_aAddinPath;
                                            break;
        case PROPERTYHANDLE_AUTOCORRECT :   aValue <<= m_aPathSettingsData.m_aAutoCorrectPath;
                                            break;
        case PROPERTYHANDLE_AUTOTEXT    :   aValue <<= m_aPathSettingsData.m_aAutoTextPath;
                                            break;
        case PROPERTYHANDLE_BACKUP      :   aValue <<= m_aPathSettingsData.m_aBackupPath;
                                            break;
        case PROPERTYHANDLE_BASIC       :   aValue <<= m_aPathSettingsData.m_aBasicPath;
                                            break;
        case PROPERTYHANDLE_BITMAP      :   aValue <<= m_aPathSettingsData.m_aBitmapPath;
                                            break;
        case PROPERTYHANDLE_CONFIG      :   aValue <<= m_aPathSettingsData.m_aConfigPath;
                                            break;
        case PROPERTYHANDLE_DICTIONARY  :   aValue <<= m_aPathSettingsData.m_aDictionaryPath;
                                            break;
        case PROPERTYHANDLE_FAVORITES   :   aValue <<= m_aPathSettingsData.m_aFavoritesPath;
                                            break;
        case PROPERTYHANDLE_FILTER      :   aValue <<= m_aPathSettingsData.m_aFilterPath;
                                            break;
        case PROPERTYHANDLE_GALLERY     :   aValue <<= m_aPathSettingsData.m_aGalleryPath;
                                            break;
        case PROPERTYHANDLE_GRAPHIC     :   aValue <<= m_aPathSettingsData.m_aGraphicPath;
                                            break;
        case PROPERTYHANDLE_HELP        :   aValue <<= m_aPathSettingsData.m_aHelpPath;
                                            break;
        case PROPERTYHANDLE_LINGUISTIC  :   aValue <<= m_aPathSettingsData.m_aLinguisticPath;
                                            break;
        case PROPERTYHANDLE_MODULE      :   aValue <<= m_aPathSettingsData.m_aModulePath;
                                            break;
        case PROPERTYHANDLE_PALETTE     :   aValue <<= m_aPathSettingsData.m_aPalettePath;
                                            break;
        case PROPERTYHANDLE_PLUGIN      :   aValue <<= m_aPathSettingsData.m_aPluginPath;
                                            break;
        case PROPERTYHANDLE_STORAGE     :   aValue <<= m_aPathSettingsData.m_aStoragePath;
                                            break;
        case PROPERTYHANDLE_TEMP        :   aValue <<= m_aPathSettingsData.m_aTempPath;
                                            break;
        case PROPERTYHANDLE_TEMPLATE    :   aValue <<= m_aPathSettingsData.m_aTemplatePath;
                                            break;
        case PROPERTYHANDLE_UICONFIG    :   aValue <<= m_aPathSettingsData.m_aUIConfigPath;
                                            break;
        case PROPERTYHANDLE_USERCONFIG  :   aValue <<= m_aPathSettingsData.m_aUserConfigPath;
                                            break;
        case PROPERTYHANDLE_USERDICTIONARY: aValue <<= m_aPathSettingsData.m_aUserDictionaryPath;
                                            break;
        case PROPERTYHANDLE_WORK        :   aValue <<= m_aPathSettingsData.m_aWorkPath;
                                            break;
    }
}

/*-************************************************************************************************************//**
    @short      test, if a property will change his value
    @descr      These methods will test, if a property will change his current value, with given parameter.
                We use a helperclass for properties. These class promote this behaviour.
                We implement a helper function for every property-type!

    @seealso    method convertFastPropertyValue()

    @param      "...Property"       ,   the property with his current value
    @param      "aNewValue"         ,   new value for property
    @param      "aOldValue"         ,   old value of property as Any for convertFastPropertyValue
    @param      "aConvertedValue"   ,   new value of property as Any for convertFastPropertyValue(it can be the old one, if nothing is changed!)
    @return     sal_True  ,if value will be changed
    @return     sal_FALSE ,otherwise.

    @onerror    IllegalArgumentException, if convert failed.
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool PathSettings::impl_tryToChangeProperty(    rtl::OUString           sCurrentValue   ,
                                                    const css::uno::Any&    aNewValue       ,
                                                    css::uno::Any&          aOldValue       ,
                                                    css::uno::Any&          aConvertedValue )
throw( css::lang::IllegalArgumentException )
{
    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    rtl::OUString sValue ;
    cppu::convertPropertyValue( sValue, aNewValue );

    // If value change ...
    if( sValue != sCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= sCurrentValue   ;
        aConvertedValue <<= sValue          ;
        // Return OK - "value will be change ..."
        bReturn = sal_True;
    }
    else
    {
        // ... clear information of return parameter!
        aOldValue.clear         () ;
        aConvertedValue.clear   () ;
        // Return NOTHING - "value will not be change ..."
        bReturn = sal_False;
    }

    return bReturn;
}

/*-************************************************************************************************************//**
    @short      return structure and information about bound properties
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper

    @param      -
    @return     structure with property-informations

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
::cppu::IPropertyArrayHelper& SAL_CALL PathSettings::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

/*-************************************************************************************************************//**
    @short      return propertysetinfo
    @descr      You can call this method to get information about transient properties
                of this object.

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper
    @seealso    interface XPropertySet
    @seealso    interface XMultiPropertySet

    @param      -
    @return     reference to object with information [XPropertySetInfo]

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PathSettings::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

/*-************************************************************************************************************//**
    @short      create table with information about properties
    @descr      We use a helper class to support properties. These class need some information about this.
                These method create a new static description table with name, type, r/w-flags and so on ...

    @seealso    class OPropertySetHelper
    @seealso    method getInfoHelper()

    @param      -
    @return     Static table with information about properties.

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
const css::uno::Sequence< css::beans::Property > PathSettings::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!
    static const css::beans::Property pProperties[] =
    {
        css::beans::Property( PROPERTYNAME_ADDIN            , PROPERTYHANDLE_ADDIN              , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_AUTOCORRECT      , PROPERTYHANDLE_AUTOCORRECT        , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_AUTOTEXT         , PROPERTYHANDLE_AUTOTEXT           , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_BACKUP           , PROPERTYHANDLE_BACKUP             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_BASIC            , PROPERTYHANDLE_BASIC              , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_BITMAP           , PROPERTYHANDLE_BITMAP             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_CONFIG           , PROPERTYHANDLE_CONFIG             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_DICTIONARY       , PROPERTYHANDLE_DICTIONARY         , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_FAVORITES        , PROPERTYHANDLE_FAVORITES          , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_FILTER           , PROPERTYHANDLE_FILTER             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_GALLERY          , PROPERTYHANDLE_GALLERY            , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_GRAPHIC          , PROPERTYHANDLE_GRAPHIC            , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_HELP             , PROPERTYHANDLE_HELP               , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_LINGUISTIC       , PROPERTYHANDLE_LINGUISTIC         , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_MODULE           , PROPERTYHANDLE_MODULE             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_PALETTE          , PROPERTYHANDLE_PALETTE            , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_PLUGIN           , PROPERTYHANDLE_PLUGIN             , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_STORAGE          , PROPERTYHANDLE_STORAGE            , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_TEMP             , PROPERTYHANDLE_TEMP               , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_TEMPLATE         , PROPERTYHANDLE_TEMPLATE           , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_UICONFIG         , PROPERTYHANDLE_UICONFIG           , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_USERCONFIG       , PROPERTYHANDLE_USERCONFIG         , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_USERDICTIONARY   , PROPERTYHANDLE_USERDICTIONARY     , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( PROPERTYNAME_WORK             , PROPERTYHANDLE_WORK               , ::getCppuType((rtl::OUString*)NULL), css::beans::PropertyAttribute::BOUND )
    };

    // Use it to initialize sequence!
    static const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pProperties, PROPERTYCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

IMPL_LINK( PathSettings, implts_ConfigurationNotify, PathSettingsNotify*, pPathSettingsNotify )
{
    if ( pPathSettingsNotify != NULL )
    {
        Sequence< ::rtl::OUString >&    rSettingsChanged = pPathSettingsNotify->aPathSettingsChanged;
        std::vector< int >              aHandleVector( rSettingsChanged.getLength() );
        PathSettingsData                aNewPathSettings;

        {
            // Thread-safe reading from configuration
            ResetableGuard aGuard( m_aLock );
            m_aImpl.ReadConfigurationData( aNewPathSettings, rSettingsChanged );

            const rtl::OUString* pNames = rSettingsChanged.getConstArray();
            for ( sal_Int32 i = 0; i < rSettingsChanged.getLength(); i++ )
            {
                int nHandle = m_aImpl.GetHandleFromPropertyName( pNames[i] );
                if ( nHandle >= 0 )
                    aHandleVector.push_back( nHandle );
            }
        }

        // Set the values through our API implementation to have automically listener notifications
        for ( sal_uInt32 i = 0; i < aHandleVector.size(); i++ )
        {
            Any aAny;
            aAny <<= aNewPathSettings.GetValue( (PathSettingsData::PathSettingsId)aHandleVector[i] );
            setPropertyValue( rtl::OUString::createFromAscii( aPropNames[ aHandleVector[i] ]), aAny );
        }
    }

    return 0;
}

} // namespace framework
