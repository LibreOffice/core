##########################################################################
#
#  Effective License of whole file:
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License version 2.1, as published by the Free Software Foundation.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#    MA  02111-1307  USA
#
#  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
#
#    The Contents of this file are made available subject to the terms of
#    the GNU Lesser General Public License Version 2.1
#
#    Copyright: 2000 by Sun Microsystems, Inc.
#
#  All parts contributed on or after August 2011:
#
#    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
#
#    The contents of this file are subject to the Mozilla Public License Version
#    1.1 (the "License"); you may not use this file except in compliance with
#    the License or as specified alternatively below. You may obtain a copy of
#    the License at http://www.mozilla.org/MPL/
#
#    Software distributed under the License is distributed on an "AS IS" basis,
#    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#    for the specific language governing rights and limitations under the
#    License.
#
#    Major Contributor(s):
#    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
#
#    All Rights Reserved.
#
#    For minor contributions see the git repository.
#
#    Alternatively, the contents of this file may be used under the terms of
#    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
#    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
#    instead of those above.
#
##########################################################################

PRJ=..$/..$/..

PRJNAME=postgresql
TARGET=postgresql
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
USE_DEFFILE=TRUE
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(BUILD_POSTGRESQL_SDBC)" == "YES"

#-------------------------------------------------------------------

# uno component naming scheme
DLLPRE=

PQ_SDBC_MAJOR=0
PQ_SDBC_MINOR=8
PQ_SDBC_MICRO=2
PQ_SDBC_VERSION=$(PQ_SDBC_MAJOR).$(PQ_SDBC_MINOR).$(PQ_SDBC_MICRO)

.IF "$(SYSTEM_POSTGRESQL)"=="YES"
LIBPQ_LINK=-lpq
.ELSE #SYSTEM_POSTGRESQL==NO
.IF "$(GUI)$(COM)"=="WNTMSC"
LIBPQ_LINK=$(OUTDIR)/lib/libpq.lib ws2_32.lib secur32.lib advapi32.lib shell32.lib
.ELSE
LIBPQ_LINK=$(OUTDIR)/lib/libpq.a
.ENDIF
POSTGRESQL_INC=-I$(OUTDIR)/inc/postgresql
POSTGRESQL_LIB=
.ENDIF

CFLAGS+=$(POSTGRESQL_INC) \
    -DPQ_SDBC_MAJOR=$(PQ_SDBC_MAJOR) \
    -DPQ_SDBC_MINOR=$(PQ_SDBC_MINOR) \
    -DPQ_SDBC_MICRO=$(PQ_SDBC_MICRO)

SHL1TARGET=postgresql-sdbc.uno
LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES= \
        $(SLO)$/pq_driver.obj

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)$/src$/reg-component.map

SHL2TARGET=postgresql-sdbc-impl.uno
LIB2TARGET=$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES= \
        $(SLO)$/pq_connection.obj \
        $(SLO)$/pq_statement.obj \
        $(SLO)$/pq_resultset.obj \
        $(SLO)$/pq_preparedstatement.obj \
        $(SLO)$/pq_resultsetmetadata.obj \
        $(SLO)$/pq_databasemetadata.obj \
        $(SLO)$/pq_sequenceresultset.obj \
        $(SLO)$/pq_baseresultset.obj \
        $(SLO)$/pq_statics.obj \
        $(SLO)$/pq_xtable.obj \
        $(SLO)$/pq_xcontainer.obj \
        $(SLO)$/pq_xbase.obj \
        $(SLO)$/pq_xtables.obj \
        $(SLO)$/pq_xcolumns.obj \
        $(SLO)$/pq_xcolumn.obj \
        $(SLO)$/pq_tools.obj \
        $(SLO)$/pq_xkey.obj \
        $(SLO)$/pq_xkeys.obj \
        $(SLO)$/pq_xkeycolumn.obj \
        $(SLO)$/pq_xkeycolumns.obj \
        $(SLO)$/pq_xuser.obj \
        $(SLO)$/pq_xusers.obj \
        $(SLO)$/pq_xview.obj \
        $(SLO)$/pq_xviews.obj \
        $(SLO)$/pq_xindex.obj \
        $(SLO)$/pq_xindexes.obj \
        $(SLO)$/pq_xindexcolumn.obj \
        $(SLO)$/pq_xindexcolumns.obj \
        $(SLO)$/pq_updateableresultset.obj \
        $(SLO)$/pq_fakedupdateableresultset.obj \
        $(SLO)$/pq_array.obj	\
        $(SLO)$/pq_sequenceresultsetmetadata.obj


SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)		\
        $(SALHELPERLIB)		\
        $(POSTGRESQL_LIB)	\
        $(LIBPQ_LINK)	\
        $(OPENSSLLIB)	\
        $(LDAPSDKLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=$(SOLARENV)$/src$/reg-component.map


SLOFILES=       $(LIB1OBJFILES) $(LIB2OBJFILES)


DRIVERNAME=postgresql-sdbc.oxt
ALLTAR : $(DLLDEST)$/$(DRIVERNAME)

.ENDIF # BUILD_POSTGRESQL_SDBC

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(GUI)" == "UNX"
INI_EXT=rc
.ELSE
INI_EXT=.ini
.ENDIF

$(DLLDEST)$/$(SHL1TARGET)$(INI_EXT): $(SHL1TARGET)
    +cp $? $@

$(DLLDEST)$/$(DRIVERNAME): \
        $(DLLDEST)$/META-INF$/manifest.xml \
        $(DLLDEST)$/description.xml \
        $(DLLDEST)$/description/description_en-US.txt \
        $(DLLDEST)$/postgresql.xcu \
        $(DLLDEST)$/$(SHL1TARGET)$(DLLPOST) \
        $(DLLDEST)$/$(SHL2TARGET)$(DLLPOST) \
        $(DLLDEST)$/$(SHL1TARGET)$(INI_EXT)
    +cd $(DLLDEST) && \
        zip -r  $(DRIVERNAME) \
            META-INF$/manifest.xml \
            $(SHL1TARGET)$(DLLPOST) \
            $(SHL2TARGET)$(DLLPOST) \
            $(SHL1TARGET)$(INI_EXT) \
            description.xml \
            description/description_en-US.txt \
            postgresql.xcu

$(DLLDEST)$/description/% : description/%
    mkdir -p $(DLLDEST)$/description
    +cp $? $@

$(DLLDEST)$/META-INF$/manifest.xml : manifest.xml
    mkdir -p $(DLLDEST)$/META-INF
    +cp $? $@

$(DLLDEST)$/description.xml : description.xml
    +sed -e 's/@EXTENSION_VERSION@/$(PQ_SDBC_VERSION)/' < $? > $@

$(DLLDEST)$/postgresql.xcu : postgresql.xcu
    rm -f $@
    cat postgresql.xcu > $@

strip :
.IF "$(GUI)"!="WNT"
    strip $(DLLDEST)$/$(SHL1TARGET)$(DLLPOST) $(DLLDEST)$/$(SHL2TARGET)$(DLLPOST)
.ENDIF
