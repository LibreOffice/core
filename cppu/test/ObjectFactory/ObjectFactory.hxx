/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:44:22 $
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

#ifndef INCLUDED_ObjectFactory_hxx
#define INCLUDED_ObjectFactory_hxx


#include "callee.hxx"


#ifdef CPPU_test_ObjectFactory_IMPL
# define CPPU_test_ObjectFactory_EXPORT SAL_DLLPUBLIC_EXPORT

#else
# define CPPU_test_ObjectFactory_EXPORT SAL_DLLPUBLIC_IMPORT

#endif


CPPU_test_ObjectFactory_EXPORT void * createObject(rtl::OUString const & envDcp, Callee * pCallee);
CPPU_test_ObjectFactory_EXPORT void   callObject  (rtl::OUString const & envDcp, void *);


#endif
