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
void CPDManager::onNameAcquired (GDBusConnection *connection,
                                 const gchar *,
                                 gpointer user_data)
{
    gchar* contents;
    GDBusNodeInfo *introspection_data;

    // Get Interface for introspection
    g_file_get_contents (FRONTEND_INTERFACE, &contents, nullptr, nullptr);
    introspection_data = g_dbus_node_info_new_for_xml (contents, nullptr);

    g_dbus_connection_register_object (connection,
                                       "/org/libreoffice/PrintDialog",
                                       introspection_data->interfaces[0],
                                       nullptr,
                                       nullptr,  /* user_data */
                                       nullptr,  /* user_data_free_func */
                                       nullptr); /* GError** */
    g_free(contents);
    g_dbus_node_info_unref(introspection_data);

    CPDManager* current = static_cast<CPDManager*>(user_data);
    std::vector<std::pair<std::string, std::string>> backends = current -> getTempBackends();
    for (std::vector<std::pair<std::string, std::string>>::iterator it = backends.begin(); it != backends.end(); ++it) {
        GDBusProxy *proxy;
        // Get Interface for introspection
        g_file_get_contents (BACKEND_INTERFACE, &contents, nullptr, nullptr);
        introspection_data = g_dbus_node_info_new_for_xml (contents, nullptr);
        proxy = g_dbus_proxy_new_sync (connection,
                                       G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                       introspection_data->interfaces[0],
                                       (*it).first.c_str(),
                                       (*it).second.c_str(),
                                       "org.openprinting.PrintBackend",
                                       nullptr,
                                       nullptr);
        g_dbus_proxy_call(proxy, "ActivateBackend",
                          nullptr,
                          G_DBUS_CALL_FLAGS_NONE,
                          -1, nullptr, nullptr, nullptr);

        g_free(contents);
        g_dbus_node_info_unref(introspection_data);
    }
}

void CPDManager::onNameLost (GDBusConnection *,
                             const gchar *name,
                             gpointer)
{
    g_message("Name Lost: %s", name);
}

void CPDManager::printerAdded (GDBusConnection *connection,
                               const gchar     *sender_name,
                               const gchar     *object_path,
                               const gchar     *interface_name,
                               const gchar     *signal_name,
                               GVariant        *parameters,
                               gpointer        user_data)
{
    CPDManager* current = static_cast<CPDManager*>(user_data);
    GDBusProxy *proxy;
    proxy = current -> getProxy(sender_name);
    if (proxy == nullptr) {
        gchar* contents;
        GDBusNodeInfo *introspection_data;

        // Get Interface for introspection
        g_file_get_contents ("/usr/share/dbus-1/interfaces/org.openprinting.Backend.xml", &contents, nullptr, nullptr);
        introspection_data = g_dbus_node_info_new_for_xml (contents, nullptr);
        proxy = g_dbus_proxy_new_sync (connection,
                                       G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                       introspection_data->interfaces[0],
                                       sender_name,
                                       object_path,
                                       interface_name,
                                       nullptr,
                                       nullptr);

        g_free(contents);
        g_dbus_node_info_unref(introspection_data);
        std::pair<std::string, GDBusProxy *> new_backend (sender_name, proxy);
        current -> addBackend(new_backend);
    }
    CPDPrinter *pDest = static_cast<CPDPrinter *>(malloc(sizeof(CPDPrinter)));
    pDest -> backend = proxy;
    g_variant_get (parameters, "(sssssbss)", &(pDest -> id), &(pDest -> name), &(pDest -> info), &(pDest -> location), &(pDest -> make_and_model), &(pDest -> is_accepting_jobs), &(pDest -> printer_state), &(pDest -> backend_name));
    std::stringstream printerName;
    printerName << pDest -> name << ", " << pDest -> backend_name;
    std::stringstream uniqueName;
    uniqueName << pDest -> id << ", " << pDest -> backend_name;
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OUString aPrinterName = OStringToOUString( printerName.str().c_str(), aEncoding );
    OUString aUniqueName = OStringToOUString( uniqueName.str().c_str(), aEncoding );
    current -> addNewPrinter(aPrinterName, aUniqueName, pDest);
}

void CPDManager::printerRemoved (GDBusConnection *,
                                 const gchar     *sender_name,
                                 const gchar     *object_path,
                                 const gchar     *,
                                 const gchar     *signal_name,
                                 GVariant        *,
                                 gpointer)
{
    // TODO: Remove every data linked to this particular printer.
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
}

void CPDManager::addTempBackend(std::pair<std::string, std::string> pair) {
    this -> m_tBackends.push_back(pair);
}

std::vector<std::pair<std::string, std::string>> CPDManager::getTempBackends() {
    return this -> m_tBackends;
}

