/*************************************************************************
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:42 $
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

#define _SV_SALSYS_CXX

#include <cstdio>
#include <string>

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_DTINT_HXX
#include <dtint.hxx>
#endif
#include <salframe.hxx>

#include <strhelper.hxx>
#include <stacktrace.hxx>


void SalSystemData::CreateDtIntegrator( SalFrame* pNewFrame )
{
    if( pNewFrame )
    {
        if( m_pDtIntegrator && m_pDtIntegrator->GetDisplay() !=
            pNewFrame->maFrameData.GetDisplay()->GetDisplay() )
        {
            m_pDtIntegrator->Release();
            m_pDtIntegrator = 0;
        }
        if( ! m_pDtIntegrator )
        {
            m_pDtIntegrator = DtIntegrator::CreateDtIntegrator( pNewFrame );
            m_pDtIntegrator->Acquire();
        }
    }
}

void SalSystemData::SetSalDisplay( SalDisplay *pSalDisplay )
{
    m_pSalDisplay = pSalDisplay;
}

SalSystemData::~SalSystemData()
{
    if( m_pDtIntegrator )
        m_pDtIntegrator->Release();
}


// -----------------------------------------------------------------------

SalSystem* SalInstance::CreateSystem()
{
    return new SalSystem();
}

// -----------------------------------------------------------------------

void SalInstance::DestroySystem( SalSystem* pSystem )
{
    delete pSystem;
}

// =======================================================================

SalSystem::SalSystem()
{
    maSystemData.m_pSalDisplay      = 0;
    maSystemData.m_pDtIntegrator    = 0;
}

// -----------------------------------------------------------------------

SalSystem::~SalSystem()
{
}

// -----------------------------------------------------------------------

BOOL SalSystem::StartProcess( SalFrame* pFrame, const String& rFileName,
                              const String& rParam,
                              const String& rWorkingDirectory )
{
    maSystemData.CreateDtIntegrator( pFrame );

    XubString aFileName( '\"' );
    aFileName += rFileName;
    aFileName += '\"';

    // StartProcess is desktop specific
    return maSystemData.m_pDtIntegrator->
        StartProcess( aFileName,
                      const_cast<XubString&>(rParam),
                      const_cast<XubString&>(rWorkingDirectory) );
}

// -----------------------------------------------------------------------

BOOL SalSystem::AddRecentDoc( SalFrame* pFrame, const XubString& rFileName )
{
    return FALSE;
}

// -----------------------------------------------------------------------

String SalSystem::GetSummarySystemInfos( ULONG nFlags )
{
    sal_PostMortem aPostMortem;

    /*
     *  unimplemented flags:
     *  SALSYSTEM_GETSYSTEMINFO_MODULES
     *  SALSYSTEM_GETSYSTEMINFO_MOUSEINFO
     *  SALSYSTEM_GETSYSTEMINFO_SYSTEMDIRS
     *  SALSYSTEM_GETSYSTEMINFO_LOCALVOLUMES
     */

    ByteString aRet;
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_SYSTEMVERSION )
        aRet += aPostMortem.getSystemInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_CPUTYPE )
        aRet += aPostMortem.getProcessorInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_MEMORYINFO )
        aRet += aPostMortem.getMemoryInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_STACK )
        aRet += aPostMortem.getStackTrace();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_GRAPHICSSYSTEM )
        aRet += aPostMortem.getGraphicsSystem();

#ifdef DEBUG
    fprintf( stderr, "SalSystem::GetSummarySystemInfos() =\n%s", aRet.GetBuffer() );
#endif
    return String( aRet, RTL_TEXTENCODING_ISO_8859_1 );
}

