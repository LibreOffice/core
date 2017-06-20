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

#include <unistd.h>

#include "unx/cpdmgr.hxx"

#include "osl/diagnose.h"
#include "osl/thread.h"

#include "rtl/ustrbuf.hxx"

#include <config_dbus.h>
#include <config_gio.h>

#include <algorithm>

using namespace psp;
using namespace osl;

// Function to execute when name is acquired on the bus
static void on_name_acquired (GDBusConnection *connection,
                              const gchar *name,
                              gpointer)
{
    g_message("Name Acquired %s", name);

    GError *local_error, *error;
    gchar* contents;
    GDBusNodeInfo *introspection_data = nullptr;

    // Get Interface for introspection
    g_file_get_contents ("/home/yash/bigGit/PrintDialog_Backend/org.openprinting.Frontend.xml", &contents, nullptr, &error);
    introspection_data = g_dbus_node_info_new_for_xml (contents, nullptr);

    g_dbus_connection_register_object (connection,
                                       "/org/libreoffice/PrintDialog",
                                       introspection_data->interfaces[0],
                                       nullptr,
                                       nullptr,  /* user_data */
                                       nullptr,  /* user_data_free_func */
                                       nullptr); /* GError** */
    local_error = nullptr;
    g_dbus_connection_emit_signal (connection,
                                   nullptr,
                                   "/org/libreoffice/PrintDialog",
                                   "org.openprinting.PrintFrontend",
                                   "GetBackend",
                                   nullptr,
                                   &local_error);
    g_assert_no_error (local_error);
    g_message("Emitted Signal GetBackend");
}

static void on_name_lost (GDBusConnection *,
                          const gchar *name,
                          gpointer)
{
    g_message("Name Lost: %s", name);
}

static void got_signal (GDBusConnection *connection,
                        const gchar     *sender_name,
                        const gchar     *object_path,
                        const gchar     *interface_name,
                        const gchar     *signal_name,
                        GVariant        *parameters,
                        gpointer        user_data)
{
    CPDManager* current = static_cast<CPDManager*>(user_data);
    g_message("Received %s from %s at %s", signal_name, sender_name, object_path);
    if (g_strcmp0 (signal_name, "PrinterAdded") == 0)
        {
            GDBusProxy *proxy;
            proxy = current -> getProxy(sender_name);
            if (proxy == nullptr) {
                gchar* contents;
                GDBusNodeInfo *introspection_data = nullptr;

                // Get Interface for introspection
                g_file_get_contents ("/home/yash/bigGit/PrintDialog_Backend/org.openprinting.Backend.xml", &contents, nullptr, nullptr);
                introspection_data = g_dbus_node_info_new_for_xml (contents, nullptr);
                proxy = g_dbus_proxy_new_sync (connection,
                                               G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                               introspection_data->interfaces[0],
                                               sender_name,
                                               object_path,
                                               interface_name,
                                               nullptr,
                                               nullptr);
                std::pair<std::string, GDBusProxy *> new_backend (sender_name, proxy);
                current -> addBackend(new_backend);
            }
            CPDPrinter *pDest = static_cast<CPDPrinter *>(malloc(sizeof(CPDPrinter)));
            // const gchar *name, *info, *location, *make_and_model, *state;
            // gboolean is_accepting_jobs;
            g_variant_get (parameters, "(ssssbs)", &(pDest -> name), &(pDest -> info), &(pDest -> location), &(pDest -> make_and_model), &(pDest -> is_accepting_jobs), &(pDest -> printer_state));
            g_message("State: %s", (pDest -> printer_state).c_str());
            const char *pName = (pDest -> name).c_str();
            rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
            OUString aPrinterName = OStringToOUString( pName, aEncoding );
            std::pair<OUString, CPDPrinter *> newDest (aPrinterName, pDest);
            current -> addDestination(newDest);
        } else if (g_strcmp0 (signal_name, "UpdatePrinter") == 0)
        {
        } else if (g_strcmp0 (signal_name, "DeletePrinter") == 0)
        {
        }
}

GDBusProxy * CPDManager::getProxy(std::string target) {
    std::unordered_map<std::string, GDBusProxy *>::const_iterator it = this -> m_pBackends.find(target);
    if (it == this -> m_pBackends.end()) {
        return nullptr;
    }
    return it -> second;
}

void CPDManager::addBackend(std::pair<std::string, GDBusProxy *> pair) {
    this -> m_pBackends.insert(pair);
    g_message("Add new backend");
}

