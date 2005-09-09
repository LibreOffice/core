/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: new_del.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:53:58 $
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

#ifndef CPV_NEW_DEL_H
#define CPV_NEW_DEL_H


void *              New(
                        intt                i_nSize );
void                Delete(
                        void *              io_pMemory );

/* at ASSIGN_NEW ptr must be an L-Value. */
#define ASSIGN_NEW( ptr, type ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr ); }

#define ASSIGN_NEW1( ptr, type, param1 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1 ); }

#define ASSIGN_NEW2( ptr, type, param1, param2 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1, param2 ); }

#define ASSIGN_NEW3( ptr, type, param1, param2, param3 ) \
    { ptr = New( sizeof( type ) ); type##_CTOR( ptr, param1, param2, param3 ); }

#define DELETE( ptr, type ) \
    { type##_DTOR( ptr ); Delete(ptr); }

#endif


