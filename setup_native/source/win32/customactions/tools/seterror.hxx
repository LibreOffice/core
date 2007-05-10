/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seterror.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gm $ $Date: 2007-05-10 11:05:29 $
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

#ifndef _SETERROR_HXX_
#define _SETERROR_HXX_

//----------------------------------------------------------
// list of own error codes

#define MSI_ERROR_INVALIDDIRECTORY          9001
#define MSI_ERROR_ISWRONGPRODUCT            9002
#define MSI_ERROR_PATCHISOLDER              9003

#define MSI_ERROR_NEW_VERSION_FOUND         9010
#define MSI_ERROR_SAME_VERSION_FOUND        9011
#define MSI_ERROR_OLD_VERSION_FOUND         9012
#define MSI_ERROR_NEW_PATCH_FOUND           9013
#define MSI_ERROR_SAME_PATCH_FOUND          9014
#define MSI_ERROR_OLD_PATCH_FOUND           9015

#define MSI_ERROR_OFFICE_IS_RUNNING         9020

//----------------------------------------------------------

void SetMsiErrorCode( int nErrorCode );

#endif

