/*************************************************************************
 *
 *  $RCSfile: main.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#ifndef CGM_MAIN_HXX
#define CGM_MAIN_HXX

#include <cgm.hxx>

// ---------------------------------------------------------------

#define CGM_LEVEL1                      0x0001  // VERSION 1 METAFILE
#define CGM_LEVEL2                      0x0002
#define CGM_LEVEL3                      0x0003
#define CGM_GDSF_ONLY                   0x00F0
#define CGM_UNKNOWN_LEVEL               0x00F1
#define CGM_UNKNOWN_COMMAND             0x00F2
#define CGM_DESCRIPTION                 0x00F3

#define CGM_EXTENDED_PRIMITIVES_SET     0x0100  // INDICATES PRIMITIVES WHICH ARE AVAILABLE IN
                                                // LEVEL 1 BUT ARE NOT DEFINED IN ISO 7942(GKS)
#define CGM_DRAWING_PLUS_CONTROL_SET    0x0200  // INDICATES THAT THIS IS AN ADDITIONAL LEVEL 1
                                                // ELEMENT
#define ComOut( Level, Description ) if ( mpCommentOut ) ImplComment( Level, Description );

#define BMCOL( _col ) BitmapColor( (sal_Int8)(_col >> 16 ), (sal_Int8)( _col >> 8 ), (sal_Int8)_col )
#define ALIGN2( _nElementSize ) { _nElementSize = ( _nElementSize + 1 ) & ~1; }
#define ALIGN4( _nElementSize ) { _nElementSize = ( _nElementSize + 3 ) & ~3; }
#define ALIGN8( _nElementSize ) { _nElementSize = ( _nElementSize + 7 ) & ~7; }

#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>
#include "bundles.hxx"
#include "bitmap.hxx"
#include "elements.hxx"


#endif

