#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:03:28 $
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
LINKFLAGSDEFS=""

PRJ=..$/..

PRJNAME=cppuhelper
TARGET=defbootstrap
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

PERLINST1:=$(shell @+$(PERL) -V:installarchlib)
PERLINST2:=$(subst,installarchlib=, $(PERLINST1))
PERLINST3:=$(PERLINST2:s/'//)
PERLINST :=$(PERLINST3:s/;//)

PERLLIBS:=$(PERLINST)$/CORE
PERLINCS:=$(PERLINST)$/CORE

CFLAGS += -I$(PERLINCS)

.IF "$(GUI)"=="WNT"
PERLLIB=perl58.lib

LIB!:=$(LIB);$(PERLLIBS)
.EXPORT : LIB

.ENDIF

# --- Files --------------------------------------------------------

#CFLAGS +=   -DVERSION=\"0.01\" -DXS_VERSION=\"0.01\" -fPIC -I/develop6/update/dev/solenv/unxlngi3/lib/perl5/5.6.0/i686-linux/CORE/
#CFLAGS += -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DVERSION=\"0.01\" -DXS_VERSION=\"0.01\" -KPIC -I/develop6/update/dev/solenv/unxsols2/lib/perl5/5.6.0/sun4-solaris/CORE  

SLOFILES= $(SLO)$/defbootstrap_lib.obj

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
        $(PERLLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=defbootstrap.map


.IF "$(GUI)"=="WNT"
UNODLL=$(DLLDEST)$/UNO.dll
.ELSE
UNODLL=$(DLLDEST)$/UNO.so	
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL: \
    ALLTAR				\
    $(UNODLL)			\
    $(BIN)$/UNO.pm
.ENDIF

$(BIN)$/UNO.pm: UNO.pm
    cp UNO.pm $@

$(UNODLL): $(SHL1TARGETN)
    cp $(SHL1TARGETN) $@

.INCLUDE :	target.mk

