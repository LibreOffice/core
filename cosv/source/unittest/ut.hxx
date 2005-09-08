/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ut.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:10:31 $
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

#ifndef COSV_UNITTEST_UT_HXX
#define COSV_UNITTEST_UT_HXX


#define UT_CHECK( fname, cond ) \
    if ( NOT (cond) ) { std::cerr << "ftest_" << #fname << " " << #cond << endl; \
        ret = false; }

#define CUT_DECL( nsp, cname ) \
    bool classtest_##cname()
#define FUT_DECL( cname, fname ) \
    bool ftest_##fname( cname & r##cname )

#define UT_RESULT( result ) \
    if (result ) std::cout << "All unit tests passed successfully." << std::endl; \
    else std::cout << "Errors in unit tests.\n" << std::endl



CUT_DECL( csv, File );
CUT_DECL( csv, String );

#endif



