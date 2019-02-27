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

#ifndef INCLUDED_VCL_INC_UNX_CPDMGR_HXX
#define INCLUDED_VCL_INC_UNX_CPDMGR_HXX

#include <config_dbus.h>
#include <config_gio.h>

#if ENABLE_DBUS && ENABLE_GIO
#include <gio/gio.h>
#else
typedef struct _GDBusProxy GDBusProxy;
typedef struct _GDBusConnection GDBusConnection;
#endif

#include <printerinfomanager.hxx>
#include "cupsmgr.hxx"

#define BACKEND_DIR "/usr/share/print-backends"
#define FRONTEND_INTERFACE "/usr/share/dbus-1/interfaces/org.openprinting.Frontend.xml"
#define BACKEND_INTERFACE "/usr/share/dbus-1/interfaces/org.openprinting.Backend.xml"

namespace psp
{

class PPDParser;

struct CPDPrinter
{
    const char* id;
    const char* name;
    const char* info;
    const char* location;
    const char* make_and_model;
    const char* printer_state;
    const char* backend_name;
    bool is_accepting_jobs;
    GDBusProxy* backend;
};

class CPDManager : public PrinterInfoManager
{
#if ENABLE_DBUS && ENABLE_GIO
    GDBusConnection * m_pConnection = nullptr;
    bool m_aPrintersChanged = true;
    std::vector<std::pair<std::string, gchar*>> m_tBackends;
    std::unordered_map< std::string, GDBusProxy * > m_pBackends;
    std::unordered_map< FILE*, OString, FPtrHash > m_aSpoolFiles;
    std::unordered_map< OUString, CPDPrinter * > m_aCPDDestMap;
    std::unordered_map< OUString, PPDContext > m_aDefaultContexts;
#endif
    CPDManager();
    // Function called when CPDManager is destroyed
    virtual ~CPDManager() override;

    virtual void initialize() override;

#if ENABLE_DBUS && ENABLE_GIO
    static void onNameAcquired(GDBusConnection *connection, const gchar* name, gpointer user_data);
    static void onNameLost (GDBusConnection *, const gchar *name, gpointer);
    static void printerAdded (GDBusConnection *connection,
                              const gchar     *sender_name,
                              const gchar     *object_path,
                              const gchar     *interface_name,
                              const gchar     *signal_name,
                              GVariant        *parameters,
                              gpointer        user_data);
    static void printerRemoved (GDBusConnection *connection,
                                const gchar     *sender_name,
                                const gchar     *object_path,
                                const gchar     *interface_name,
                                const gchar     *signal_name,
                                GVariant        *parameters,
                                gpointer        user_data);

    static void getOptionsFromDocumentSetup( const JobData& rJob, bool bBanner, const OString& rJobName, int& rNumOptions, GVariant **arr );
#endif

public:
#if ENABLE_DBUS && ENABLE_GIO
    // Functions involved in initialization
    GDBusProxy* getProxy(const std::string& target);
    void addBackend( std::pair< std::string, GDBusProxy * > pair );
    void addTempBackend(const std::pair<std::string, gchar*>& pair);
    std::vector<std::pair<std::string, gchar*>> const & getTempBackends();
    void addNewPrinter( const OUString&, const OUString&, CPDPrinter * );
#endif

    // Create CPDManager
    static CPDManager* tryLoadCPD();

    // Create a PPDParser for CPD Printers
    const PPDParser* createCPDParser( const OUString& rPrinter );

    // Functions related to printing
    virtual FILE* startSpool( const OUString& rPrinterName, bool bQuickCommand ) override;
    virtual bool endSpool( const OUString& rPrinterName, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString& rFaxNumber ) override;
    virtual void setupJobContextData( JobData& rData ) override;

    // check if the printer configuration has changed
    virtual bool checkPrintersChanged( bool bWait ) override;
};

} // namespace psp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

