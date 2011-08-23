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

#ifndef _SGFFILT_HXX
#define _SGFFILT_HXX
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

namespace binfilter
{

BYTE CheckSgfTyp(SvStream& rInp, USHORT& nVersion);
BOOL SgfBMapFilter(SvStream& rInp, SvStream& rOut);
BOOL SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf);
BOOL SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, INetURLObject aIniPath );

// Konstanten fÅr CheckSgfTyp()
#define SGF_BITIMAGE   1     /* Bitmap                      */
#define SGF_SIMPVECT   2     /* Einfaches Vectorformat      */
#define SGF_POSTSCRP   3     /* Postscript file             */
#define SGF_STARDRAW   7     /* StarDraw SGV-Datei          */
#define SGF_DONTKNOW 255     /* Unbekannt oder kein SGF/SGV */

#define SGV_VERSION    3     /* SGV mit anderer Version wird abgewiesen */
                             /* 3 entspricht StarDraw 2.00/2.01 MÑrz'93 */
}

#endif  //_SGFFILT_HXX
