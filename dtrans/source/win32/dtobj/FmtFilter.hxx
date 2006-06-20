/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FmtFilter.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:05:44 $
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


#ifndef _FMTFILTER_HXX_
#define _FMTFILTER_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/*------------------------------------------------------------------------
    input:
    aMetaFilePict - a sequence of bytes containing a METAFILEPICT struct
------------------------------------------------------------------------*/
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL WinMFPictToOOMFPict( com::sun::star::uno::Sequence< sal_Int8 >& aMetaFilePict );
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL WinENHMFPictToOOMFPict( HENHMETAFILE hEnhMetaFile );

/*------------------------------------------------------------------------
    input:
    aByteStream - a sequence of bytes containing a openoffice metafile
                  picture with a leading METAFILEHEADER
------------------------------------------------------------------------*/
HMETAFILEPICT SAL_CALL OOMFPictToWinMFPict( com::sun::star::uno::Sequence< sal_Int8 >& aOOMetaFilePict );
HENHMETAFILE  SAL_CALL OOMFPictToWinENHMFPict( com::sun::star::uno::Sequence< sal_Int8 >& aOOMetaFilePict );

/*------------------------------------------------------------------------
    input:
    aWinDIB - sequence of bytes containing a windows device independent
              bitmap
------------------------------------------------------------------------*/
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL WinDIBToOOBMP( const com::sun::star::uno::Sequence< sal_Int8 >& aWinDIB );

/*------------------------------------------------------------------------
    input:
    aOOBmp - sequence of bytes containing a openoffice bitmap
------------------------------------------------------------------------*/
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OOBmpToWinDIB( com::sun::star::uno::Sequence< sal_Int8 >& aOOBmp );

/*------------------------------------------------------------------------
    input:
    aTextHtml - a sequence of text/html which will be converted to the
    HTML Format; the HTML Format has header before the real html data
    the Format is described in the MSDN Library under HTML Clipboard
    Format
------------------------------------------------------------------------*/
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL TextHtmlToHTMLFormat( com::sun::star::uno::Sequence< sal_Int8 >& aTextHtml );

/**
    Return a FileList in which Windows Shell Links (lnk) are resolved.
    If for what ever reason a resolution is not possible leave the
    original lnk file.
*/
com::sun::star::uno::Sequence< sal_Int8 > CF_HDROPToFileList(HGLOBAL hGlobal);

#endif
