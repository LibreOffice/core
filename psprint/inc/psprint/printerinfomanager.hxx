/*************************************************************************
 *
 *  $RCSfile: printerinfomanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-06-15 11:06:33 $
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

namespace psp
{
struct PrinterInfo : JobData
{
    // basename of PPD
    ::rtl::OUString             m_aDriverName;
    // can be the queue
    ::rtl::OUString             m_aLocation;
    // a user defined comment
    ::rtl::OUString             m_aComment;
    // a command line to pipe a PS-file to
    ::rtl::OUString             m_aCommand;
    // a list of special features separated by ',' not used by psprint
    // but assigned from the outside (currently only for "fax")
    ::rtl::OUString             m_aFeatures;
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
    ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >
    m_aFontSubstitutes;
    ::std::hash_map< fontID, fontID >
    m_aFontSubstitutions;
};

class PrinterInfoManager
{
    // needed for checkPrintersChanged: files (not necessarily existant)
    // and their last known modification time
    struct WatchFile
    {
        // the file in question
        ::rtl::OUString         m_aFilePath;
        // the last know modification time or 0, if file did not exist
        TimeValue               m_aModified;
    };

    // internal data to describe a printer
    struct Printer
    {
        // configuration file containing this printer
        // empty means a freshly added printer that has to be saved yet
        ::rtl::OUString         m_aFile;
        // group in m_aFile containing the printer
        // this must be unique over all configuration files
        // it usually should be the printer name
        ::rtl::OString          m_aGroup;
        // wether changes need to be saved
        bool                    m_bModified;
        // the corresponding info and job data
        PrinterInfo             m_aInfo;
    };

    ::std::hash_map< ::rtl::OUString, Printer, ::rtl::OUStringHash > m_aPrinters;
    PrinterInfo                         m_aGlobalDefaults;
    ::std::list< WatchFile >            m_aWatchFiles;
    ::rtl::OUString                     m_aDefaultPrinter;
    ::rtl::OUString                     m_aSystemPrintCommand;
    ::std::list< ::rtl::OUString >      m_aSystemPrintQueues;

    PrinterInfoManager();
    ~PrinterInfoManager();

    void initialize();

    // fill in font substitutions
    // the resulting hash_map maps from source to target font ids
    void fillFontSubstitutions( PrinterInfo& rInfo ) const;
public:

    // there can only be one
    static PrinterInfoManager& get();

    // lists the names of all known printers
    void listPrinters( ::std::list< ::rtl::OUString >& rList ) const;

    // gets the number of known printers
    int countPrinters() const { return m_aPrinters.size(); }

    // gets info about a named printer
    const PrinterInfo& getPrinterInfo( const ::rtl::OUString& rPrinter ) const;

    // gets the name of the default printer
    const ::rtl::OUString& getDefaultPrinter() const { return m_aDefaultPrinter; }

    // changes the info about a named printer
    void changePrinterInfo( const ::rtl::OUString& rPrinter, const PrinterInfo& rNewInfo );

    // check if the printer configuration has changed
    bool checkPrintersChanged();

    // members for administration (->padmin)

    // add a named printer
    // addPrinter fails if a printer with the same name already exists
    // or the driver does not exist
    bool addPrinter( const ::rtl::OUString& rPrinterName, const ::rtl::OUString& rDriverName );

    // remove a named printer
    // this fails if the config file belonging to this printer
    // is not writeable
    // if bCheckOnly is true, the printer is not really removed;
    // this is for checking if the removal would fail
    bool removePrinter( const ::rtl::OUString& rPrinterName, bool bCheckOnly = false );

    // save the changes to all printers. this fails if there
    // is no writable config file at all
    bool writePrinterConfig();

    // set a new default printer
    // fails if the specified printer does not exist
    bool setDefaultPrinter( const ::rtl::OUString& rPrinterName );

    // primarily used internally but also by padmin
    // returns the printer queue names
    const ::std::list< ::rtl::OUString >& getSystemPrintQueues();

    // similar but returnse whole commandlines
    void getSystemPrintCommands( ::std::list< ::rtl::OUString >& rCommands );
};

} // namespace

#endif // _PSPRINT_PRINTERINFOMANAGER_HXX_
