/*************************************************************************
 *
 *  $RCSfile: printerjob.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 14:24:02 $
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

#ifndef _PSPRINT_PRINTERJOB_HXX_
#define _PSPRINT_PRINTERJOB_HXX_

#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _PSPRINT_JOBDATA_HXX_
#include <psprint/jobdata.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

namespace psp {

// forward declarations
class PrinterGfx;


class PrinterJob
{
private:            // private data

    rtl::OUString           maSpoolDirName;
    rtl::OUString           maFileName; // empty: spool to command, else spool to named file
    int                     mnFileMode;

    osl::File*              mpJobHeader;
    osl::File*              mpJobTrailer;

    std::list< osl::File* > maPageList;
    std::list< osl::File* > maHeaderList;

    JobData                 m_aDocumentJobData;
    JobData                 m_aLastJobData;
    PrinterGfx*             m_pGraphics;

    sal_uInt32      mnResolution;

    sal_uInt32      mnWidthPt;
    sal_uInt32      mnHeightPt;
    sal_uInt32      mnMaxWidthPt;
    sal_uInt32      mnMaxHeightPt;

    sal_uInt32      mnLMarginPt;
    sal_uInt32      mnRMarginPt;
    sal_uInt32      mnTMarginPt;
    sal_uInt32      mnBMarginPt;

    double          mfXScale;
    double          mfYScale;

    sal_Int32       mnErrorCode;

private:            // private methods

    osl::File*      CreateSpoolFile (const rtl::OUString& rName,
                                     const rtl::OUString& rExtension);
    void            InitPaperSize (const JobData& rJobSetup);

    bool            writeFeatureList( osl::File* pFile, const JobData&, bool bDocumentSetup );
    bool            writeSetup( osl::File* pFile, const JobData& );
    bool            writePageSetup( osl::File* pFile, const JobData& );
    bool            writeProlog (osl::File* pFile);

public:             // for usage in PrinterGfx

    sal_uInt32      GetResolution () const { return mnResolution; }
    void            GetScale (double &rXScale, double &rYScale) const;
    sal_uInt16      GetDepth () const;
    sal_uInt16      GetPostscriptLevel (const JobData *pJobData = NULL) const;
    sal_Bool        IsColorPrinter () const;

    osl::File*      GetDocumentHeader ();
    osl::File*      GetDocumentTrailer ();
    osl::File*      GetCurrentPageHeader ();
    osl::File*      GetCurrentPageBody ();

    const ::rtl::OUString& GetPrinterName() const { return m_aLastJobData.m_aPrinterName; }

public:
    PrinterJob ();
    ~PrinterJob ();

    /*  rFileName: if length is greater than 0 save resulting PostScript
     *  to named file.
     *  nMode: only meaningful when saving to file: if nonzero, try
     *  to impose the mode on the resulting file's inode; for nonexistant
     *  files use open, for existant files try a chmod
     *  rJobName: text to appear in the %%Title comment
     *  rAppName: text to appear in the %%Creator comment
     *  rSetupData: JobData that apply to this job
     *  pGraphics: the graphics used to print this job;
     *             this graphics must live until End/AbortJob has returned
     */
    sal_Bool        StartJob (const rtl::OUString& rFileName,
                              int nMode,
                              const rtl::OUString& rJobName,
                              const rtl::OUString& rAppName,
                              const JobData& rSetupData,
                              PrinterGfx* pGraphics
                              );
    sal_Bool        EndJob ();

    sal_Bool        AbortJob ();

    sal_Bool        StartPage (const JobData& rJobSetup, sal_Bool bNewJobData);
    sal_Bool        EndPage ();

    sal_uInt32      GetErrorCode ();
};

}  /* namespace psp */

#endif /* _PSPRINT_PRINTERJOB_HXX_ */

