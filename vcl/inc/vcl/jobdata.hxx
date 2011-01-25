/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _PSPRINT_JOBDATA_HXX_
#define _PSPRINT_JOBDATA_HXX_

#include "vcl/ppdparser.hxx"

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
    int                     m_nLeftMarginAdjust;
    int                     m_nRightMarginAdjust;
    int                     m_nTopMarginAdjust;
    int                     m_nBottomMarginAdjust;
    // user overrides for PPD
    int                     m_nColorDepth;
    int                     m_nPSLevel;     // 0: no override, else languaglevel to use
    int                     m_nColorDevice; // 0: no override, -1 grey scale, +1 color
    int                     m_nPDFDevice;   // 0: PostScript, 1: PDF
    orientation::type       m_eOrientation;
    ::rtl::OUString         m_aPrinterName;
    const PPDParser*        m_pParser;
    PPDContext              m_aContext;

    JobData() :
            m_nCopies( 1 ),
            m_nLeftMarginAdjust( 0 ),
            m_nRightMarginAdjust( 0 ),
            m_nTopMarginAdjust( 0 ),
            m_nBottomMarginAdjust( 0 ),
            m_nColorDepth( 24 ),
            m_nPSLevel( 0 ),
            m_nColorDevice( 0 ),
            m_nPDFDevice( 0 ),
            m_eOrientation( orientation::Portrait ),
            m_pParser( NULL ) {}

    JobData& operator=(const psp::JobData& rRight);

    JobData( const JobData& rData ) { *this = rData; }

    void setCollate( bool bCollate );
    bool setPaper( int nWidth, int nHeight ); // dimensions in pt
    bool setPaperBin( int nPaperBin ); // dimensions in pt

    // creates a new buffer using new
    // it is up to the user to delete it again
    bool getStreamBuffer( void*& pData, int& bytes );
    static bool constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData );
};

} // namespace


#endif // PSPRINT_JOBDATA_HXX
