/*************************************************************************
 *
 *  $RCSfile: svtuno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-09-18 14:45:44 $
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

#ifndef _SVTOOLS_SVTUNO_HXX
#define _SVTOOLS_SVTUNO_HXX

// Macro to define const unicode a'la "..."
// It's better then "OUString::createFromAscii(...)" !!!
#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))
#define DEFINE_CONST_OUSTRING(CONSTASCII)       OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

// defines ---------------------------------------------------------------
#define UNOANY                          ::com::sun::star::uno::Any
#define UNOEXCEPTION                    ::com::sun::star::uno::Exception
#define UNOMUTEX                        ::osl::Mutex
#define UNOMUTEXGUARD                   ::osl::MutexGuard
#define UNOOIMPLEMENTATIONID            ::cppu::OImplementationId
#define UNOOTYPECOLLECTION              ::cppu::OTypeCollection
#define UNOOUSTRING                     ::rtl::OUString
#define UNOPROPERTYVALUE                ::com::sun::star::beans::PropertyValue
#define UNOSTRINGPAIR                   ::com::sun::star::beans::StringPair
#define UNOREFERENCE                    ::com::sun::star::uno::Reference
#define UNORUNTIMEEXCEPTION             ::com::sun::star::uno::RuntimeException
#define UNOINVALIDREGISTRYEXCEPTION     ::com::sun::star::registry::InvalidRegistryException
#define UNOSEQUENCE                     ::com::sun::star::uno::Sequence
#define UNOTYPE                         ::com::sun::star::uno::Type
#define UNOURL                          ::com::sun::star::util::URL
#define UNOXINTERFACE                   ::com::sun::star::uno::XInterface
#define UNOXMULTISERVICEFACTORY         ::com::sun::star::lang::XMultiServiceFactory
#define UNOXSINGLESERVICEFACTORY        ::com::sun::star::lang::XSingleServiceFactory
#define UNOXTYPEPROVIDER                ::com::sun::star::lang::XTypeProvider
#define UNOILLEGALARGUMENTEXCEPTION     ::com::sun::star::lang::IllegalArgumentException

// -----------------------------------------------------------------------

#endif