void CPDManager::addNewPrinter(OUString aPrinterName, OUString aUniqueName, CPDPrinter *pDest) {
    std::pair<OUString, CPDPrinter *> newPrinter (aUniqueName, pDest);
    std::unordered_map<OUString, CPDPrinter *, OUStringHash>::iterator it = m_aCPDDestMap.find( aUniqueName );
    if (it == m_aCPDDestMap.end()) {
        m_aCPDDestMap.insert(newPrinter);
    } else {
        m_aCPDDestMap.erase(it);
        m_aCPDDestMap.insert(newPrinter);
    }
    bool bSetToGlobalDefaults = m_aPrinters.find( aUniqueName ) == m_aPrinters.end();
    Printer aPrinter = m_aPrinters[ aUniqueName ];
    if( bSetToGlobalDefaults )
        aPrinter.m_aInfo = m_aGlobalDefaults;
    aPrinter.m_aInfo.m_aPrinterName = aPrinterName;

    // TODO: I don't know how this should work when we have multiple
    // sources with multiple possible defaults for each
    // if( pDest->is_default )
    //     m_aDefaultPrinter = aPrinterName;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    aPrinter.m_aInfo.m_aComment = OStringToOUString(pDest -> info, aEncoding);
    aPrinter.m_aInfo.m_aLocation = OStringToOUString(pDest -> location, aEncoding);
    OUStringBuffer aBuf( 256 );
    aBuf.append( "CPD:" );
    aBuf.append( aUniqueName );
    // note: the parser that goes with the PrinterInfo
    // is created implicitly by the JobData::operator=()
    // when it detects the NULL ptr m_pParser.
    // if we wanted to fill in the parser here this
    // would mean we'd have to send a dbus message for each and
    // every printer - which would be really bad runtime
    // behaviour
    aPrinter.m_aInfo.m_pParser = nullptr;
    aPrinter.m_aInfo.m_aContext.setParser( nullptr );
    std::unordered_map< OUString, PPDContext, OUStringHash >::const_iterator c_it = m_aDefaultContexts.find( aPrinterName );
    if( c_it != m_aDefaultContexts.end() )
    {
        aPrinter.m_aInfo.m_pParser = c_it->second.getParser();
        aPrinter.m_aInfo.m_aContext = c_it->second;
    }
    aPrinter.m_aInfo.setDefaultBackend(true);
    aPrinter.m_aInfo.m_aDriverName = aBuf.makeStringAndClear();
    aPrinter.m_bModified = false;
    m_aPrinters[ aUniqueName ] = aPrinter;
}

/*
 *  CPDManager class
 */

CPDManager* CPDManager::tryLoadCPD()
{
    CPDManager* pManager = nullptr;
#if ENABLE_DBUS && ENABLE_GIO
    static const char* pEnv = getenv("SAL_DISABLE_CPD");

    if (!pEnv || !*pEnv) {
        GDir *dir;
        GError *error;
        const gchar *filename;
        dir = g_dir_open(BACKEND_DIR, 0, &error);
        if (error != nullptr) {
            while ((filename = g_dir_read_name(dir))) {
                if (pManager == nullptr) {
                    pManager = new CPDManager();
                }
                gchar* contents;
                std::stringstream filepath;
                filepath << BACKEND_DIR << '/' << filename;
                g_file_get_contents(filepath.str().c_str(), &contents, nullptr, nullptr);
                std::pair<std::string, std::string> new_tbackend (filename, contents);
                pManager -> addTempBackend(new_tbackend);
            }
            g_dir_close(dir);
        }
    }
#endif
    return pManager;
}

CPDManager::CPDManager() :
    PrinterInfoManager( PrinterInfoManager::Type::CPD )
{
    // Get Destinations number and pointers
    GError *error = nullptr;
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
    g_dbus_connection_flush_sync (m_pConnection,
                                  nullptr,
                                  nullptr);
    g_dbus_connection_close_sync (m_pConnection,
                                  nullptr,
                                  nullptr);
    std::unordered_map<std::string, GDBusProxy *>::iterator it = this -> m_pBackends.begin();
    for(; it != m_pBackends.end(); ++it)
    {
        g_object_unref(it->second);
    }
    std::unordered_map<OUString, CPDPrinter *, OUStringHash>::iterator dest_it =
        m_aCPDDestMap.begin();
    for(; it != m_pBackends.end(); ++it)
    {
        free(it->second);
    }
}


const PPDParser* CPDManager::createCPDParser( const OUString& rPrinter )
{
    const PPDParser* pNewParser = nullptr;
    OUString aPrinter;

    if( rPrinter.startsWith("CPD:") )
        aPrinter = rPrinter.copy( 4 );
    else
        aPrinter = rPrinter;


    if( ! pNewParser )
    {
        // get the default PPD
        pNewParser = PPDParser::getParser( "SGENPRT" );
        SAL_WARN("vcl.unx.print", "Parsing default SGENPRT PPD" );

        PrinterInfo& rInfo = m_aPrinters[ aPrinter ].m_aInfo;

        rInfo.m_pParser = pNewParser;
        rInfo.m_aContext.setParser( pNewParser );
    }

    return pNewParser;
}


