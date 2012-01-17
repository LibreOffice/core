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
TARGET=resourcemodel
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION


# --- Files --------------------------------------------------------

# work around gcc taking hours and/or OOM'ing on this file
NOOPTFILES= \
    $(SLO)$/qnametostr.obj

SLOFILES= \
    $(SLO)$/Fraction.obj \
    $(SLO)$/LoggedResources.obj \
    $(SLO)$/Protocol.obj \
    $(SLO)$/ResourceModelHelper.obj \
    $(SLO)$/TagLogger.obj \
    $(SLO)$/WW8Analyzer.obj \
    $(SLO)$/XPathLogger.obj \
    $(SLO)$/qnametostr.obj \
    $(SLO)$/resourcemodel.obj \
    $(SLO)$/sprmcodetostr.obj \
    $(SLO)$/util.obj \

# FreeBSD/Linux 64-bit: compiler (gcc 4.2.x) fails with 'out of memory'
.IF "$(OUTPATH)"=="unxfbsdx" || "$(OUTPATH)"=="unxfbsdi" || "$(OUTPATH)"=="unxlngx6"
NOOPTFILES= \
    $(SLO)$/qnametostr.obj
.ENDIF

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
RTFTOKLIB=-lrtftok
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
.ELIF "$(GUI)"=="WNT"
RTFTOKLIB=$(LB)$/irtftok.lib
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

RESOURCEMODELCXXOUTDIR=$(MISC)
DOCTOKHXXOUTDIR=$(INCCOM)$/doctok
OOXMLHXXOUTDIR=$(INCCOM)$/ooxml

DOCTOKHXXOUTDIRCREATED=$(DOCTOKHXXOUTDIR)$/created
OOXMLHXXOUTDIRCREATED=$(OOXMLHXXOUTDIR)$/created

OOXMLMODEL=..$/ooxml$/model.xml
OOXMLPREPROCESSXSL=..$/ooxml$/modelpreprocess.xsl
OOXMLPREPROCESSXSLCOPIED=$(MISC)$/modelpreprocess.xsl
OOXMLQNAMETOSTRXSL=..$/ooxml$/qnametostr.xsl
OOXMLANALYZERXSL=..$/ooxml$/analyzer.xsl
OOXMLRESOURCEIDSXSL=..$/ooxml$/resourceids.xsl
OOXMLFACTORYTOOLSXSL=..$/ooxml$/factorytools.xsl
DOCTOKMODEL=..$/doctok$/resources.xmi
DOCTOKQNAMETOSTRXSL=..$/doctok$/qnametostr.xsl
DOCTOKANALYZERXSL=..$/doctok$/analyzer.xsl
DOCTOKSPRMCODETOSTRXSL=..$/doctok$/sprmcodetostr.xsl
DOCTOKRESOURCEIDSXSL=..$/doctok$/resourceids.xsl
DOCTOKSPRMIDSXSL=..$/doctok$/sprmids.xsl
DOCTOKRESOURCETOOLS=..$/doctok$/resourcetools.xsl

NSPROCESS=namespace_preprocess.pl

MODELPROCESSED=$(MISC)$/model_preprocessed.xml

QNAMETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/qnametostr.cxx
OOXMLQNAMETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/OOXMLqnameToStr.tmp
DOCTOKQNAMETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/DOCTOKqnameToStr.tmp
SPRMCODETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.cxx
SPRMCODETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.tmp
DOCTOKRESOURCEIDSHXX=$(DOCTOKHXXOUTDIR)$/resourceids.hxx
SPRMIDSHXX=$(DOCTOKHXXOUTDIR)$/sprmids.hxx
OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

NSXSL=$(MISC)$/namespacesmap.xsl
NAMESPACESTXT=$(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/namespaces.txt

GENERATEDHEADERS=$(DOCTOKRESOURCEIDSHXX) $(OOXMLRESOURCEIDSHXX) $(SPRMIDSHXX)
GENERATEDFILES= \
    $(GENERATEDHEADERS) \
    $(QNAMETOSTRCXX) \
    $(SPRMCODETOSTRCXX) \
    $(MODELPROCESSED) \
    $(OOXMLQNAMETOSTRTMP) \
    $(DOCTOKQNAMETOSTRTMP) \
    $(SPRMCODETOSTRTMP)

$(OOXMLQNAMETOSTRTMP): $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(XSLTPROC) $(OOXMLQNAMETOSTRXSL:s!\!/!) $(MODELPROCESSED) > $@

$(DOCTOKQNAMETOSTRTMP): $(DOCTOKQNAMETOSTRXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
    $(XSLTPROC) $(DOCTOKQNAMETOSTRXSL:s!\!/!) $(DOCTOKMODEL) > $@

$(QNAMETOSTRCXX): $(OOXMLQNAMETOSTRTMP) $(DOCTOKQNAMETOSTRTMP) qnametostrheader qnametostrfooter $(OOXMLFACTORYTOOLSXSL) $(DOCTOKRESOURCETOOLS)
    @$(TYPE) qnametostrheader $(OOXMLQNAMETOSTRTMP) $(DOCTOKQNAMETOSTRTMP) qnametostrfooter > $@

$(SPRMCODETOSTRTMP): $(DOCTOKSPRMCODETOSTRXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
    $(XSLTPROC) $(DOCTOKSPRMCODETOSTRXSL:s!\!/!) $(DOCTOKMODEL) > $@

$(SPRMCODETOSTRCXX): sprmcodetostrheader $(SPRMCODETOSTRTMP) sprmcodetostrfooter
    @$(TYPE) $< > $@

$(SLO)$/sprmcodetostr.obj: $(SPRMCODETOSTRCXX)
$(SLO)$/qnametostr.obj: $(QNAMETOSTRCXX)

$(SLOFILES): $(GENERATEDHEADERS)

$(DOCTOKHXXOUTDIRCREATED):
    @$(MKDIRHIER) $(DOCTOKHXXOUTDIR)
    @$(TOUCH) $@

$(DOCTOKRESOURCEIDSHXX): $(DOCTOKHXXOUTDIRCREATED) $(DOCTOKRESOURCETOOLS) $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKRESOURCEIDSXSL:s!\!/!) $(DOCTOKMODEL) > $@

$(OOXMLHXXOUTDIRCREATED):
    @$(MKDIRHIER) $(OOXMLHXXOUTDIR)
    @$(TOUCH) $@

$(OOXMLPREPROCESSXSLCOPIED): $(OOXMLPREPROCESSXSL)
    @$(COPY) $(OOXMLPREPROCESSXSL) $@

$(NSXSL) : $(OOXMLMODEL) $(NAMESPACESTXT) $(NSPROCESS)
    @$(PERL) $(NSPROCESS) $(NAMESPACESTXT) > $@

$(MODELPROCESSED): $(NSXSL) $(OOXMLPREPROCESSXSLCOPIED) $(OOXMLMODEL)
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(XSLTPROC) $(NSXSL) $(OOXMLMODEL) > $@

$(OOXMLRESOURCEIDSHXX): $(OOXMLHXXOUTDIRCREATED) $(OOXMLFACTORYTOOLSXSL) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(OOXMLRESOURCEIDSXSL:s!\!/!) $(MODELPROCESSED) > $@

$(SPRMIDSHXX): $(DOCTOKHXXOUTDIRCREATED) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKSPRMIDSXSL:s!\!/!) $(DOCTOKMODEL) > $@

.PHONY: genclean genmake gendirs

genclean: 
    rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)

