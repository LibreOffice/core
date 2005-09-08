/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tstMgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:31:22 $
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
#ifndef _SOLTOOLS_TESTSHL_TSTMGR_HXX__
#define _SOLTOOLS_TESTSHL_TSTMGR_HXX__

#ifndef _SAL_TYPES_H_
#include    <sal/types.h>
#endif

#include <vector>

using namespace std;

// <namespace_tstutl>
namespace tstutl {

// <class_tstMgr>
class tstMgr {

    // <private_members>
    struct tstMgr_Impl;
    tstMgr_Impl* pImpl;
    // </private_members>

    // <private_methods>
    void cleanup();
    // </private_methods>

public:

    // <dtor>
    ~tstMgr(){
        cleanup();
    } // </dtor>


    // <public_methods>
    sal_Bool initialize( sal_Char* moduleName, sal_Bool boom = sal_False );
    sal_Bool test_Entry( sal_Char* entry, sal_Char* logName = 0 );
    sal_Bool test_Entries( vector< sal_Char* > entries, sal_Char* logName = 0 );
    sal_Bool test_EntriesFromFile( sal_Char* fName, sal_Char* logName = 0 );
    // </public_methods>

}; // </class_tstMgr>

} // </namespace_tstutl>

#endif



