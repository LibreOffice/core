#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: hbrinkm $ $Date: 2007-06-15 09:31:01 $
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
    $(SLO)$/sprmcodetostr.obj

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
    $(COMPHELPERLIB) \
    $(DOCTOKLIB) \
    $(OOXMLLIB)
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

RESOURCEMODELCXXOUTDIR=.
WRITERFILTERINCDIR=..$/..$/inc

OOXMLMODEL=..$/ooxml$/model.xml
OOXMLPREPROCESSXSL=..$/ooxml$/modelpreprocess.xsl
OOXMLQNAMETOSTRXSL=..$/ooxml$/qnametostr.xsl
OOXMLRESOURCEIDSXSL=..$/ooxml$/resourceids.xsl
DOCTOKMODEL=..$/doctok$/resources.xmi
DOCTOKQNAMETOSTRXSL=..$/doctok$/qnametostr.xsl
DOCTOKSPRMCODETOSTRXSL=..$/doctok$/sprmcodetostr.xsl
DOCTOKRESOURCEIDSXSL=..$/doctok$/resourceids.xsl
DOCTOKSPRMIDSXSL=..$/doctok$/sprmids.xsl

OOXMLRESOURCEIDSHXX=$(WRITERFILTERINCDIR)$/ooxml$/resourceids.hxx
DOCTOKRESOURCEIDSHXX=$(WRITERFILTERINCDIR)$/doctok$/resourceids.hxx
RESOURCESPRMIDSHXX=$(WRITERFILTERINCDIR)$/resourcemodel$/sprmids.hxx

MODELPROCESSED=$(MISC)$/model_preprocessed
QNAMETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/qnametostr.cxx
SPRMCODETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.cxx

XALANJAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/xalan.jar
XALAN=$(JAVA) -jar $(XALANJAR)

$(MODELPROCESSED): $(OOXMLPREPROCESSXSL) $(OOXMLMODEL)
    $(XALAN) -xsl $(OOXMLPREPROCESSXSL) -in $(OOXMLMODEL) > $(MODELPROCESSED)

$(QNAMETOSTRCXX): $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED) $(DOCTOKQNAMETOSTRXSL) qnametostrheader qnametostrfooter $(DOCTOKRESOURCEIDSHXX) $(OOXMLRESOURCEIDSHXX)
    $(TYPE) qnametostrheader > $(QNAMETOSTRCXX) 
    $(XALAN) -xsl $(OOXMLQNAMETOSTRXSL) -in $(MODELPROCESSED) >> $(QNAMETOSTRCXX)
    $(XALAN) -xsl $(DOCTOKQNAMETOSTRXSL) -in $(DOCTOKMODEL) >> $(QNAMETOSTRCXX)
    $(TYPE) qnametostrfooter >> $(QNAMETOSTRCXX)

$(SLO)$/qnametostr.obj: $(QNAMETOSTRCXX)

$(SPRMCODETOSTRCXX): sprmcodetostrheader $(DOCTOKSPRMCODETOSTRXSL) $(DOCTOKMODEL) $(RESOURCESPRMIDSHXX)
    $(TYPE) sprmcodetostrheader > $(SPRMCODETOSTRCXX) 
    $(XALAN) -xsl $(DOCTOKSPRMCODETOSTRXSL) -in $(DOCTOKMODEL) >> $(SPRMCODETOSTRCXX)
    echo "}" >> $(SPRMCODETOSTRCXX)

$(SLO)$/sprmcodetostr.obj: $(SPRMCODETOSTRCXX)

$(DOCTOKRESOURCEIDSHXX): $(DOCTOKMODEL) $(DOCTOKRESOURCEIDSXSL)
    $(MKDIRHIER) $(INCCOM)$/doctok
    $(XALAN) -xsl $(DOCTOKRESOURCEIDSXSL) -in $(DOCTOKMODEL) > $(DOCTOKRESOURCEIDSHXX)

$(OOXMLRESOURCEIDSHXX): $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED)
    $(MKDIRHIER) $(INCCOM)$/ooxml
    $(XALAN) -xsl $(OOXMLRESOURCEIDSXSL) -in $(MODELPROCESSED) > $(OOXMLRESOURCEIDSHXX)

$(RESOURCESPRMIDSHXX): $(DOCTOKMODEL) $(DOCTOKSPRMIDSXSL)
    $(MKDIRHIER) $(INCCOM)$/resourcemodel
    $(XALAN) -xsl $(DOCTOKSPRMIDSXSL) -in $(DOCTOKMODEL) > $(RESOURCESPRMIDSHXX)

.ENDIF