void CPDManager::addDestination(std::pair<OUString, CPDPrinter * > pair) {
    // initialize printer with possible configuration from psprint.conf
    OUString aPrinterName = pair.first;
    CPDPrinter *pDest = pair.second;
    std::unordered_map<OUString, CPDPrinter *, OUStringHash>::iterator it = m_aCPDDestMap.find( aPrinterName );
    if (it == m_aCPDDestMap.end()) {
        m_aCPDDestMap.insert(pair);
    } else {
        m_aCPDDestMap.erase(it);
        m_aCPDDestMap.insert(pair);
    }
    bool bSetToGlobalDefaults = m_aPrinters.find( aPrinterName ) == m_aPrinters.end();
    Printer aPrinter = m_aPrinters[ aPrinterName ];
    if( bSetToGlobalDefaults )
        aPrinter.m_aInfo = m_aGlobalDefaults;
    aPrinter.m_aInfo.m_aPrinterName = aPrinterName;
    // if( pDest->is_default )
    //     m_aDefaultPrinter = aPrinterName;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const char *pInfo = (pDest -> info).c_str();
    aPrinter.m_aInfo.m_aComment = OStringToOUString(pInfo, aEncoding);
    const char *pLocation = (pDest -> location).c_str();
    aPrinter.m_aInfo.m_aLocation = OStringToOUString(pLocation, aEncoding);
    OUStringBuffer aBuf( 256 );
    aBuf.append( "CPD:" );
    aBuf.append( aPrinterName );
    // note: the parser that goes with the PrinterInfo
    // is created implicitly by the JobData::operator=()
    // when it detects the NULL ptr m_pParser.
    // if we wanted to fill in the parser here this
    // would mean we'd have to download PPDs for each and
    // every printer - which would be really bad runtime
    // behaviour
    aPrinter.m_aInfo.m_pParser = nullptr;
    aPrinter.m_aInfo.m_aContext.setParser( nullptr );
    aPrinter.m_aInfo.setDefaultBackend(true);
    aPrinter.m_aInfo.m_aDriverName = aBuf.makeStringAndClear();
    aPrinter.m_bModified = false;
    m_aPrinters[ aPrinter.m_aInfo.m_aPrinterName ] = aPrinter;
    g_message("Add new Printer");
}

/*
 *  CPDManager class
 */

CPDManager* CPDManager::tryLoadCPD()
{
    CPDManager* pManager = nullptr;
#if ENABLE_DBUS && ENABLE_GIO
    static const char* pEnv = getenv("SAL_DISABLE_CPD");

    if (!pEnv || !*pEnv)
        pManager = new CPDManager();
    g_message("Loaded CPD\n");
#endif
    return pManager;
}

CPDManager::CPDManager() :
    PrinterInfoManager( PrinterInfoManager::Type::CPD )
{
    // Get Destinations number and pointers
    GError *error = nullptr;
    g_message("Created CPDManager\n");
    m_pConnection = g_bus_get_sync (G_BUS_TYPE_SESSION, nullptr, &error);
    g_assert_no_error (error);
}

CPDManager::~CPDManager()
{
    g_dbus_connection_emit_signal (m_pConnection,
                                   nullptr,
                                   "/org/libreoffice/PrintDialog",
                                   "org.openprinting.PrintFrontend",
                                   "StopListing",
                                   nullptr,
                                   nullptr);
    g_message("Sent StopListing\n");
    g_dbus_connection_flush_sync (m_pConnection,
                                  nullptr,
                                  nullptr);
    g_dbus_connection_close_sync (m_pConnection,
                                  nullptr,
                                  nullptr);
    // Free pointers to destinations in cpd
    fprintf( stderr, "Destroyed CPDManager\n");
}





void CPDManager::initialize()
{
    // get normal printers, clear printer list
    PrinterInfoManager::initialize();
    g_message("Initialised CPDManager\n");
    g_bus_own_name_on_connection (m_pConnection,
                                  "org.libreoffice.print-dialog",
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  on_name_acquired,
                                  on_name_lost,
                                  NULL,
                                  NULL);

    g_dbus_connection_signal_subscribe (m_pConnection,                    // DBus Connection
                                        NULL,                             // Sender Name
                                        "org.openprinting.PrintBackend",  // Sender Interface
                                        NULL,                             // Signal Name
                                        NULL,                             // Object Path
                                        NULL,                             // arg0 behaviour
                                        G_DBUS_SIGNAL_FLAGS_NONE,         // Signal Flags
                                        got_signal,                       // Callback Function
                                        this,
                                        NULL);
    // If password CB is needed
    //cpdSetPasswordCB( setPasswordCallback );
}

