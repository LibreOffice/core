#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 18:54:07 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=oox
TARGET=token

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/tokenmap.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/tokens.hxx $(INCCOM)$/tokenwords.inc $(MISC)$/tokens.gperf : tokens.txt gentoken.pl
        $(PERL) gentoken.pl tokens.txt $(INCCOM)$/tokens.hxx $(INCCOM)$/tokenwords.inc $(MISC)$/tokens.gperf

$(INCCOM)$/tokens.inc : $(MISC)$/tokens.gperf
        gperf --compare-strncmp $(MISC)$/tokens.gperf | $(SED) -e "s/(char\*)0/(char\*)0, 0/g" >$(INCCOM)$/tokens.inc

$(SLO)$/tokenmap.obj : $(INCCOM)$/tokens.inc $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx

$(INCCOM)$/tokens.gperf : $(INCCOM)$/tokens.hxx
