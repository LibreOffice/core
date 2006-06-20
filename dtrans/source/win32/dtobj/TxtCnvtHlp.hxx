/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TxtCnvtHlp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:06:06 $
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


#ifndef _TXTCNVTHLP_HXX_
#define _TXTCNVTHLP_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HPP_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

int MultiByteToWideCharEx( UINT cp_src,
                           LPCSTR lpMultiByteString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero = TRUE );

int WideCharToMultiByteEx( UINT cp_dest,
                           LPCWSTR lpWideCharString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero = TRUE );

#endif
