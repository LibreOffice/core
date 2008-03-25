#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.14 $
#
#  last change: $Author: obo $ $Date: 2008-03-25 13:55:16 $
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
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820

NAMESPACES= \
    wml \
    dml-stylesheet \
    dml-styleDefaults \
    dml-shape3DLighting \
    dml-shape3DScene \
    dml-shape3DStyles \
    dml-shape3DCamera \
    dml-baseStylesheet \
    dml-textCharacter \
    dml-shapeEffects \
    dml-shapeLineProperties \
    dml-shapeProperties \
    dml-baseTypes \
    dml-documentProperties \
    dml-graphicalObject \
    dml-shapeGeometry \
    dml-wordprocessingDrawing \
    sml-customXmlMappings \
    shared-math \
    shared-relationshipReference \
    dml-chartDrawing \
    vml-main \
    vml-officeDrawing

# --- Files --------------------------------------------------------

SLOFASTNAMESPACES= \
    $(SLO)$/OOXMLfastresources_{$(NAMESPACES)}.obj

SLOFILES= \
    $(SLOFASTNAMESPACES) \
    $(SLO)$/OOXMLBinaryObjectReference.obj\
    $(SLO)$/OOXMLfastresources.obj \
    $(SLO)$/OOXMLvalues.obj \
    $(SLO)$/OOXMLPropertySetImpl.obj \
    $(SLO)$/OOXMLParserState.obj \
    $(SLO)$/Handler.obj \
    $(SLO)$/OOXMLDocumentImpl.obj \
    $(SLO)$/OOXMLStreamImpl.obj \
    $(SLO)$/OOXMLFastDocumentHandler.obj \
    $(SLO)$/OOXMLFastContextHandler.obj \
    $(SLO)$/OOXMLFastTokenHandler.obj 


SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
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
    $(COMPHELPERLIB) \
    $(RESOURCEMODELLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.PHONY : test

test:
    echo $(SLOFILES)

OOXMLCXXOUTDIR=$(MISC)
OOXMLHXXOUTDIR=$(INCCOM)/ooxml
OOXMLHXXOUTDIRCREATED=$(OOXMLHXXOUTDIR)/created

OOXMLFASTRESOURCECXXS=$(OOXMLCXXOUTDIR)$/OOXMLfastresources_{$(NAMESPACES)}.cxx
OOXMLFASTRESOURCEHXXS=$(OOXMLHXXOUTDIR)$/OOXMLfastresources_{$(NAMESPACES)}.hxx

OOXMLMODEL=model.xml
OOXMLPREPROCESSXSL=modelpreprocess.xsl
OOXMLRESOURCESTOOLSXSL=resourcestools.xsl
OOXMLRESORUCETOKENSXSL=resourcestokens.xsl
OOXMLFASTTOKENSXSL=fasttokens.xsl
OOXMLFASTRESOURCESXSL=fastresources.xsl
OOXMLRESOURCESIMPLXSL=resourcesimpl.xsl
OOXMLFASTRESOURCESIMPLXSL=fastresourcesimpl.xsl
OOXMLVALUESXSL=values.xsl
OOXMLVALUESIMPLXSL=valuesimpl.xsl
OOXMLRESOURCEIDSXSL=resourceids.xsl
OOXMLGPERFFASTTOKENXSL=gperffasttokenhandler.xsl

OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

OOXMLFASTTOKENSHXX=$(OOXMLHXXOUTDIR)$/OOXMLFastTokens.hxx
OOXMLFASTRESOURCESHXX=$(OOXMLHXXOUTDIR)$/OOXMLfastresources.hxx
OOXMLFASTRESOURCESCXX=$(OOXMLCXXOUTDIR)$/OOXMLfastresources.cxx
OOXMLVALUESHXX=$(OOXMLCXXOUTDIR)$/OOXMLvalues.hxx
OOXMLVALUESCXX=$(OOXMLCXXOUTDIR)$/OOXMLvalues.cxx
GPERFFASTTOKENHXX=$(OOXMLHXXOUTDIR)$/gperffasttoken.hxx
MODELPROCESSED=$(MISC)$/model_preprocessed.xml

OOXMLGENHEADERS= \
    $(OOXMLFASTRESOURCESHXX) \
    $(OOXMLFASTTOKENSHXX) \
    $(OOXMLVALUESHXX) \
    $(OOXMLFASTRESOURCEHXXS) \
    $(GPERFFASTTOKENHXX) \
    $(OOXMLRESOURCEIDSHXX)

GENERATEDFILES= \
    $(OOXMLGENHEADERS) \
    $(MODELPROCESSED) \
    $(OOXMLFASTRESOURCECXXS) \
    $(OOXMLVALUESCXX)

.IF "$(SYSTEM_LIBXSLT)"=="NO"
XSLTPROC=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/xsltproc
.ELSE
XSLTPROC=xsltproc
.ENDIF

$(MODELPROCESSED): $(OOXMLPREPROCESSXSL) $(OOXMLMODEL)
    $(XSLTPROC) $(OOXMLPREPROCESSXSL) $(OOXMLMODEL) > $@

$(OOXMLHXXOUTDIRCREATED):
    $(MKDIRHIER) $(OOXMLHXXOUTDIR)
    $(TOUCH) $@

$(OOXMLGENHEADERS): $(OOXMLHXXOUTDIRCREATED)

$(OOXMLFASTTOKENSHXX): $(OOXMLFASTTOKENSXSL) $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLFASTTOKENSXSL) $(MODELPROCESSED) > $@

