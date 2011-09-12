#*************************************************************************
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#              2011 by Lionel Elie Mamane <lionel@mamane.lu>
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=postgresql
TARGET=postgresql
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
USE_DEFFILE=TRUE
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(SYSTEM_POSTGRESQL)" != "YES"
.INCLUDE :  $(SOLARINCDIR)$/postgresql/postgresql-version.mk
.ENDIF
#-------------------------------------------------------------------

# uno component naming scheme
DLLPRE=

PQ_SDBC_MAJOR=0
PQ_SDBC_MINOR=8
PQ_SDBC_MICRO=0
PQ_SDBC_VERSION=$(PQ_SDBC_MAJOR).$(PQ_SDBC_MINOR).$(PQ_SDBC_MICRO)
.IF "$(SYSTEM_POSTGRESQL)" == "YES"
POSTGRESQL_MAJOR:=$(shell @pg_config --version | awk '{ print $$2 }' | cut -d. -f1)
POSTGRESQL_MINOR:=$(shell @pg_config --version | awk '{ print $$2 }' | cut -d. -f2)
POSTGRESQL_MICRO:=$(shell @pg_config --version | awk '{ print $$2 }' | cut -d. -f3)
.ENDIF

.IF "$(SYSTEM_POSTGRESQL)" != "YES"
POSTGRESQL_INCLUDES=-I$(SOLARINCDIR)$/postgresql
.ELSE
POSTGRESQL_INCLUDES:=-I$(shell @pg_config --includedir)
.ENDIF

CFLAGS+=$(POSTGRESQL_INCLUDES) \
    -DPOSTGRESQL_MAJOR=$(POSTGRESQL_MAJOR) \
    -DPOSTGRESQL_MINOR=$(POSTGRESQL_MINOR) \
    -DPOSTGRESQL_MICRO=$(POSTGRESQL_MICRO) \
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

# use the static version
.IF "$(GUI)"=="WNT"
PQLIB=libpq.lib wsock32.lib advapi32.lib
.ELSE
PQLIB=-lpq -lcrypt
.ENDIF
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
        $(PQLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=$(SOLARENV)$/src$/reg-component.map


SLOFILES=       $(LIB1OBJFILES) $(LIB2OBJFILES)


DRIVERNAME=postgresql-sdbc-$(PQ_SDBC_VERSION).oxt
ALLTAR : $(DLLDEST)$/$(DRIVERNAME)

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
    +sed -s -e 's/@EXTENSION_VERSION@/$(PQ_SDBC_VERSION)/' < $? > $@

$(DLLDEST)$/postgresql.xcu : postgresql.xcu
    rm -f $@
    cat postgresql.xcu > $@

strip :
.IF "$(GUI)"!="WNT"
    strip $(DLLDEST)$/$(SHL1TARGET)$(DLLPOST) $(DLLDEST)$/$(SHL2TARGET)$(DLLPOST)
.ENDIF
