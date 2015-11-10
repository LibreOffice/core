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

#ifndef INCLUDED_VCL_JOBDATA_HXX
#define INCLUDED_VCL_JOBDATA_HXX

#include <vcl/ppdparser.hxx>

namespace psp {

namespace orientation {
enum type {
    Portrait,
    Landscape
};
}

struct VCL_DLLPUBLIC JobData
{
    int                     m_nCopies;
    bool                    m_bCollate;
    int                     m_nLeftMarginAdjust;
    int                     m_nRightMarginAdjust;
    int                     m_nTopMarginAdjust;
    int                     m_nBottomMarginAdjust;
    // user overrides for PPD
    int                     m_nColorDepth;
    int                     m_nPSLevel;     // 0: no override, else languagelevel to use
    int                     m_nColorDevice; // 0: no override, -1 grey scale, +1 color
    int                     m_nPDFDevice;   // 0: no override, -1 PostScript, +1: Automatically PDF, +2: Explicitly PDF
    orientation::type       m_eOrientation;
    OUString         m_aPrinterName;
    const PPDParser*        m_pParser;
    PPDContext              m_aContext;

    JobData() :
            m_nCopies( 1 ),
            m_bCollate(false),
            m_nLeftMarginAdjust( 0 ),
            m_nRightMarginAdjust( 0 ),
            m_nTopMarginAdjust( 0 ),
            m_nBottomMarginAdjust( 0 ),
            m_nColorDepth( 24 ),
            m_nPSLevel( 0 ),
            m_nColorDevice( 0 ),
            m_nPDFDevice( 0 ),
            m_eOrientation( orientation::Portrait ),
            m_pParser( nullptr ) {}

    JobData& operator=(const psp::JobData& rRight);

    JobData( const JobData& rData ) { *this = rData; }

    void setCollate( bool bCollate );
    bool setPaper( int nWidth, int nHeight ); // dimensions in pt
    bool setPaperBin( int nPaperBin );
    void resolveDefaultBackend();
    void setDefaultBackend(bool bUsePDF);

    // creates a new buffer using new
    // it is up to the user to delete it again
    bool getStreamBuffer( void*& pData, int& bytes );
    static bool constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData );
};

bool operator==(const psp::JobData& rLeft, const psp::JobData& rRight);
inline bool operator!=(const psp::JobData& rLeft, const psp::JobData& rRight)
{
    return !( rLeft == rRight );
}

} // namespace


#endif // PSPRINT_JOBDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
