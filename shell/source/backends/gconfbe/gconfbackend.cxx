/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gconfbackend.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#include "gconfbackend.hxx"
#include "gconflayer.hxx"
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#include <osl/process.h>
#include <osl/time.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.h>

#include <stdio.h>

#ifdef ENABLE_LOCKDOWN
// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue SetupConfigurationValuesList[] =
{
    {
        SETTING_WRITER_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/writer_default_document_format",
        "org.openoffice.Setup/Office/Factories/com.sun.star.text.TextDocument/ooSetupFactoryDefaultFilter",
        "string",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_IMPRESS_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/impress_default_document_format",
        "org.openoffice.Setup/Office/Factories/com.sun.star.presentation.PresentationDocument/ooSetupFactoryDefaultFilter",
        "string",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_CALC_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/calc_default_document_format",
        "org.openoffice.Setup/Office/Factories/com.sun.star.sheet.SpreadsheetDocument/ooSetupFactoryDefaultFilter",
        "string",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },
};
#endif // ENABLE_LOCKDOWN

#ifdef ENABLE_LOCKDOWN
// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue RecoveryConfigurationValuesList[] =
{
    {
        SETTING_AUTO_SAVE,
        GCONF_AUTO_SAVE_KEY,
        "org.openoffice.Office.Recovery/AutoSave/Enabled",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_AUTO_SAVE_INTERVAL,
        "/apps/openoffice/auto_save_interval",
        "org.openoffice.Office.Recovery/AutoSave/TimeIntervall",
        "int",
        sal_False,
        sal_False,
        SETTING_AUTO_SAVE
    },
};
#endif // ENABLE_LOCKDOWN

/*
 * This should be in a different backend actually, but this has to wait ..
 */

#ifdef ENABLE_LOCKDOWN
// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue UserProfileConfigurationValuesList[] =
{
    {
        SETTING_USER_GIVENNAME,
        "/desktop/gnome/url-handlers/mailto/command", // dummy, needed for getTimestamp
        "org.openoffice.UserProfile/Data/givenname",
        "string",
        sal_False,
        sal_True,
        SETTING_USER_GIVENNAME
    },

    {
        SETTING_USER_SURNAME,
        "/desktop/gnome/url-handlers/mailto/command", // dummy, needed for getTimestamp
        "org.openoffice.UserProfile/Data/sn",
        "string",
        sal_False,
        sal_True,
        SETTING_USER_SURNAME
    },
};
#endif // ENABLE_LOCKDOWN

// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue VCLConfigurationValuesList[] =
{
    {
        SETTING_ENABLE_ACCESSIBILITY,
        "/desktop/gnome/interface/accessibility",
        "org.openoffice.VCL/Settings/Accessibility/EnableATToolSupport",
        "string",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

#ifdef ENABLE_LOCKDOWN

    {
        SETTING_DISABLE_PRINTING,
        "/desktop/gnome/lockdown/disable_printing",
        "org.openoffice.VCL/Settings/DesktopManagement/DisablePrinting",
        "string",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

#endif // ENABLE_LOCKDOWN

};

// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue InetConfigurationValuesList[] =
{
    {
        SETTING_PROXY_MODE,
        GCONF_PROXY_MODE_KEY,
        "org.openoffice.Inet/Settings/ooInetProxyType",
        "int",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_PROXY_HTTP_HOST,
        "/system/http_proxy/host",
        "org.openoffice.Inet/Settings/ooInetHTTPProxyName",
        "string",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_HTTP_PORT,
        "/system/http_proxy/port",
        "org.openoffice.Inet/Settings/ooInetHTTPProxyPort",
        "int",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

     {
        SETTING_PROXY_HTTPS_HOST,
        "/system/proxy/secure_host",
        "org.openoffice.Inet/Settings/ooInetHTTPSProxyName",
        "string",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_HTTPS_PORT,
        "/system/proxy/secure_port",
        "org.openoffice.Inet/Settings/ooInetHTTPSProxyPort",
        "int",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_FTP_HOST,
        "/system/proxy/ftp_host",
        "org.openoffice.Inet/Settings/ooInetFTPProxyName",
        "string",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_FTP_PORT,
        "/system/proxy/ftp_port",
        "org.openoffice.Inet/Settings/ooInetFTPProxyPort",
        "int",
        sal_False,
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_NO_PROXY_FOR,
        "/system/http_proxy/ignore_hosts",
        "org.openoffice.Inet/Settings/ooInetNoProxy",
        "string",
        sal_False,
        sal_True,
        SETTING_PROXY_MODE
    },
};

// each entry should have an identifying ConfigurationSetting
static const ConfigurationValue CommonConfigurationValuesList[] =
{
    {
        SETTING_MAILER_PROGRAM,
        "/desktop/gnome/url-handlers/mailto/command",
        "org.openoffice.Office.Common/ExternalMailer/Program",
        "string",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },
    {
        SETTING_SOURCEVIEWFONT_NAME,
        "/desktop/gnome/interface/monospace_font_name",
        "org.openoffice.Office.Common/Font/SourceViewFont/FontName",
        "string",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },
    {
        SETTING_SOURCEVIEWFONT_HEIGHT,
        "/desktop/gnome/interface/monospace_font_name",
        "org.openoffice.Office.Common/Font/SourceViewFont/FontHeight",
        "short",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

#ifdef ENABLE_LOCKDOWN

    {
        SETTING_USE_SYSTEM_FILE_DIALOG,
        "/apps/openoffice/use_system_file_dialog",
        "org.openoffice.Office.Common/Misc/UseSystemFileDialog",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_DISABLE_UI_CUSTOMIZATION,
        "/apps/openoffice/lockdown/disable_ui_customization",
        "org.openoffice.Office.Common/Misc/DisableUICustomization",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_PRINTING_MODIFIES_DOCUMENT,
        "/apps/openoffice/printing_modifies_doc",
        "org.openoffice.Office.Common/Print/PrintingModifiesDocument",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_ICONS_IN_MENUS,
        "/apps/openoffice/show_menu_icons",
        "org.openoffice.Office.Common/View/Menu/ShowIconsInMenues",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_INACTIVE_MENUITEMS,
        "/apps/openoffice/show_menu_inactive_items",
        "org.openoffice.Office.Common/View/Menu/DontHideDisabledEntry",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_FONT_PREVIEW,
        "/apps/openoffice/show_font_preview",
        "org.openoffice.Office.Common/Font/View/ShowFontBoxWYSIWYG",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_FONT_HISTORY,
        "/apps/openoffice/show_font_history",
        "org.openoffice.Office.Common/Font/View/History",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_ENABLE_OPENGL,
        "/apps/openoffice/use_opengl",
        "org.openoffice.Office.Common/_3D_Engine/OpenGL",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_OPTIMIZE_OPENGL,
        "/apps/openoffice/optimize_opengl",
        "org.openoffice.Office.Common/_3D_Engine/OpenGL_Faster",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_USE_SYSTEM_FONT,
        "/apps/openoffice/use_system_font",
        "org.openoffice.Office.Common/Accessibility/IsSystemFont",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_USE_FONT_ANTI_ALIASING,
        "/apps/openoffice/use_font_anti_aliasing",
        "org.openoffice.Office.Common/View/FontAntiAliasing/Enabled",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_FONT_ANTI_ALIASING_MIN_PIXEL,
        "/apps/openoffice/font_anti_aliasing_min_pixel",
        "org.openoffice.Office.Common/View/FontAntiAliasing/MinPixelHeight",
        "short",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_CREATE_PDF,
        "/apps/openoffice/lockdown/warn_info_create_pdf",
        "org.openoffice.Office.Common/Security/Scripting/WarnCreatePDF",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_PRINT_DOC,
        "/apps/openoffice/lockdown/warn_info_printing",
        "org.openoffice.Office.Common/Security/Scripting/WarnPrintDoc",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_SAVEORSEND_DOC,
        "/apps/openoffice/lockdown/warn_info_saving",
        "org.openoffice.Office.Common/Security/Scripting/WarnSaveOrSendDoc",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_SIGN_DOC,
        "/apps/openoffice/lockdown/warn_info_signing",
        "org.openoffice.Office.Common/Security/Scripting/WarnSignDoc",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_REMOVE_PERSONAL_INFO,
        "/apps/openoffice/lockdown/remove_personal_info_on_save",
        "org.openoffice.Office.Common/Security/Scripting/RemovePersonalInfoOnSaving",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_RECOMMEND_PASSWORD,
        "/apps/openoffice/lockdown/recommend_password_on_save",
        "org.openoffice.Office.Common/Security/Scripting/RecommendPasswordProtection",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_UNDO_STEPS,
        "/apps/openoffice/undo_steps",
        "org.openoffice.Office.Common/Undo/Steps",
        "int",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SYMBOL_SET,
        "/apps/openoffice/icon_size",
        "org.openoffice.Office.Common/Misc/SymbolSet",
        "short",
        sal_False,
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_MACRO_SECURITY_LEVEL,
        "/apps/openoffice/lockdown/macro_security_level",
        "org.openoffice.Office.Common/Security/Scripting/MacroSecurityLevel",
        "int",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_CREATE_BACKUP,
        "/apps/openoffice/create_backup",
        "org.openoffice.Office.Common/Save/Document/CreateBackup",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_ALIEN_FORMAT,
        "/apps/openoffice/warn_alien_format",
        "org.openoffice.Office.Common/Save/Document/WarnAlienFormat",
        "boolean",
        sal_False,
        sal_False,
        SETTINGS_LAST
    },

#endif // ENABLE_LOCKDOWN

};

static const ConfigurationValue PathsConfigurationValuesList[] =
{
    {
        SETTING_WORK_DIRECTORY,
        "/desktop/gnome/url-handlers/mailto/command", // dummy, needed for getTimestamp
        "org.openoffice.Office.Paths/Variables/Work",
        "string",
        sal_False,
        sal_True,
        SETTING_WORK_DIRECTORY, // so that the existence of the dir can be checked
    },
};

#ifdef ENABLE_LOCKDOWN
static const char * SetupPreloadValuesList[] =
{
    "/apps/openoffice",
    NULL
};
#endif // ENABLE_LOCKDOWN

#ifdef ENABLE_LOCKDOWN
static const char * RecoveryPreloadValuesList[] =
{
    "/apps/openoffice",
    NULL
};
#endif // ENABLE_LOCKDOWN

#ifdef ENABLE_LOCKDOWN
static const char * UserProfilePreloadValuesList[] =
{
    NULL
};
#endif // ENABLE_LOCKDOWN

static const char * VCLPreloadValuesList[] =
{
    "/desktop/gnome/interface",
#ifdef ENABLE_LOCKDOWN
    "/desktop/gnome/lockdown",
#endif // ENABLE_LOCKDOWN
    NULL
};

static const char * InetPreloadValuesList[] =
{
    "/system/proxy",
    "/system/http_proxy/host",
    NULL
};

static const char * CommonPreloadValuesList[] =
{
    "/desktop/gnome/url-handlers/mailto",
#ifdef ENABLE_LOCKDOWN
    "/apps/openoffice/lockdown",
    "/apps/openoffice",
#endif // ENABLE_LOCKDOWN
    NULL
};

static const char * PathsPreloadValuesList[] =
{
    NULL
};

//==============================================================================

/*
void ONotificationThread::run()
{
    mLoop= NULL;
    //Need to start a GMain loop for notifications to work
    mLoop=g_main_loop_new(g_main_context_default(),FALSE);
    g_main_loop_run(mLoop);
} ;

*/

//------------------------------------------------------------------------------

GconfBackend* GconfBackend::mInstance= 0;

GconfBackend* GconfBackend::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    if(mInstance == 0)
    {
        mInstance = new GconfBackend (xContext);
    }

    return mInstance;
}

//------------------------------------------------------------------------------

GconfBackend::GconfBackend(const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException)
    : BackendBase(mMutex), m_xContext(xContext)
//    , mNotificationThread(NULL)


{
}

//------------------------------------------------------------------------------

GconfBackend::~GconfBackend(void) {


//    delete (mNotificationThread);
    GconfBackend::mClient = NULL;

}

//------------------------------------------------------------------------------

GConfClient* GconfBackend::mClient= 0;


GConfClient* GconfBackend::getGconfClient()
{

    if (mClient == NULL)
    {
        /* initialize glib object type library */
        g_type_init();

        GError* aError = NULL;
        if (!gconf_init(0, NULL, &aError))
        {
            rtl::OUStringBuffer msg;
            msg.appendAscii("GconfBackend:GconfLayer: Cannot Initialize Gconf connection - " );
            msg.appendAscii(aError->message);

            g_error_free(aError);
            aError = NULL;
            throw uno::RuntimeException(msg.makeStringAndClear(),NULL);
        }

        mClient = gconf_client_get_default();
        if (!mClient)
        {
            throw uno::RuntimeException(rtl::OUString::createFromAscii
                ("GconfBackend:GconfLayer: Cannot Initialize Gconf connection"),NULL);
        }
    }

    return mClient;
}

//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL GconfBackend::getLayer(
    const rtl::OUString& aComponent, const rtl::OUString& /*aTimestamp*/)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{
    uno::Reference<backend::XLayer> xLayer;

    if( aComponent.equalsAscii("org.openoffice.Office.Common" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 CommonConfigurationValuesList,
                                 G_N_ELEMENTS( CommonConfigurationValuesList ),
                                 CommonPreloadValuesList );
    }
    else if( aComponent.equalsAscii("org.openoffice.Inet" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 InetConfigurationValuesList,
                                 G_N_ELEMENTS( InetConfigurationValuesList ),
                                 InetPreloadValuesList );
    }
    else if( aComponent.equalsAscii("org.openoffice.VCL" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 VCLConfigurationValuesList,
                                 G_N_ELEMENTS( VCLConfigurationValuesList ),
                                 VCLPreloadValuesList );
    }
    else if( aComponent.equalsAscii("org.openoffice.Office.Paths" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 PathsConfigurationValuesList,
                                 G_N_ELEMENTS( PathsConfigurationValuesList ),
                                 PathsPreloadValuesList );
    }

#ifdef ENABLE_LOCKDOWN
    else if( aComponent.equalsAscii("org.openoffice.UserProfile" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 UserProfileConfigurationValuesList,
                                 G_N_ELEMENTS( UserProfileConfigurationValuesList ),
                                 UserProfilePreloadValuesList );
    }
    else if( aComponent.equalsAscii("org.openoffice.Office.Recovery" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 RecoveryConfigurationValuesList,
                                 G_N_ELEMENTS( RecoveryConfigurationValuesList ),
                                 RecoveryPreloadValuesList );
    }
    else if( aComponent.equalsAscii("org.openoffice.Setup" ) )
    {
        xLayer = new GconfLayer( m_xContext,
                                 SetupConfigurationValuesList,
                                 G_N_ELEMENTS( SetupConfigurationValuesList ),
                                 SetupPreloadValuesList );
    }
#endif // ENABLE_LOCKDOWN

    return xLayer;
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
GconfBackend::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException( rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("GconfBackend: No Update Operation allowed, Read Only access") ),
        *this) ;
}

//------------------------------------------------------------------------------

// currently not used
#if 0
static void
keyChangedCallback(GConfClient* aClient,
                   guint aID,
                   GConfEntry* aEntry,
                   gpointer aBackend)
{

    OSL_TRACE("In KeyChangedCallback Function");
    rtl::OUString aGconfKey= rtl::OUString::createFromAscii(aEntry->key);
    GconfBackend * aGconfBe = (GconfBackend*) aBackend;

    aGconfBe->notifyListeners(aGconfKey);

}
#endif

//------------------------------------------------------------------------------

void GconfBackend::notifyListeners(const rtl::OUString& /*aGconfKey*/)
{
/*
    //look up associated component from Map using GconfKey
    KeyMappingTable::iterator aIter;
    rtl::OUString aComponent;
    for( aIter= mKeyMap.begin(); aIter != mKeyMap.end(); aIter++)
    {
        if (aIter->second.mGconfName == aGconfKey)
        {
            aComponent = aIter->first;
            break;
        }

    }


    //Need to store updated layer TimeStamp as Gconf library
    //has no access to TimeStamp via its api
    TimeValue aTimeValue ={0,0} ;
    osl_getSystemTime(&aTimeValue);

    oslDateTime aLayerTS;
    rtl::OUString aTimeStamp;

    if (osl_getDateTimeFromTimeValue(&aTimeValue, &aLayerTS)) {
        sal_Char asciiStamp [20] ;

        sprintf(asciiStamp, "%04d%02d%02d%02d%02d%02dZ",
                aLayerTS.Year, aLayerTS.Month, aLayerTS.Day,
                aLayerTS.Hours, aLayerTS.Minutes, aLayerTS.Seconds) ;
        aTimeStamp = rtl::OUString::createFromAscii(asciiStamp) ;
    }


    TSMappingTable::iterator aTSIter;
    aTSIter = mTSMap.find(aComponent);
    if (aTSIter == mTSMap.end())
    {
        mTSMap.insert(TSMappingTable::value_type(aComponent,aTimeStamp));
    }
    else
    {
        aTSIter->second = aTimeStamp;

    }
    typedef ListenerList::iterator LLIter;
    typedef std::pair<LLIter, LLIter> BFRange;
    BFRange aRange = mListenerList.equal_range(aComponent);
    while (aRange.first != aRange.second)
    {
        LLIter cur = aRange.first++;
        backend::ComponentChangeEvent aEvent;
        aEvent.Component = aComponent;
        aEvent.Source = *this;
        cur->second->componentDataChanged(aEvent);

    }
*/
}


//------------------------------------------------------------------------------
void SAL_CALL GconfBackend::addChangesListener(
    const uno::Reference<backend::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
/*
    osl::MutexGuard aGuard(mMutex);

    GConfClient* aClient = getGconfClient();

    ListenerList::iterator aIter;
    aIter = mListenerList.find(aComponent);
    if (aIter == mListenerList.end())
    {
        typedef KeyMappingTable::iterator KMTIter;
        typedef std::pair<KMTIter, KMTIter> BFRange;

        BFRange aRange = mKeyMap.equal_range(aComponent);

        while (aRange.first != aRange.second)
        {
            KMTIter cur = aRange.first++;

            sal_Int32 nIndex = cur->second.mGconfName.lastIndexOf('/');
            rtl::OUString aDirectory = cur->second.mGconfName.copy(0, nIndex);
            rtl::OString aDirectoryStr= rtl::OUStringToOString(aDirectory, RTL_TEXTENCODING_ASCII_US);
            GError* aError = NULL;
            gconf_client_add_dir(aClient,
                                 aDirectoryStr.getStr(),
                                 GCONF_CLIENT_PRELOAD_NONE,
                                 &aError);

            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot register listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }

            rtl::OString aKey =  rtl::OUStringToOString(cur->second.mGconfName,
                                                        RTL_TEXTENCODING_ASCII_US);

            GConfClientNotifyFunc aNotifyFunc = &keyChangedCallback;
            sal_uInt32 aID = gconf_client_notify_add(
                                                    aClient,
                                                    aKey.getStr(),
                                                    aNotifyFunc,
                                                    this,
                                                    NULL,
                                                    &aError);
            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot register listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }



        }

    }
*/

/*
    if (mNotificationThread == NULL)
    {

        mNotificationThread = new ONotificationThread();

        if ( mNotificationThread == NULL)
        {
          OSL_ENSURE(false,"Could not start Notification Thread ");
        }
        else
        {
            mNotificationThread->create();
        }
    }
*/
    //Store listener in list
    mListenerList.insert(ListenerList::value_type(aComponent, xListener));


}
//------------------------------------------------------------------------------
void SAL_CALL GconfBackend::removeChangesListener(
    const uno::Reference<backend::XBackendChangesListener>& /*xListener*/,
    const rtl::OUString& /*aComponent*/)
    throw (::com::sun::star::uno::RuntimeException)
{
/*
    osl::MutexGuard aGuard(mMutex);
    GConfClient* aClient = GconfBackend::getGconfClient();
    ListenerList::iterator aIter;
    aIter = mListenerList.find(aComponent);
    if (aIter == mListenerList.end())
    {

        OSL_TRACE("GconfBackend:: Cannot deregister listener for Component %s - Listner not registered",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );

        typedef KeyMappingTable::iterator KMTIter;
        typedef std::pair<KMTIter, KMTIter> BFRange;

        BFRange aRange = mKeyMap.equal_range(aComponent);

        while (aRange.first != aRange.second)
    {
            KMTIter cur = aRange.first++;

            sal_Int32 nIndex = cur->second.mGconfName.lastIndexOf('/');
            rtl::OUString aDirectory = cur->second.mGconfName.copy(0, nIndex);
            rtl::OString aDirectoryStr= rtl::OUStringToOString(aDirectory, RTL_TEXTENCODING_ASCII_US);
            GError* aError = NULL;
            gconf_client_remove_dir(aClient,
                                    aDirectoryStr.getStr(),
                                    &aError);

            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot deRegister listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }
       }
    }
*/
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL GconfBackend::getBackendName(void) {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.GconfBackend") );
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL GconfBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL GconfBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(1) ;
    aServices[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.GconfBackend")) ;

    return aServices ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL GconfBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL GconfBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

// ---------------------------------------------------------------------------------------


