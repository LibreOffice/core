/*************************************************************************
 *
 *  $RCSfile: cupsmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:07:09 $
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

#ifndef _PSPRINT_CUPSMGR_HXX_
#define _PSPRINT_CUPSMGR_HXX_

#include <psprint/printerinfomanager.hxx>
#include <osl/module.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>

namespace psp
{

class CUPSWrapper;
class PPDParser;

struct FPtrHash
{
    size_t operator()(const FILE* pPtr) const
    { return (size_t)pPtr; }
};

class CUPSManager : public PrinterInfoManager
{
    CUPSWrapper*                                                m_pCUPSWrapper;
    std::hash_map< FILE*, rtl::OString, FPtrHash >              m_aSpoolFiles;
    int                                                         m_nDests;
    void*                                                       m_pDests;
    bool                                                        m_bNewDests;
    std::hash_map< rtl::OUString, int, rtl::OUStringHash >      m_aCUPSDestMap;

    rtl::OString                                                m_aUser;
    // this is a security risk, but the CUPS API demands
    // to deliver a pointer to a static buffer containing
    // the password, so this cannot be helped
    rtl::OString                                                m_aPassword;

    osl::Mutex                                                  m_aCUPSMutex;
    oslThread                                                   m_aDestThread;

    CUPSManager( CUPSWrapper* );
    virtual ~CUPSManager();

    virtual void initialize();

    void runDests();
    static void runDestThread(void* pMgr);
public:

    static CUPSManager* tryLoadCUPS();

    const PPDParser* createCUPSParser( const rtl::OUString& rPrinter );
    // wraps cupsGetPPD, so unlink after use !

    const char* authenticateUser( const char* );

    virtual FILE* startSpool( const rtl::OUString& rPrinterName );
    virtual int endSpool( const rtl::OUString& rPrinterName, const rtl::OUString& rJobTitle, FILE* pFile );
    virtual void setupJobContextData( JobData& rData );

    // changes the info about a named printer
    virtual void changePrinterInfo( const ::rtl::OUString& rPrinter, const PrinterInfo& rNewInfo );

    // check if the printer configuration has changed
    virtual bool checkPrintersChanged();

    // members for administration (->padmin)
    // disable for CUPS
    virtual bool addPrinter( const rtl::OUString& rPrinterName, const ::rtl::OUString& rDriverName );
    virtual bool removePrinter( const rtl::OUString& rPrinterName, bool bCheckOnly = false );
    virtual bool writePrinterConfig();
    virtual bool setDefaultPrinter( const rtl::OUString& rPrinterName );
};

} // namespace psp

#endif
