/*************************************************************************
 *
 *  $RCSfile: salprn.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cp $ $Date: 2000-11-17 18:35:17 $
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#ifdef USE_PSPRINT

#ifndef _PSPRINT_JOBDATA_HXX_
#include <psprint/jobdata.hxx>
#endif
#ifndef _PSPRINT_PRINTERGFX_HXX_
#include <psprint/printergfx.hxx>
#endif
#ifndef _PSPRINT_PRINTERJOB_HXX_
#include <psprint/printerjob.hxx>
#endif

class SalGraphics;

struct SalInfoPrinterData
{
    SalGraphics*            m_pGraphics;
    ::psp::JobData          m_aJobData;
    ::psp::PrinterGfx       m_aPrinterGfx;
};

struct SalPrinterData
{
    String                  m_aFileName;
    String                  m_aFaxNr;
    bool                    m_bFax;
    SalGraphics*            m_pGraphics;
    ::psp::PrinterJob       m_aPrintJob;
    ::psp::JobData          m_aJobData;
    ::psp::PrinterGfx       m_aPrinterGfx;
};


#else

#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif

#ifndef _SV_PRNTYPES_HXX
#include <prntypes.hxx>
#endif

#include <ppdparser.hxx>

// forward declarations
class SalInfoPrinter;

class SalPrinterQueueInfo;
class ImplJobSetup;

class ImplSalPrinterData;


// class declarations


class SalInfoPrinterData
{
  friend class SalInfoPrinter;
  friend class SalPrinterData;

  ImplSalPrinterData*   mpImplData;

  inline        SalInfoPrinterData();
  inline        ~SalInfoPrinterData();

public:

  inline void   Init( SalPrinterQueueInfo*  pQueueInfo,
                      ImplJobSetup*         pJobSetup);
};


// -=-= SalPrinterData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class SalPrinterData
{
    friend class SalPrinter;

    ImplSalPrinterData* mpImplData;

    BOOL                    mbJobStarted;       // is job started
    BOOL                    mbPageStarted;      // is page started
    String                  maFileName;         // print to file
    int                     mnError;            // error code
    ImplJobSetup*           mpJobSetup;         // job setup
    BOOL                    mbAbort:1;          // is job aborted
    BOOL                    mbFirstPage:1;      // false after first startpage

    inline      SalPrinterData();
    inline      ~SalPrinterData();

public:
    inline void Init( SalInfoPrinter*   pInfoPrinter );
};


// necessary to get changes in Xpdefaults
void StartPrinterListening();
void StopPrinterListening();

#endif

#endif // _SV_SALPRN_H


