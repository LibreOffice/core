#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.8 $
#
#  last change: $Author: hr $ $Date: 2007-08-06 12:58:42 $
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
TARGET=ooxml
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION -DDEBUG_OOXML_ELEMENT


# --- Files --------------------------------------------------------

# SunStudio 8 x86: the optimizer pass (ube) uses up all available memory
# and exits with a "out of memory" condition on this file (TokenMapInit.cxx). 
# Remove if TokenMapInit.cxx is either restructured or a new compiler version
# is used.
.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES+=$(SLO)$/TokenMapInit.obj
.ENDIF

SLOFILES= \
    $(SLO)$/OOXMLresources.obj \
    $(SLO)$/OOXMLvalues.obj \
    $(SLO)$/TokenMapInit.obj \
    $(SLO)$/OOXMLPropertySetImpl.obj \
    $(SLO)$/OOXMLContext.obj \
    $(SLO)$/NamespaceMapper.obj \
    $(SLO)$/TokenMap.obj \
    $(SLO)$/OOXMLSaxHandler.obj \
    $(SLO)$/OOXMLDocumentImpl.obj \
    $(SLO)$/OOXMLStreamImpl.obj


SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="WNT"
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
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

OOXMLCXXOUTDIR=.
OOXMLHXXOUTDIR=.
WRITERFILTERINCDIR=..$/..$/inc

OOXMLMODEL=model.xml
OOXMLPREPROCESSXSL=modelpreprocess.xsl
OOXMLRESOURCESTOOLSXSL=resourcestools.xsl
OOXMLRESORUCETOKENSXSL=resourcestokens.xsl
OOXMLRESOURCESXSL=resources.xsl
OOXMLRESOURCESIMPLXSL=resourcesimpl.xsl
OOXMLTOKENMAPINITXSL=tokenmapinit.xsl
OOXMLVALUESIMPLXSL=valuesimpl.xsl
OOXMLRESOURCEIDSXSL=resourceids.xsl
OOXMLGPERFATTRIBUTESXSL=gperfattributes.xsl
OOXMLGPERFELEMENTSXSL=gperfelements.xsl

OOXMLRESOURCEIDSHXX=$(WRITERFILTERINCDIR)$/ooxml$/resourceids.hxx

OOXMLTOKENSHXX=$(OOXMLHXXOUTDIR)$/OOXMLtokens.hxx
OOXMLRESOURCESHXX=$(OOXMLHXXOUTDIR)$/OOXMLresources.hxx
OOXMLRESOURCESCXX=$(OOXMLCXXOUTDIR)$/OOXMLresources.cxx
TOKENMAPINITCXX=$(OOXMLCXXOUTDIR)$/TokenMapInit.cxx
OOXMLVALUESCXX=$(OOXMLCXXOUTDIR)$/OOXMLvalues.cxx
GPERFATTRIBUTESHXX=$(OOXMLHXXOUTDIR)$/gperfattributes.hxx
GPERFELEMENTSHXX=$(OOXMLHXXOUTDIR)$/gperfelements.hxx
MODELPROCESSED=$(MISC)$/model_preprocessed.xml

OOXMLGENHEADERS=$(OOXMLRESOURCESHXX) $(OOXMLTOKENSHXX) $(GPERFATTRIBUTESHXX) $(GPERFELEMENTSHXX)

XALANJAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/xalan.jar
XALAN=$(JAVA) -jar $(XALANJAR)

$(MODELPROCESSED): $(OOXMLPREPROCESSXSL) $(OOXMLMODEL) $(OOXMLRESOURCESTOOLSXSL)
    $(XALAN) -xsl $(OOXMLPREPROCESSXSL) -in $(OOXMLMODEL) > $(MODELPROCESSED)

$(OOXMLTOKENSHXX): $(OOXMLRESORUCETOKENSXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLRESORUCETOKENSXSL) -in $(MODELPROCESSED) > $(OOXMLTOKENSHXX)
$(OOXMLRESOURCESHXX): $(OOXMLRESOURCESXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLRESOURCESXSL) -in $(MODELPROCESSED) > $(OOXMLRESOURCESHXX)

$(OOXMLRESOURCESCXX): $(OOXMLRESOURCESIMPLXSL) $(MODELPROCESSED) $(OOXMLRESOURCESHXX)
    $(XALAN) -xsl $(OOXMLRESOURCESIMPLXSL) -in $(MODELPROCESSED) > $(OOXMLRESOURCESCXX)

$(TOKENMAPINITCXX): $(OOXMLTOKENMAPINITXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLTOKENMAPINITXSL) -in $(MODELPROCESSED) > $(TOKENMAPINITCXX)

$(OOXMLVALUESCXX): $(OOXMLVALUESIMPLXSL) $(MODELPROCESSED) $(OOXMLRESOURCESHXX)
    $(XALAN) -xsl $(OOXMLVALUESIMPLXSL) -in $(MODELPROCESSED) > $(OOXMLVALUESCXX)

$(OOXMLRESOURCEIDSHXX):  $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLRESOURCEIDSXSL) -in $(MODELPROCESSED) > $(OOXMLRESOURCEIDSHXX)

$(GPERFELEMENTSHXX): $(OOXMLGPERFELEMENTSXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLGPERFELEMENTSXSL) -in $(MODELPROCESSED) | gperf -t -E -S1 -LC++ > $(GPERFELEMENTSHXX)

$(GPERFATTRIBUTESHXX): $(OOXMLGPERFATTRIBUTESXSL) $(MODELPROCESSED)
    $(XALAN) -xsl $(OOXMLGPERFATTRIBUTESXSL) -in $(MODELPROCESSED) | gperf -t -E -S1 -LC++ > $(GPERFATTRIBUTESHXX)

$(SLO)/OOXMLresources.obj: $(OOXMLRESOURCESCXX)

$(SLO)/TokenMapInit.obj: $(TOKENMAPINITCXX)

$(SLO)/OOXMLvalues.obj: $(OOXMLVALUESCXX)

$(SLO)$/NamespaceMapper.obj: $(OOXMLGENHEADERS)
$(SLO)$/OOXMLContext.obj: $(OOXMLGENHEADERS)
$(SLO)$/OOXMLDocumentImpl.obj: $(OOXMLGENHEADERS)
$(SLO)$/OOXMLPropertySetImpl.obj: $(OOXMLGENHEADERS)
$(SLO)$/OOXMLSaxHandler.obj: $(OOXMLGENHEADERS)
$(SLO)$/OOXMLStreamImpl.obj: $(OOXMLGENHEADERS)
$(SLO)$/TokenMap.obj: $(OOXMLGENHEADERS)
.ENDIF