void CPDManager::initialize()
{
    // get normal printers, clear printer list
    PrinterInfoManager::initialize();
    g_bus_own_name_on_connection (m_pConnection,
                                  "org.libreoffice.print-dialog",
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  onNameAcquired,
                                  onNameLost,
                                  this,
                                  nullptr);

    g_dbus_connection_signal_subscribe (m_pConnection,                    // DBus Connection
                                        nullptr,                          // Sender Name
                                        "org.openprinting.PrintBackend",  // Sender Interface
                                        "PrinterAdded",                   // Signal Name
                                        nullptr,                          // Object Path
                                        nullptr,                          // arg0 behaviour
                                        G_DBUS_SIGNAL_FLAGS_NONE,         // Signal Flags
                                        printerAdded,                     // Callback Function
                                        this,
                                        nullptr);
    g_dbus_connection_signal_subscribe (m_pConnection,                    // DBus Connection
                                        nullptr,                          // Sender Name
                                        "org.openprinting.PrintBackend",  // Sender Interface
                                        "PrinterRemoved",                 // Signal Name
                                        nullptr,                          // Object Path
                                        nullptr,                          // arg0 behaviour
                                        G_DBUS_SIGNAL_FLAGS_NONE,         // Signal Flags
                                        printerRemoved,                   // Callback Function
                                        this,
                                        nullptr);

    // remove everything that is not a CUPS printer and not
    // a special purpose printer (PDF, Fax)
    std::list< OUString > aRemovePrinters;
    for( std::unordered_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.begin();
         it != m_aPrinters.end(); ++it )
    {
        if( m_aCPDDestMap.find( it->first ) != m_aCPDDestMap.end() )
            continue;

        if( !it->second.m_aInfo.m_aFeatures.isEmpty() )
            continue;
        aRemovePrinters.push_back( it->first );
    }
    while( aRemovePrinters.begin() != aRemovePrinters.end() )
    {
        m_aPrinters.erase( aRemovePrinters.front() );
        aRemovePrinters.pop_front();
    }
    // If password CB is needed
    //cpdSetPasswordCB( setPasswordCallback );
}

void CPDManager::setupJobContextData( JobData& rData )
{
    std::unordered_map<OUString, CPDPrinter *, OUStringHash>::iterator dest_it =
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

    if( p_it->second.m_aInfo.m_pParser == nullptr )
    {
        // in turn calls createCPDParser
        // which updates the printer info
        p_it->second.m_aInfo.m_pParser = PPDParser::getParser( p_it->second.m_aInfo.m_aDriverName );
    }
    if( p_it->second.m_aInfo.m_aContext.getParser() == nullptr )
    {
        OUString aPrinter;
        if( p_it->second.m_aInfo.m_aDriverName.startsWith("CPD:") )
            aPrinter = p_it->second.m_aInfo.m_aDriverName.copy( 4 );
        else
            aPrinter = p_it->second.m_aInfo.m_aDriverName;

        p_it->second.m_aInfo.m_aContext = m_aDefaultContexts[ aPrinter ];
    }

    rData.m_pParser     = p_it->second.m_aInfo.m_pParser;
    rData.m_aContext    = p_it->second.m_aInfo.m_aContext;
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
    bool success = false;
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
        OString aSysFile = it->second;
        CPDPrinter* pDest = dest_it->second;
        GVariant* ret;
        GVariantBuilder *builder;
        gint job_id;
        builder = g_variant_builder_new(G_VARIANT_TYPE("a(ss)"));
        int num_options = 0;
        if( rDocumentJobData.m_nPDFDevice > 0 && rDocumentJobData.m_nCopies > 1 )
        {
            OString aVal( OString::number( rDocumentJobData.m_nCopies ) );
            g_variant_builder_add(builder, "(ss)", "copies", aVal.getStr());
            num_options++;
            aVal = OString::boolean(rDocumentJobData.m_bCollate);
            g_variant_builder_add(builder, "(ss)", "collate", aVal.getStr());
            num_options++;
        }
        if( ! bBanner )
        {
            g_variant_builder_add(builder, "(ss)", "job-sheets", "none");
            num_options++;
        }
        g_variant_builder_add(builder, "(ss)", "NA", "NA");

        GVariant *arr = g_variant_new("a(ss)", builder);
        ret = g_dbus_proxy_call_sync (pDest -> backend, "printFile",
                                      g_variant_new(
                                                    "(ssi@a(ss))",
                                                    (pDest -> id),
                                                    aSysFile.getStr(),
                                                    num_options,
                                                    arr
                                                    ),
                                      G_DBUS_CALL_FLAGS_NONE,
                                      -1, nullptr, nullptr);
        g_variant_builder_unref(builder);
        g_variant_get (ret, "(i)", &job_id);
        if (job_id != -1) {
            success = true;
        }
        g_variant_unref(ret);
        unlink( it->second.getStr() );
        m_aSpoolFiles.erase( pFile );
    }

    return success;
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

