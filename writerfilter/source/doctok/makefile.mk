#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.6 $
#
#  last change: $Author: hbrinkm $ $Date: 2007-06-15 09:21:24 $
#
#  The Contents of this file are made available subject to
#  the terms of GNU Lesser General Public License Version 2.1.
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
# ***********************************************************************/

PRJ=..$/..
PRJNAME=writerfilter
TARGET=doctok
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION #-DDEBUG_DOCTOK_DUMP


# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/WW8Annotation.obj \
    $(SLO)$/WW8OutputWithDepth.obj \
    $(SLO)$/WW8Text.obj \
    $(SLO)$/DffImpl.obj \
    $(SLO)$/Dff.obj \
    $(SLO)$/WW8Analyzer.obj \
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
.ELIF "$(GUI)"=="WNT"
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF


SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB) \
    $(RESOURCEMODELLIB)
SHL1IMPLIB=i$(SHL1TARGET)
#SHL1USE_EXPORTS=name
SHL1USE_EXPORTS=ordinal

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(WRITERFILTER_GEN)"=="yes"

WRITERFILTERINCDIR=..$/..$/inc
DOCTOKHXXOUTDIR=.
DOCTOKCXXOUTDIR=.

DOCTOKMODEL=resources.xmi
DOCTOKRESOURCESXSL=resources.xsl
DOCTOKRESOURCESIMPLXSL=resourcesimpl.xsl
DOCTOKRESOURCEIDSXSL=resourceids.xsl
DOCTOKSPRMIDSXSL=sprmids.xsl

DOCTOKRESOURCEIDSHXX=$(WRITERFILTERINCDIR)$/doctok$/resourceids.hxx
RESOURCEMODELSPRMIDSHXX=$(WRITERFILTERINCDIR)$/resourcemodel$/sprmids.hxx
RESOURCESHXX=$(DOCTOKHXXOUTDIR)$/resources.hxx
RESOURCESCXX=$(DOCTOKCXXOUTDIR)$/resources.cxx

DOCTOKGENHEADERS=$(DOCTOKRESOURCEIDSHXX) $(RESOURCEMODELSPRMIDSHXX) $(RESOURCESHXX)

XALANJAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/xalan.jar
XALAN=$(JAVA) -jar $(XALANJAR)

$(RESOURCESHXX): $(DOCTOKRESOURCESXSL) $(DOCTOKMODEL)
    $(XALAN) -xsl $(DOCTOKRESOURCESXSL) -in $(DOCTOKMODEL) > $(RESOURCESHXX)

$(RESOURCESCXX): $(DOCTOKRESOURCESIMPLXSL) $(DOCTOKMODEL) $(DOCTOKRESOURCEIDSHXX)
    $(XALAN) -xsl $(DOCTOKRESOURCESIMPLXSL) -in $(DOCTOKMODEL) > $(RESOURCESCXX)

$(SLO)$/resources.obj: $(RESOURCESCXX) $(RESOURCESHXX)

$(DOCTOKRESOURCEIDSHXX):  $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL)
    $(MKDIRHIER) $(WRITERFILTERINCDIR)/doctok
    $(XALAN) -xsl $(DOCTOKRESOURCEIDSXSL) -in $(DOCTOKMODEL) > $(DOCTOKRESOURCEIDSHXX)

$(RESOURCEMODELSPRMIDSHXX): $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    $(MKDIRHIER) $(WRITERFILTERINCDIR)/resourcemodel
    $(XALAN) -xsl $(DOCTOKSPRMIDSXSL) -in $(DOCTOKMODEL) > $(RESOURCEMODELSPRMIDSHXX)

$(SLO)$/Dff.obj: $(DOCTOKGENHEADERS)
$(SLO)$/DffImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/doctokutil.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Analyzer.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Annotation.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8BinTableImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Clx.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8CpAndFc.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8DocumentImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8FKPImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8FontTable.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8LFOLevel.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8LFOTable.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8ListTable.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8OutputWithDepth.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Picture.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8PieceTableImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8PropertySetImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8ResourceModelImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8StreamImpl.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8StructBase.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Sttbf.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8StyleSheet.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Table.obj: $(DOCTOKGENHEADERS)
$(SLO)$/WW8Text.obj: $(DOCTOKGENHEADERS)

.ENDIF
