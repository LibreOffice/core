#************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
# ***********************************************************************/

PRJ=..$/..
PRJNAME=writerfilter
TARGET=doctok
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE
#EXTERNAL_WARNINGS_NOT_ERRORS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
#CDEFS+=-DDEBUG_DOCTOK_DUMP


# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/WW8Annotation.obj \
    $(SLO)$/WW8OutputWithDepth.obj \
    $(SLO)$/WW8Text.obj \
    $(SLO)$/DffImpl.obj \
    $(SLO)$/Dff.obj \
    $(SLO)$/WW8Picture.obj \
    $(SLO)$/WW8Sttbf.obj \
    $(SLO)$/WW8Table.obj \
    $(SLO)$/WW8StyleSheet.obj \
    $(SLO)$/WW8FontTable.obj \
    $(SLO)$/WW8LFOTable.obj \
    $(SLO)$/WW8ListTable.obj \
    $(SLO)$/resources.obj \
    $(SLO)$/doctokutil.obj \
    $(SLO)$/WW8CpAndFc.obj \
    $(SLO)$/WW8StructBase.obj \
    $(SLO)$/WW8Clx.obj \
    $(SLO)$/WW8FKPImpl.obj \
    $(SLO)$/WW8BinTableImpl.obj \
    $(SLO)$/WW8StreamImpl.obj \
    $(SLO)$/WW8PropertySetImpl.obj \
    $(SLO)$/WW8DocumentImpl.obj \
    $(SLO)$/WW8PieceTableImpl.obj \
    $(SLO)$/WW8ResourceModelImpl.obj


SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="OS2"
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
RESOURCEMODELLIB=-lresourcemodel
.ELSE
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF


SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB) \
    $(RESOURCEMODELLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

DOCTOKHXXOUTDIR=$(INCCOM)/doctok
DOCTOKCXXOUTDIR=$(MISC)
DOCTOKHXXOUTDIRCREATED=$(DOCTOKHXXOUTDIR)/created

DOCTOKMODEL=resources.xmi
DOCTOKRESOURCESXSL=resources.xsl
DOCTOKRESOURCESIMPLXSL=resourcesimpl.xsl
DOCTOKRESOURCEIDSXSL=resourceids.xsl
DOCTOKSPRMIDSXSL=sprmids.xsl
DOCTOKRESOURCETOOLS=resourcetools.xsl

DOCTOKRESOURCEIDSHXX=$(DOCTOKHXXOUTDIR)$/resourceids.hxx
SPRMIDSHXX=$(DOCTOKHXXOUTDIR)$/sprmids.hxx
RESOURCESHXX=$(DOCTOKHXXOUTDIR)$/resources.hxx
RESOURCESCXX=$(DOCTOKCXXOUTDIR)$/resources.cxx

DOCTOKGENHEADERS=$(DOCTOKRESOURCEIDSHXX) $(SPRMIDSHXX) $(RESOURCESHXX)
DOCTOKGENFILES=$(DOCTOKGENHEADERS) $(RESOURCESCXX)

$(DOCTOKHXXOUTDIRCREATED):
    $(MKDIRHIER) $(DOCTOKHXXOUTDIR)
    $(TOUCH) $@

$(DOCTOKGENHEADERS): $(DOCTOKHXXOUTDIRCREATED)

$(RESOURCESHXX): $(DOCTOKRESOURCESXSL) $(DOCTOKMODEL)
    $(XSLTPROC) $(DOCTOKRESOURCESXSL) $(DOCTOKMODEL) > $@

$(SPRMIDSHXX): $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    $(XSLTPROC) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL) > $@

$(RESOURCESCXX): $(DOCTOKRESOURCESIMPLXSL) $(DOCTOKMODEL) $(DOCTOKRESOURCEIDSHXX) $(DOCTOKRESOURCETOOLS)
    $(XSLTPROC) $(DOCTOKRESOURCESIMPLXSL) $(DOCTOKMODEL) > $@

$(DOCTOKRESOURCEIDSHXX):  $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL)
    $(XSLTPROC) $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL) > $@

$(SLOFILES) : $(DOCTOKGENHEADERS)

.PHONY: genmake genclean

genmake: $(DOCTOKGENFILES)

genclean:
    rm -f $(DOCTOKGENFILES)

genheaders: $(DOCTOKGENHEADERS)
