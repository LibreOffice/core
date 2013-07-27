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
    vml-officeDrawing \
    vml-wordprocessingDrawing

# --- Files --------------------------------------------------------

SLOFACTORIESNAMESPACES= \
    $(SLO)$/OOXMLFactory_{$(NAMESPACES)}.obj

SLOFILES= \
    $(SLOFACTORIESNAMESPACES) \
    $(SLOFASTNAMESPACES) \
    $(SLO)$/OOXMLFactory_generated.obj \
    $(SLO)$/OOXMLFactory.obj \
    $(SLO)$/OOXMLBinaryObjectReference.obj\
    $(SLO)$/OOXMLFactory_values.obj \
    $(SLO)$/OOXMLPropertySetImpl.obj \
    $(SLO)$/OOXMLParserState.obj \
    $(SLO)$/Handler.obj \
    $(SLO)$/OOXMLDocumentImpl.obj \
    $(SLO)$/OOXMLStreamImpl.obj \
    $(SLO)$/OOXMLFastDocumentHandler.obj \
    $(SLO)$/OOXMLFastContextHandler.obj \
    $(SLO)$/OOXMLFastTokenHandler.obj

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

OOXMLFACTORYCXXS=$(OOXMLCXXOUTDIR)$/OOXMLFactory_{$(NAMESPACES)}.cxx
OOXMLFACTORYHXXS=$(OOXMLHXXOUTDIR)$/OOXMLFactory_{$(NAMESPACES)}.hxx

OOXMLMODEL=model.xml
OOXMLPREPROCESSXSL=modelpreprocess.xsl
OOXMLFACTORYTOOLSXSL=factorytools.xsl
OOXMLRESORUCETOKENSXSL=resourcestokens.xsl
OOXMLFASTTOKENSXSL=fasttokens.xsl
OOXMLRESOURCESIMPLXSL=resourcesimpl.xsl
OOXMLNAMESPACEIDSXSL=namespaceids.xsl
OOXMLFACTORYVALUESXSL=factory_values.xsl
OOXMLFACTORYVALUESIMPLXSL=factoryimpl_values.xsl
OOXMLRESOURCEIDSXSL=resourceids.xsl
OOXMLGPERFFASTTOKENXSL=gperffasttokenhandler.xsl

OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

TOKENXML=$(OOXMLCXXOUTDIR)$/token.xml
TOKENXMLTMP=$(OOXMLCXXOUTDIR)$/token.tmp

OOXMLFACTORYGENERATEDHXX=$(OOXMLHXXOUTDIR)$/OOXMLFactory_generated.hxx
OOXMLFACTORYGENERATEDCXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_generated.cxx
OOXMLFASTTOKENSHXX=$(OOXMLHXXOUTDIR)$/OOXMLFastTokens.hxx
OOXMLNAMESPACEIDSHXX=$(OOXMLHXXOUTDIR)$/OOXMLnamespaceids.hxx
OOXMLFACTORYVALUESHXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_values.hxx
OOXMLFACTORYVALUESCXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_values.cxx
GPERFFASTTOKENHXX=$(OOXMLHXXOUTDIR)$/gperffasttoken.hxx
MODELPROCESSED=$(MISC)$/model_preprocessed.xml

OOXMLGENHEADERS= \
    $(OOXMLFACTORYGENERATEDHXX) \
    $(OOXMLFACTORYHXXS) \
    $(OOXMLFASTTOKENSHXX) \
    $(OOXMLFACTORYVALUESHXX) \
    $(GPERFFASTTOKENHXX) \
    $(OOXMLRESOURCEIDSHXX) \
    $(OOXMLNAMESPACEIDSHXX)

GENERATEDFILES= \
    $(OOXMLGENHEADERS) \
    $(OOXMLFACTORYGENERATEDCXX) \
    $(OOXMLFACTORYCXXS) \
    $(OOXMLFACTORYVALUESCXX) \
    $(TOKENXMLTMP) \
    $(TOKENXML)

$(TOKENXMLTMP): $(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/tokens.txt
    @$(TYPE) $(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/tokens.txt | $(SED) "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@

$(TOKENXML): tokenxmlheader $(TOKENXMLTMP) tokenxmlfooter
    @$(TYPE) tokenxmlheader $(TOKENXMLTMP) tokenxmlfooter > $@

$(OOXMLHXXOUTDIRCREATED):
    $(MKDIRHIER) $(OOXMLHXXOUTDIR)
    @$(TOUCH) $@

$(OOXMLGENHEADERS): $(OOXMLHXXOUTDIRCREATED)

$(OOXMLFASTTOKENSHXX): $(OOXMLFASTTOKENSXSL) $(TOKENXML)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFASTTOKENSXSL) $(TOKENXML) > $@

$(OOXMLFACTORYGENERATEDHXX): factoryinc.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYGENERATEDCXX): factoryimpl.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYGENERATEDCXX): $(MODELPROCESSED)

$(OOXMLFACTORYGENERATEDHXX): $(MODELPROCESSED)

$(OOXMLFACTORYCXXS): $(MODELPROCESSED)

$(OOXMLFACTORYHXXS): $(MODELPROCESSED)

$(OOXMLCXXOUTDIR)$/OOXMLFactory%.cxx: factoryimpl_ns.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) --stringparam file $@ $< $(MODELPROCESSED) > $@

$(OOXMLHXXOUTDIR)$/OOXMLFactory%.hxx: factory_ns.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) --stringparam file $@ $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYVALUESHXX): $(OOXMLFACTORYVALUESXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFACTORYVALUESXSL) $(MODELPROCESSED) > $@

$(OOXMLFACTORYVALUESCXX): $(OOXMLFACTORYVALUESIMPLXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFACTORYVALUESIMPLXSL) $(MODELPROCESSED) > $@

$(OOXMLRESOURCEIDSHXX):  $(OOXMLHXXOUTDIRCREATED) $(OOXMLRESOURCEIDSXSL) \
    $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED) > $@

$(OOXMLNAMESPACEIDSHXX):  $(OOXMLHXXOUTDIRCREATED) $(OOXMLNAMESPACEIDSXSL) \
    $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLNAMESPACEIDSXSL) $(MODELPROCESSED) > $@

$(GPERFFASTTOKENHXX): $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED) | tr -d '\r' | $(GPERF) -I -t -E -S1 -c -G -LC++ > $@

$(SLOFACTORIESNAMESPACES): $(OOXMLFACTORYSCXXS) $(OOXMLGENHEADERS)

$(GENERATEDFILES): $(OOXMLFACTORYTOOLSXSL)

$(SLOFILES): $(OOXMLGENHEADERS)

$(SLO)/OOXMLFactory_values.obj: $(OOXMLFACTORYVALUESCXX) $(OOXMLFACTORYVALUESHXX)

$(SLO)$/OOXMLFactory_generated.obj: $(OOXMLFACTORYGENERATEDCXX) $(OOXMLGENHEADERS)

.PHONY: genclean genmake genheaders

genclean:
    rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)

genheaders: $(GENHEADERS)
