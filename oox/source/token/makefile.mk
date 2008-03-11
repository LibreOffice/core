#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2008-03-11 13:01:25 $
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

$(MISC)$/tokens.gperf $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx : 
    @@noop $(assign do_phony:=.PHONY)

$(MISC)$/do_tokens $(do_phony) : tokens.txt gentoken.pl $(MISC)$/tokens.gperf $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx
        $(PERL) gentoken.pl tokens.txt $(INCCOM)$/tokens.hxx $(INCCOM)$/tokenwords.inc $(MISC)$/tokens.gperf && $(TOUCH) $@

$(INCCOM)$/tokens.inc : $(MISC)$/tokens.gperf $(MISC)$/do_tokens
        gperf --compare-strncmp $(MISC)$/tokens.gperf | $(SED) -e "s/(char\*)0/(char\*)0, 0/g" >$(INCCOM)$/tokens.inc

$(SLO)$/tokenmap.obj : $(INCCOM)$/tokens.inc $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx $(MISC)$/do_tokens