void CPDManager::setupJobContextData( JobData& rData )
{
    std::unordered_map< OUString, CPDPrinter *, OUStringHash >::iterator dest_it =
        m_aCPDDestMap.find( rData.m_aPrinterName );

    if( dest_it == m_aCPDDestMap.end() )
        return PrinterInfoManager::setupJobContextData( rData );

    std::unordered_map< OUString, Printer, OUStringHash >::iterator p_it =
        m_aPrinters.find( rData.m_aPrinterName );
    if( p_it == m_aPrinters.end() ) // huh ?
        {
            SAL_WARN("vcl.unx.print", "CPD printer list in disorder, "
                     "no dest for printer " << rData.m_aPrinterName);
            return;
        }
    g_message("Setup JobContextData called");
}

FILE* CPDManager::startSpool( const OUString& rPrintername, bool bQuickCommand )
{
    SAL_INFO( "vcl.unx.print", "startSpool: " << rPrintername << " " << (bQuickCommand ? "true" : "false") );
    if( m_aCPDDestMap.find( rPrintername ) == m_aCPDDestMap.end() )
        {
            SAL_INFO( "vcl.unx.print", "defer to PrinterInfoManager::startSpool" );
            return PrinterInfoManager::startSpool( rPrintername, bQuickCommand );
        }
    OUString aTmpURL, aTmpFile;
    osl_createTempFile( nullptr, nullptr, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    OString aSysFile = OUStringToOString( aTmpFile, osl_getThreadTextEncoding() );
    FILE* fp = fopen( aSysFile.getStr(), "w" );
    if( fp )
        m_aSpoolFiles[fp] = aSysFile;

    return fp;
}

bool CPDManager::endSpool( const OUString& rPrintername, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString& rFaxNumber )
{
    SAL_INFO( "vcl.unx.print", "endSpool: " << rPrintername << "," << rJobTitle << " copy count = " << rDocumentJobData.m_nCopies );
    std::unordered_map< OUString, CPDPrinter *, OUStringHash >::iterator dest_it =
        m_aCPDDestMap.find( rPrintername );
    if( dest_it == m_aCPDDestMap.end() )
        {
            SAL_INFO( "vcl.unx.print", "defer to PrinterInfoManager::endSpool" );
            return PrinterInfoManager::endSpool( rPrintername, rJobTitle, pFile, rDocumentJobData, bBanner, rFaxNumber );
        }

    std::unordered_map< FILE*, OString, FPtrHash >::const_iterator it = m_aSpoolFiles.find( pFile );
    if( it != m_aSpoolFiles.end() )
        {
            fclose( pFile );
            rtl_TextEncoding aEnc = osl_getThreadTextEncoding();
            OString sJobName(OUStringToOString(rJobTitle, aEnc));
            if (!rFaxNumber.isEmpty())
                {
                    sJobName = OUStringToOString(rFaxNumber, aEnc);
                }
            CPDPrinter* pDest = dest_it->second;
            g_message("Printing");
            // g_dbus_proxy_call(pDest -> backend, "printJob",
            //                   g_variant_new("(s)", "yo"),
            //                   G_DBUS_CALL_FLAGS_NONE,
            //                   -1, nullptr, nullptr, nullptr);
            unlink( it->second.getStr() );
            m_aSpoolFiles.erase( pFile );
        }

    return true;
}

bool CPDManager::checkPrintersChanged( bool )
{
    bool bChanged = m_aPrintersChanged;
    m_aPrintersChanged = false;
    g_dbus_connection_emit_signal (m_pConnection,
                                   nullptr,
                                   "/org/libreoffice/PrintDialog",
                                   "org.openprinting.PrintFrontend",
                                   "RefreshBackend",
                                   nullptr,
                                   nullptr);
    return bChanged;
}

bool CPDManager::addPrinter( const OUString& rName, const OUString& rDriver )
{
    // don't touch the CPD printers
    if( m_aCPDDestMap.find( rName ) != m_aCPDDestMap.end() ||
        rDriver.startsWith("CPD:")
        )
        return false;
    return PrinterInfoManager::addPrinter( rName, rDriver );
}

bool CPDManager::removePrinter( const OUString& rName, bool bCheck )
{
    // don't touch the CPD printers
    if( m_aCPDDestMap.find( rName ) != m_aCPDDestMap.end() )
        return false;
    return PrinterInfoManager::removePrinter( rName, bCheck );
}

bool CPDManager::setDefaultPrinter( const OUString& rName )
{
    bool bSuccess = false;
    std::unordered_map< OUString, CPDPrinter *, OUStringHash >::iterator nit =
        m_aCPDDestMap.find( rName );
    if( nit != m_aCPDDestMap.end())
        {
            m_aDefaultPrinter = rName;
            bSuccess = true;
        }
    else
        bSuccess = PrinterInfoManager::setDefaultPrinter( rName );

    return bSuccess;
}

bool CPDManager::writePrinterConfig()
{
    return PrinterInfoManager::writePrinterConfig();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

