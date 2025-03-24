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

#pragma once

#include "ppdparser.hxx"

namespace psp {

enum class orientation {
    Portrait,
    Landscape
};

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
    int                     m_nColorDevice; // 0: no override, -1 grey scale, +1 color
    orientation             m_eOrientation;
    OUString                m_aPrinterName;
    bool                    m_bPapersizeFromSetup;
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
            m_nColorDevice( 0 ),
            m_eOrientation( orientation::Portrait ),
            m_bPapersizeFromSetup( false ),
            m_pParser( nullptr ) {}

    JobData& operator=(const psp::JobData& rRight);

    JobData( const JobData& rData ) { *this = rData; }

    void setCollate( bool bCollate );
    void setPaper( int nWidth, int nHeight ); // dimensions in pt
    void setPaperBin( int nPaperBin );

    // creates a new buffer using new
    // it is up to the user to delete it again
    bool getStreamBuffer( std::unique_ptr<sal_uInt8[]>& pData, sal_uInt32& bytes );
    static bool constructFromStreamBuffer( const void* pData, sal_uInt32 bytes, JobData& rJobData );
};

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
