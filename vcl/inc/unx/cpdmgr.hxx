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
#endif

#include "printerinfomanager.hxx"
#include "cupsmgr.hxx"

namespace psp
{

struct CPDPrinter
{
    std::string name;
    std::string info;
    std::string location;
    std::string make_and_model;
    std::string printer_state;
    bool is_accepting_jobs;
    GDBusProxy* backend;
};

class CPDManager : public PrinterInfoManager
{
#if ENABLE_DBUS && ENABLE_GIO
    GDBusConnection * m_pConnection = nullptr;
    bool m_aPrintersChanged = true;
    std::unordered_map< std::string, GDBusProxy * > m_pBackends;
    std::unordered_map< FILE*, OString, FPtrHash > m_aSpoolFiles;
    std::unordered_map< OUString, CPDPrinter *, OUStringHash > m_aCPDDestMap;
#endif
    CPDManager();
    virtual void initialize() override;

public:
    GDBusProxy * getProxy( std::string target );
    void addBackend( std::pair< std::string, GDBusProxy * > pair );
    void addDestination( std::pair< OUString, CPDPrinter * > pair );
    static CPDManager* tryLoadCPD();
    virtual ~CPDManager() override;
    virtual void setupJobContextData( JobData& rData ) override;
    /// check if the printer configuration has changed
    virtual bool checkPrintersChanged( bool bWait ) override;
    // members for administration
    // disable for CUPS
    virtual bool addPrinter( const OUString& rPrinterName, const OUString& rDriverName ) override;
    virtual bool removePrinter( const OUString& rPrinterName, bool bCheckOnly ) override;
    virtual bool writePrinterConfig() override;
    virtual bool setDefaultPrinter( const OUString& rPrinterName ) override;

    virtual FILE* startSpool( const OUString& rPrinterName, bool bQuickCommand ) override;
    virtual bool endSpool( const OUString& rPrinterName, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString& rFaxNumber ) override;


};

} // namespace psp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