$(OOXMLFASTRESOURCESHXX): $(OOXMLFASTRESOURCESXSL) $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLFASTRESOURCESXSL) $(MODELPROCESSED) > $@

$(OOXMLFASTRESOURCESCXX): $(OOXMLFASTRESOURCEHXXS) \
    $(OOXMLFASTRESOURCESIMPLXSL) $(MODELPROCESSED) 
    $(XSLTPROC) $(OOXMLFASTRESOURCESIMPLXSL) $(MODELPROCESSED) > $@

$(OOXMLFASTRESOURCECXXS): $(MODELPROCESSED)

$(OOXMLFASTRESOURCEHXXS): $(MODELPROCESSED)

$(OOXMLCXXOUTDIR)$/OOXMLfastresources%.cxx: fastresourcesimpl%.xsl
    $(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLHXXOUTDIR)$/OOXMLfastresources%.hxx: fastresources%.xsl
    $(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLVALUESHXX): $(OOXMLVALUESXSL) $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLVALUESXSL) $(MODELPROCESSED) > $@

$(OOXMLVALUESCXX): $(OOXMLVALUESIMPLXSL) $(MODELPROCESSED) \
    $(OOXMLFASTRESOURCESHXX)
    $(XSLTPROC) $(OOXMLVALUESIMPLXSL) $(MODELPROCESSED) > $@

$(OOXMLRESOURCEIDSHXX):  $(OOXMLHXXOUTDIRCREATED) $(OOXMLRESOURCEIDSXSL) \
    $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED) > $@

$(GPERFFASTTOKENHXX): $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED) | tr -d '\r' | gperf -I -t -E -S1 -c -G -LC++ > $@

$(SLOFASTNAMESPACES): $(OOXMLFASTRESOURCESCXX) $(OOXMLGENHEADERS)

$(GENERATEDFILES): $(OOXMLRESOURCESTOOLSXSL)

$(SLOFILES): $(OOXMLGENHEADERS)

$(SLO)/OOXMLfastresources.obj: $(OOXMLFASTRESOURCESCXX) $(OOXMLGENHEADERS)

$(SLO)/OOXMLvalues.obj: $(OOXMLVALUESCXX) $(OOXMLVALUESHXX)

$(SLOFASTNAMESPACES): OOXMLFastHelper.hxx

.PHONY: genclean genmake genheaders

genclean:
    rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)

genheaders: $(OOXMLFASTRESOURCEHXXS)
