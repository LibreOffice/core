/*************************************************************************
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-07-25 11:39:19 $
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

#include <salsys.hxx>
#include <stacktrace.hxx>

#ifdef DEBUG
#include <stdio.h>
#endif

// -----------------------------------------------------------------------

String GetSalSummarySystemInfos( ULONG nFlags )
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

