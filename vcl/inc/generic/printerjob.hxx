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

#ifndef INCLUDED_VCL_INC_GENERIC_PRINTERJOB_HXX
#define INCLUDED_VCL_INC_GENERIC_PRINTERJOB_HXX

#include "vcl/jobdata.hxx"
#include "osl/file.hxx"
#include "rtl/string.hxx"

#include <list>

namespace psp {

class PrinterGfx;

class PrinterJob
{
private:
    OUString           maSpoolDirName;
    OUString           maFileName; // empty: spool to command, else spool to named file
    OUString           maJobTitle;
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

    int mnLandscapes;
    int mnPortraits;

    sal_uInt32      mnLMarginPt;
    sal_uInt32      mnRMarginPt;
    sal_uInt32      mnTMarginPt;
    sal_uInt32      mnBMarginPt;

    double          mfXScale;
    double          mfYScale;

    bool            m_bQuickJob;

private:
    osl::File*      CreateSpoolFile (const OUString& rName,
                                     const OUString& rExtension);
    void            InitPaperSize (const JobData& rJobSetup);

    bool            writeFeatureList( osl::File* pFile, const JobData&, bool bDocumentSetup );
    bool            writeSetup( osl::File* pFile, const JobData& );
    bool            writePageSetup( osl::File* pFile, const JobData&, bool bWriteFeatures = true );
    static void     writeJobPatch( osl::File* File, const JobData& );
    static bool     writeProlog (osl::File* pFile, const JobData& );

public:             // for usage in PrinterGfx
    sal_uInt32      GetResolution () const { return mnResolution; }
    void            GetScale (double &rXScale, double &rYScale) const;
    sal_uInt16      GetDepth () const;
    sal_uInt16      GetPostscriptLevel (const JobData *pJobData = nullptr) const;
    bool        IsColorPrinter () const;

    osl::File*      GetCurrentPageHeader ();
    osl::File*      GetCurrentPageBody ();

    const OUString& GetPrinterName() const { return m_aLastJobData.m_aPrinterName; }

public:
    PrinterJob ();
    ~PrinterJob ();

    /*  rFileName: if length is greater than 0 save resulting PostScript
     *  to named file.
     *  nMode: only meaningful when saving to file: if nonzero, try
     *  to impose the mode on the resulting file's inode; for nonexistent
     *  files use open, for existent files try a chmod
     *  rJobName: text to appear in the %%Title comment
     *  rAppName: text to appear in the %%Creator comment
     *  rSetupData: JobData that apply to this job
     *  pGraphics: the graphics used to print this job;
     *             this graphics must live until EndJob() has returned
     *  bIsQuickJob: the job was started as "direct print" meaning
     *               the quick command for spooling should be used instead
     *               of the normal command
     */
    bool        StartJob (const OUString& rFileName,
                              int nMode,
                              const OUString& rJobName,
                              const OUString& rAppName,
                              const JobData& rSetupData,
                              PrinterGfx* pGraphics,
                              bool bIsQuickJob
                              );
    bool        EndJob ();

    bool        StartPage (const JobData& rJobSetup);
    bool        EndPage ();
};

}  // namespace psp

#endif // INCLUDED_VCL_INC_GENERIC_PRINTERJOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
