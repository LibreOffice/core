#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.6 $
#
#  last change: $Author: obo $ $Date: 2008-01-10 12:16:36 $
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
TARGET=resourcemodel
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION


# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/qnametostr.obj \
    $(SLO)$/sprmcodetostr.obj \
    $(SLO)$/resourcemodel.obj \
    $(SLO)$/WW8Analyzer.obj

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ODIAPILIB=-lodiapi
RTFTOKLIB=-lrtftok
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
.ELIF "$(GUI)"=="WNT"
ODIAPILIB=$(LB)$/iodiapi.lib
RTFTOKLIB=$(LB)$/irtftok.lib
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)
#SHL1USE_EXPORTS=name
SHL1USE_EXPORTS=ordinal

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

RESOURCEMODELCXXOUTDIR=$(MISC)
DOCTOKHXXOUTDIR=$(INCCOM)/doctok
OOXMLHXXOUTDIR=$(INCCOM)/ooxml

OOXMLMODEL=..$/ooxml$/model.xml
OOXMLPREPROCESSXSL=..$/ooxml$/modelpreprocess.xsl
OOXMLQNAMETOSTRXSL=..$/ooxml$/qnametostr.xsl
OOXMLANALYZERXSL=..$/ooxml$/analyzer.xsl
OOXMLRESOURCEIDSXSL=..$/ooxml$/resourceids.xsl
OOXMLRESOURCESTOOLSXSL=..$/ooxml$/resourcestools.xsl
DOCTOKMODEL=..$/doctok$/resources.xmi
DOCTOKQNAMETOSTRXSL=..$/doctok$/qnametostr.xsl
DOCTOKANALYZERXSL=..$/doctok$/analyzer.xsl
DOCTOKSPRMCODETOSTRXSL=..$/doctok$/sprmcodetostr.xsl
DOCTOKRESOURCEIDSXSL=..$/doctok$/resourceids.xsl
DOCTOKSPRMIDSXSL=..$/doctok$/sprmids.xsl
DOCTOKRESOURCETOOLS=..$/doctok$/resourcetools.xsl

MODELPROCESSED=$(MISC)$/model_preprocessed.xml
QNAMETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/qnametostr.cxx
SPRMCODETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.cxx
DOCTOKRESOURCEIDSHXX=$(DOCTOKHXXOUTDIR)$/resourceids.hxx
SPRMIDSHXX=$(DOCTOKHXXOUTDIR)$/sprmids.hxx
OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

.IF "$(SYSTEM_LIBXSLT)"=="NO"
XSLTPROC=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/xsltproc
.ELSE
XSLTPROC=xsltproc
.ENDIF

GENERATEDHEADERS=$(DOCTOKRESOURCEIDSHXX) $(OOXMLRESOURCEIDSHXX) $(SPRMIDSHXX)
GENERATEDFILES=$(GENERATEDHEADERS) $(QNAMETOSTRCXX) $(SPRMCODETOSTRCXX) $(MODELPROCESSED)

$(QNAMETOSTRCXX): $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED) $(DOCTOKQNAMETOSTRXSL) qnametostrheader qnametostrfooter $(OOXMLRESOURCESTOOLSXSL) $(DOCTOKRESOURCETOOLS)
    $(TYPE) qnametostrheader > $@
    $(XSLTPROC) $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED) >> $@
    $(XSLTPROC) $(DOCTOKQNAMETOSTRXSL) $(DOCTOKMODEL) >> $@
    $(TYPE) qnametostrfooter >> $@

$(SPRMCODETOSTRCXX): sprmcodetostrheader sprmcodetostrfooter $(DOCTOKSPRMCODETOSTRXSL) $(DOCTOKMODEL)
    $(TYPE) sprmcodetostrheader > $(SPRMCODETOSTRCXX) 
    $(XSLTPROC) $(DOCTOKSPRMCODETOSTRXSL) $(DOCTOKMODEL) >> $(SPRMCODETOSTRCXX)
    $(TYPE) sprmcodetostrfooter >> $(SPRMCODETOSTRCXX)

$(SLO)$/sprmcodetostr.obj: $(SPRMCODETOSTRCXX) $(GENERATEDHEADERS)
$(SLO)$/qnametostr.obj: $(QNAMETOSTRCXX) $(GENERATEDHEADERS)
$(SLO)$/WW8Analyzer.obj: $(GENERATEDHEADERS)
$(SLO)$/resourcemodel.obj: $(GENERATEDHEADERS)

$(DOCTOKHXXOUTDIR):
    $(MKDIRHIER) $@

$(DOCTOKRESOURCEIDSHXX): $(DOCTOKHXXOUTDIR) $(DOCTOKRESOURCETOOLS) $(DOCTOKRESOURCEIDSXSL)
    $(XSLTPROC) $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL) > $@

$(OOXMLHXXOUTDIR):
    $(MKDIRHIER) $@

$(MODELPROCESSED): $(OOXMLMODEL) $(OOXMLRESOURCESTOOLSXSL) $(OOXMLPREPROCESSXSL)
    $(XSLTPROC) $(OOXMLPREPROCESSXSL) $(OOXMLMODEL) > $@

$(OOXMLRESOURCEIDSHXX): $(OOXMLHXXOUTDIR) $(OOXMLRESOURCESTOOLSXSL) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED)
    $(XSLTPROC) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED) > $@

$(SPRMIDSHXX): $(DOCTOKHXXOUTDIR) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    $(XSLTPROC) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL) > $@

.PHONY: genclean genmake

genclean: 
    rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)
