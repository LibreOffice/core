/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdenumdef.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:59:14 $
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

#ifndef _SD_ENUMDEF_HXX
#define _SD_ENUMDEF_HXX

//CHINA001 move from dlgass.hxx
enum OutputType { OUTPUT_PAGE, OUTPUT_OVERHEAD, OUTPUT_SLIDE, OUTPUT_PRESENTATION, OUTPUT_ORIGINAL };
enum StartType { ST_EMPTY, ST_TEMPLATE, ST_OPEN };
//CHINA001 end

//CHINA001 move from dlgsnap.hxx
#define RET_SNAP_DELETE 111
enum SnapKind { SK_HORIZONTAL, SK_VERTICAL, SK_POINT };
//CHINA001 end

#endif

