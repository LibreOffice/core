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

#ifndef INCLUDED_VCL_PRINTERINFOMANAGER_HXX
#define INCLUDED_VCL_PRINTERINFOMANAGER_HXX

#include <list>
#include <unordered_map>

#include <vcl/dllapi.h>
#include <vcl/jobdata.hxx>
#include <osl/file.hxx>
#include <unx/helper.hxx>

#include <cstdio>

namespace psp
{

class SystemQueueInfo;

struct PrinterInfo : JobData
{
    // basename of PPD
    OUString             m_aDriverName;
    // can be the queue
    OUString             m_aLocation;
    // a user defined comment
    OUString             m_aComment;
    // a command line to pipe a PS-file to
    OUString             m_aCommand;
    // a command line to pipe a PS-file to in case of direct print
    OUString             m_aQuickCommand;
    // a list of special features separated by ',' not used by psprint
    // but assigned from the outside (currently for "fax","pdf=","autoqueue","external_dialog")
    OUString             m_aFeatures;
    bool                 m_bPapersizeFromSetup;

    PrinterInfo()
        : JobData()
        , m_bPapersizeFromSetup(false)
    {}
};

class VCL_DLLPUBLIC PrinterInfoManager
{
public:
    enum class Type { Default = 0, CUPS = 1 };

    struct SystemPrintQueue
    {
        OUString       m_aQueue;
        OUString       m_aLocation;
        OUString       m_aComment;
    };
protected:
    // needed for checkPrintersChanged: files (not necessarily existent)
    // and their last known modification time
    struct WatchFile
    {
        // the file in question
        OUString         m_aFilePath;
        // the last know modification time or 0, if file did not exist
        TimeValue               m_aModified;
    };

    // internal data to describe a printer
    struct Printer
    {
        // configuration file containing this printer
        // empty means a freshly added printer that has to be saved yet
        OUString         m_aFile;
        // details other config files that have this printer
        // in case of removal all have to be removed
        std::list< OUString > m_aAlternateFiles;
        // group in m_aFile containing the printer
        // this must be unique over all configuration files
        // it usually should be the printer name
        OString          m_aGroup;
        // whether changes need to be saved
        bool                    m_bModified;
        // the corresponding info and job data
        PrinterInfo             m_aInfo;
    };

    std::unordered_map< OUString, Printer, OUStringHash > m_aPrinters;
    PrinterInfo                         m_aGlobalDefaults;
    std::list< WatchFile >            m_aWatchFiles;
    OUString                     m_aDefaultPrinter;
    OUString                     m_aSystemPrintCommand;

    std::list< SystemPrintQueue >     m_aSystemPrintQueues;

    std::unique_ptr<SystemQueueInfo>
                                      m_pQueueInfo;

    Type                              m_eType;
    bool                              m_bUseIncludeFeature;
    bool                              m_bUseJobPatch;
    OUString                     m_aSystemDefaultPaper;

    PrinterInfoManager( Type eType = Type::Default );

    virtual void initialize();

    // fill default paper if not configured in config file
    // default paper is e.g. locale dependent
    // if a paper is already set it will not be overwritten
    void setDefaultPaper( PPDContext& rInfo ) const;

public:

    // there can only be one
    static PrinterInfoManager& get();
    // only called by SalData destructor, frees the global instance
    static void release();

    // get PrinterInfoManager type
    Type getType() const { return m_eType; }

    // lists the names of all known printers
    void listPrinters( std::list< OUString >& rList ) const;

    // gets info about a named printer
    const PrinterInfo& getPrinterInfo( const OUString& rPrinter ) const;

    // gets the name of the default printer
    const OUString& getDefaultPrinter() const { return m_aDefaultPrinter; }

    virtual void setupJobContextData( JobData& rData );

    // check if the printer configuration has changed
    // if bwait is true, then this method waits for eventual asynchronous
    // printer discovery to finish
    virtual bool checkPrintersChanged( bool bWait );

    // members for administration

    // add a named printer
    // addPrinter fails if a printer with the same name already exists
    // or the driver does not exist
    virtual bool addPrinter( const OUString& rPrinterName, const OUString& rDriverName );

    // remove a named printer
    // this fails if the config file belonging to this printer
    // is not writeable
    // if bCheckOnly is true, the printer is not really removed;
    // this is for checking if the removal would fail
    virtual bool removePrinter( const OUString& rPrinterName, bool bCheckOnly );

    // save the changes to all printers. this fails if there
    // is no writable config file at all
    virtual bool writePrinterConfig();

    // set a new default printer
    // fails if the specified printer does not exist
    virtual bool setDefaultPrinter( const OUString& rPrinterName );

    // abstract print command
    // returns a stdio FILE* that a postscript file may be written to
    // this may either be a regular file or the result of popen()
    virtual FILE* startSpool( const OUString& rPrinterName, bool bQuickCommand );
    // close the FILE* returned by startSpool and does the actual spooling
    // set bBanner to "false" will attempt to suppress banner printing
    // set bBanner to "true" will rely on the system default
    // returns true on success
    virtual bool endSpool( const OUString& rPrinterName, const OUString& rJobTitle, FILE* pFile, const JobData& rDocumentJobData, bool bBanner, const OUString &rFaxNumber );

    bool getUseIncludeFeature() const { return m_bUseIncludeFeature; }
    bool getUseJobPatch() const { return m_bUseJobPatch; }

    // check whether a printer's feature string contains a subfeature
    bool checkFeatureToken( const OUString& rPrinterName, const char* pToken ) const;

    virtual ~PrinterInfoManager();
};

} // namespace

#endif // INCLUDED_VCL_PRINTERINFOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
