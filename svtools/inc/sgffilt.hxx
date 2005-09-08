/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sgffilt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:40:14 $
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

#ifndef _SGFFILT_HXX
#define _SGFFILT_HXX
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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
#endif  //_SGFFILT_HXX
