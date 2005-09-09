/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comdep.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:12:48 $
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

#ifdef MAC
#define private public
#define protected public
#endif

#include "comdep.hxx"

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _LIST_HXX
#include <list.hxx>
#endif
#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif

DBG_NAMEEX( DirEntry );

//--------------------------------------------------------------------

#if defined( DOS ) || defined( WIN )

#ifdef MSC
#include "dosmsc.cxx"
#endif

#if defined( BLC ) || defined( TCPP )
#include "dosblc.cxx"
#endif

#ifdef ZTC
#include "dosztc.cxx"
#endif

#else

#if defined( WNT ) && !defined( WTC )
#include "wntmsc.cxx"
#endif

#ifdef UNX
#include "unx.cxx"
#endif

#ifdef PM2
#include "os2.cxx"
#endif

#ifdef MAC
#include "mac.cxx"
#endif

#endif

