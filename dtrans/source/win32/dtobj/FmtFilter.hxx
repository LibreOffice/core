/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _FMTFILTER_HXX_
#define _FMTFILTER_HXX_

#include <sal/types.h>

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
    aWinDIB - sequence of bytes containing a windows bitmap handle
------------------------------------------------------------------------*/
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL WinBITMAPToOOBMP( HBITMAP );

/*------------------------------------------------------------------------
    input:
    aOOBmp - sequence of bytes containing a openoffice bitmap
    May contain CF_DIBV5 or CF_DIB, but removing the BITMAPFILEHEADER
    is always the same size
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
