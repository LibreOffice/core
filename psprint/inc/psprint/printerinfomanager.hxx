/*************************************************************************
 *
 *  $RCSfile: printerinfomanager.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 09:58:25 $
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

#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#define _PSPRINT_PRINTERINFOMANAGER_HXX_

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _PSPRINT_HELPER_HXX_
#include <psprint/helper.hxx>
#endif
#ifndef _PSPRINT_JOBDATA_HXX_
#include <psprint/jobdata.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _PSPRINT_HELPER_HXX_
#include <psprint/helper.hxx>
#endif
#include <cstdio>

#ifdef MACOSX
/* MacOS X print system discovery constants:
 *
 * These values enumerate Apple OS printing systems we could use.
 * They are used as indications _as well as_ indexes into the
 * aParms structure used below for print commands.
 */
#define     kApplePrintingFailure       0       /* For whatever reason, we can't print at all */
#define     kApplePrintingLPR           1       /* Darwin 5.x style LPR printing */
#define     kApplePrintingCUPS          2       /* MacOS X 10.2/Darwin 6 style CUPS (also CUPS installed by users on 10.1) */
#define     kApplePrintingPrintCenter   3       /* MacOS X 10.1 /usr/sbin/Print printing */

/* Constants for PostScript vs PDF printing */
#define     kApplePrintingUsePDF        1       /* Default; Run PS through ps2pdf first (requires GhostScript) */
#define     kApplePrintingUsePS         2       /* Optional, print PS directly to printer (less compatible, requires PS printer) */

/* Define various printing commands */
#define     kApplePCPrintCommand    "/usr/sbin/Print"       /* Mac OS X 10.1 Print Center Printing command */
#define     kApplePS2PDFLocation    "/usr/local/bin/ps2pdf" /* PS -> PDF conversion command */
#define     kApplePCQueueName       "Apple Print Center Default Printer"    /* Name that appears in Print... dialog as the default */
                                                                /* printer for 10.1 Print Center printing */

/* Prototype for print method discovery function.  Returns
 * a constant defined in printerinfomanager.hxx
 */
sal_Int32   macxp_GetSystemPrintMethod( void );
#endif


namespace psp
{

class SystemQueueInfo;

struct PrinterInfo : JobData
{
    // basename of PPD
    rtl::OUString             m_aDriverName;
    // can be the queue
    rtl::OUString             m_aLocation;
    // a user defined comment
    rtl::OUString             m_aComment;
    // a command line to pipe a PS-file to
    rtl::OUString             m_aCommand;
    // a list of special features separated by ',' not used by psprint
    // but assigned from the outside (currently only for "fax")
    rtl::OUString             m_aFeatures;
    // a mapping of fonts to other fonts.
    // this provides a method for the user
    // to replace arbitrary fonts by printer builtin fonts
    // currently this is only a mapping between font names
    // assuming that only adbobe standard encoding fonts are
    // built into the printer. in future it may be necessary
    // to map to a font name and UCS2 vector which should be mapped
    // this vector is currently implicitly given by the adobe
    // standard encoding
    bool                        m_bPerformFontSubstitution;
    std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >
    m_aFontSubstitutes;
    std::hash_map< fontID, fontID >
    m_aFontSubstitutions;

    PrinterInfo() :
            JobData(),
            m_bPerformFontSubstitution( false )
    {}
};

class PrinterInfoManager
{
public:
    enum Type { Default = 0, CUPS = 1 };

protected:
    // needed for checkPrintersChanged: files (not necessarily existant)
    // and their last known modification time
    struct WatchFile
    {
        // the file in question
        rtl::OUString         m_aFilePath;
        // the last know modification time or 0, if file did not exist
        TimeValue               m_aModified;
    };

    // internal data to describe a printer
    struct Printer
    {
        // configuration file containing this printer
        // empty means a freshly added printer that has to be saved yet
        rtl::OUString         m_aFile;
        // details other config files that have this printer
        // in case of removal all have to be removed
        std::list< rtl::OUString > m_aAlternateFiles;
        // group in m_aFile containing the printer
        // this must be unique over all configuration files
        // it usually should be the printer name
        rtl::OString          m_aGroup;
        // whether changes need to be saved
        bool                    m_bModified;
        // the corresponding info and job data
        PrinterInfo             m_aInfo;
    };

    std::hash_map< rtl::OUString, Printer, rtl::OUStringHash > m_aPrinters;
    PrinterInfo                         m_aGlobalDefaults;
    std::list< WatchFile >            m_aWatchFiles;
    rtl::OUString                     m_aDefaultPrinter;
    rtl::OUString                     m_aSystemPrintCommand;
    std::list< rtl::OUString >      m_aSystemPrintQueues;

    SystemQueueInfo*                    m_pQueueInfo;

    Type                                m_eType;

    PrinterInfoManager( Type eType = Default );
    virtual ~PrinterInfoManager();

    virtual void initialize();

    // fill in font substitutions
    // the resulting hash_map maps from source to target font ids
    void fillFontSubstitutions( PrinterInfo& rInfo ) const;
public:

    // there can only be one
    static PrinterInfoManager& get();

    // get PrinterInfoManager type
    Type getType() const { return m_eType; }

    // lists the names of all known printers
    void listPrinters( std::list< rtl::OUString >& rList ) const;

    // gets the number of known printers
    int countPrinters() const { return m_aPrinters.size(); }

    // gets info about a named printer
    const PrinterInfo& getPrinterInfo( const rtl::OUString& rPrinter ) const;

    // gets the name of the default printer
    const rtl::OUString& getDefaultPrinter() const { return m_aDefaultPrinter; }

    virtual void setupJobContextData( JobData& rData );

    // changes the info about a named printer
    virtual void changePrinterInfo( const rtl::OUString& rPrinter, const PrinterInfo& rNewInfo );

    // check if the printer configuration has changed
    virtual bool checkPrintersChanged();

    // members for administration (->padmin)

    // add a named printer
    // addPrinter fails if a printer with the same name already exists
    // or the driver does not exist
    virtual bool addPrinter( const rtl::OUString& rPrinterName, const rtl::OUString& rDriverName );

    // remove a named printer
    // this fails if the config file belonging to this printer
    // is not writeable
    // if bCheckOnly is true, the printer is not really removed;
    // this is for checking if the removal would fail
    virtual bool removePrinter( const rtl::OUString& rPrinterName, bool bCheckOnly = false );

    // save the changes to all printers. this fails if there
    // is no writable config file at all
    virtual bool writePrinterConfig();

    // set a new default printer
    // fails if the specified printer does not exist
    virtual bool setDefaultPrinter( const rtl::OUString& rPrinterName );

    // primarily used internally but also by padmin
    // returns the printer queue names
    virtual const std::list< rtl::OUString >& getSystemPrintQueues();

    // similar but returnse whole commandlines
    virtual void getSystemPrintCommands( std::list< rtl::OUString >& rCommands );

    // abstract print command
    // returns a stdio FILE* that a postscript file may be written to
    // this may either be a regular file or the result of popen()
    virtual FILE* startSpool( const rtl::OUString& rPrinterName );
    // close the FILE* returned by startSpool and does the actual spooling
    // returns a numerical job id
    virtual int endSpool( const rtl::OUString& rPrinterName, const rtl::OUString& rJobTitle, FILE* pFile );
};

} // namespace

#endif // _PSPRINT_PRINTERINFOMANAGER_HXX_
