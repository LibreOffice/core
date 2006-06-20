/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnose_ex.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:58:35 $
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

#ifndef CONNECTIVITY_DIAGNOSE_EX_H
#define CONNECTIVITY_DIAGNOSE_EX_H

#if OSL_DEBUG_LEVEL > 0
    #define OSL_VERIFY_RES( expression, fail_message ) \
        OSL_ENSURE( expression, fail_message )
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        OSL_ENSURE( expression == compare, fail_message )
#else
    #define OSL_VERIFY_RES( expression, fail_message ) \
        (void)(expression)
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        (void)(expression)
#endif

#define OSL_UNUSED( expression ) \
    (void)(expression)

#endif // CONNECTIVITY_DIAGNOSE_EX_H
