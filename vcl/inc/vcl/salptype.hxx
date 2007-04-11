/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salptype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:08:39 $
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

#ifndef _SV_SALPTYPE_HXX
#define _SV_SALPTYPE_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// --------------------
// - SalJobSetupFlags -
// --------------------

#define SAL_JOBSET_ORIENTATION                  ((ULONG)0x00000001)
#define SAL_JOBSET_PAPERBIN                     ((ULONG)0x00000002)
#define SAL_JOBSET_PAPERSIZE                    ((ULONG)0x00000004)
#define SAL_JOBSET_ALL                          (SAL_JOBSET_ORIENTATION | SAL_JOBSET_PAPERBIN | SAL_JOBSET_PAPERSIZE)

// -------------------
// - SalPrinterError -
// -------------------

#define SAL_PRINTER_ERROR_GENERALERROR          1
#define SAL_PRINTER_ERROR_ABORT                 2

// -------------------
// - SalPrinterProcs -
// -------------------

class SalPrinter;
typedef long (*SALPRNABORTPROC)( void* pInst, SalPrinter* pPrinter );

#endif // _SV_SALPTYPE_HXX
