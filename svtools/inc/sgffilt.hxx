/*************************************************************************
 *
 *  $RCSfile: sgffilt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
BOOL SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, INetURLObject aIniPath, const INetURLObject& rFltPath, const INetURLObject& rCfgPath);

// Konstanten fÅr CheckSgfTyp()
#define SGF_BITIMAGE   1     /* Bitmap                      */
#define SGF_SIMPVECT   2     /* Einfaches Vectorformat      */
#define SGF_POSTSCRP   3     /* Postscript file             */
#define SGF_STARDRAW   7     /* StarDraw SGV-Datei          */
#define SGF_DONTKNOW 255     /* Unbekannt oder kein SGF/SGV */

#define SGV_VERSION    3     /* SGV mit anderer Version wird abgewiesen */
                             /* 3 entspricht StarDraw 2.00/2.01 MÑrz'93 */
#endif  //_SGFFILT_HXX
