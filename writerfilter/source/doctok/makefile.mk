#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************
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

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)"=="OS2"
RESOURCEMODELLIB=-lresourcemodel
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
    @$(MKDIRHIER) $(DOCTOKHXXOUTDIR)
    @$(TOUCH) $@

$(DOCTOKGENHEADERS): $(DOCTOKHXXOUTDIRCREATED)

$(RESOURCESHXX): $(DOCTOKRESOURCESXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKRESOURCESXSL) $(DOCTOKMODEL) > $@

$(SPRMIDSHXX): $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL) > $@

$(RESOURCESCXX): $(DOCTOKRESOURCESIMPLXSL) $(DOCTOKMODEL) $(DOCTOKRESOURCEIDSHXX) $(DOCTOKRESOURCETOOLS)
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKRESOURCESIMPLXSL) $(DOCTOKMODEL) > $@

$(DOCTOKRESOURCEIDSHXX):  $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL) > $@

$(SLOFILES) : $(DOCTOKGENHEADERS)

.PHONY: genmake genclean

genmake: $(DOCTOKGENFILES)

genclean:
    rm -f $(DOCTOKGENFILES)

genheaders: $(DOCTOKGENHEADERS)
