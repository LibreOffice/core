/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVTOOLS_SVTUNO_HXX
#define _SVTOOLS_SVTUNO_HXX

namespace binfilter
{

// Macro to define const unicode a'la "..."
// It's better then "OUString::createFromAscii(...)" !!!
#define	DEFINE_CONST_UNICODE(CONSTASCII)		UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))
#define	DEFINE_CONST_OUSTRING(CONSTASCII)		OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

// defines ---------------------------------------------------------------
#define	UNOANY							::com::sun::star::uno::Any
#define	UNOEXCEPTION					::com::sun::star::uno::Exception
#define	UNOMUTEX						::osl::Mutex
#define	UNOMUTEXGUARD					::osl::MutexGuard
#define	UNOOIMPLEMENTATIONID			::cppu::OImplementationId
#define	UNOOTYPECOLLECTION				::cppu::OTypeCollection
#define	UNOOUSTRING						::rtl::OUString
#define	UNOPROPERTYVALUE				::com::sun::star::beans::PropertyValue
#define	UNOSTRINGPAIR					::com::sun::star::beans::StringPair
#define	UNOREFERENCE					::com::sun::star::uno::Reference
#define	UNORUNTIMEEXCEPTION				::com::sun::star::uno::RuntimeException
#define	UNOINVALIDREGISTRYEXCEPTION		::com::sun::star::registry::InvalidRegistryException
#define	UNOSEQUENCE						::com::sun::star::uno::Sequence
#define	UNOTYPE							::com::sun::star::uno::Type
#define	UNOURL							::com::sun::star::util::URL
#define	UNOXINTERFACE					::com::sun::star::uno::XInterface
#define	UNOXMULTISERVICEFACTORY			::com::sun::star::lang::XMultiServiceFactory
#define	UNOXSINGLESERVICEFACTORY		::com::sun::star::lang::XSingleServiceFactory
#define	UNOXTYPEPROVIDER				::com::sun::star::lang::XTypeProvider
#define UNOILLEGALARGUMENTEXCEPTION	    ::com::sun::star::lang::IllegalArgumentException

// -----------------------------------------------------------------------

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
