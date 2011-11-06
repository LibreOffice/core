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
