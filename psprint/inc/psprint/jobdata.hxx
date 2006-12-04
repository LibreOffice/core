/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jobdata.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-04 16:33:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PSPRINT_JOBDATA_HXX_
#define _PSPRINT_JOBDATA_HXX_

#ifndef _PSPRINT_PPDPARSER_HXX_
#include <psprint/ppdparser.hxx>
#endif

namespace psp {

namespace orientation {
enum type {
    Portrait,
    Landscape
};
}

struct JobData
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
            m_eOrientation( orientation::Portrait ),
            m_pParser( NULL ) {}

    JobData& operator=(const psp::JobData& rRight);

    JobData( const JobData& rData ) { *this = rData; }

    // creates a new buffer using new
    // it is up to the user to delete it again
    bool getStreamBuffer( void*& pData, int& bytes );
    static bool constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData );
};

} // namespace


#endif // PSPRINT_JOBDATA_HXX
