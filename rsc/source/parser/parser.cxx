/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parser.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:39:54 $
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
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <ctype.h>

#include <tools/solar.h>
#define RSC_COMPILER

#include <rscall.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rsckey.hxx>
#include <rsctree.hxx>
#include <rscerror.h>
#include <rscdef.hxx>

#include <rsctop.hxx>
#include <rscmgr.hxx>
#include <rscconst.hxx>
#include <rscarray.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rscrange.hxx>
#include <rscflag.hxx>
#include <rscstr.hxx>

#include <rscdb.hxx>
#include <rscpar.hxx>

