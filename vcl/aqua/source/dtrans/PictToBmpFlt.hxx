/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_PICTTOBMPFLT_HXX
#define INCLUDED_PICTTOBMPFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

/* Transform PICT into the a Window BMP.

   Returns true if the conversion was successful false
   otherwise.
 */
bool PICTtoBMP(com::sun::star::uno::Sequence<sal_Int8>& aPict,
               com::sun::star::uno::Sequence<sal_Int8>& aBmp);

/* Transform a Windows BMP to a PICT.

   Returns true if the conversion was successful false
   otherwise.
 */
bool BMPtoPICT(com::sun::star::uno::Sequence<sal_Int8>& aBmp,
               com::sun::star::uno::Sequence<sal_Int8>& aPict);

#define PICTImageFileType ((NSBitmapImageFileType)~0)

bool ImageToBMP( com::sun::star::uno::Sequence<sal_Int8>& aPict,
                 com::sun::star::uno::Sequence<sal_Int8>& aBmp,
                 NSBitmapImageFileType eInFormat);

bool BMPToImage( com::sun::star::uno::Sequence<sal_Int8>& aBmp,
                 com::sun::star::uno::Sequence<sal_Int8>& aPict,
                 NSBitmapImageFileType eOutFormat
                );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
