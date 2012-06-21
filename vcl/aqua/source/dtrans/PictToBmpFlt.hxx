/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
