#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2003-10-20 16:50:02 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sw
TARGET=doc

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/SwStyleNameMapper.obj \
        $(SLO)$/acmplwrd.obj \
        $(SLO)$/doc.obj \
        $(SLO)$/docbasic.obj \
        $(SLO)$/docbm.obj \
        $(SLO)$/docchart.obj \
        $(SLO)$/doccomp.obj \
        $(SLO)$/doccorr.obj \
        $(SLO)$/docdde.obj \
        $(SLO)$/docdesc.obj \
        $(SLO)$/docdraw.obj \
        $(SLO)$/docedt.obj \
        $(SLO)$/docfld.obj \
        $(SLO)$/docfly.obj \
        $(SLO)$/docfmt.obj \
        $(SLO)$/docftn.obj \
        $(SLO)$/doclay.obj \
        $(SLO)$/docglbl.obj \
        $(SLO)$/docglos.obj \
        $(SLO)$/docnew.obj \
        $(SLO)$/docnum.obj \
        $(SLO)$/docredln.obj \
        $(SLO)$/docruby.obj \
        $(SLO)$/docsort.obj \
        $(SLO)$/docstat.obj \
        $(SLO)$/doctxm.obj \
        $(SLO)$/extinput.obj \
        $(SLO)$/fmtcol.obj \
        $(SLO)$/ftnidx.obj \
        $(SLO)$/gctable.obj \
        $(SLO)$/lineinfo.obj \
        $(SLO)$/notxtfrm.obj \
        $(SLO)$/number.obj \
        $(SLO)$/poolfmt.obj \
        $(SLO)$/sortopt.obj \
        $(SLO)$/swserv.obj \
        $(SLO)$/swtable.obj \
        $(SLO)$/tblafmt.obj \
        $(SLO)$/tblcpy.obj \
        $(SLO)$/tblrwcl.obj \
        $(SLO)$/visiturl.obj \
        $(SLO)$/htmltbl.obj

.IF "$(OS)$(CVER)"=="MACOSXC295"
SLOFILES += $(SLO)$/gcc_outl.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

