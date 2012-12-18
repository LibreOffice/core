#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..
PRJNAME=mysqlc

.INCLUDE : $(PRJ)$/version.mk

TARGET=mysqlc
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN = TRUE
LIBTARGET=NO
EXTENSIONNAME:=mysql-connector-ooo

EXTERNAL_WARNINGS_NOT_ERRORS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

#---- extension version
EXTENSION_VERSION_BASE=$(MYSQLC_VERSION_MAJOR).$(MYSQLC_VERSION_MINOR).$(MYSQLC_VERSION_MICRO)
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)

#---- extension title package name
EXTENSION_ZIPNAME:=$(EXTENSIONNAME)

#-------------------------------------------------------------------

# The headers delivered from C/C++ have a habit of putting a ";" after the closing bracked of a namespace, which
# yields a warning with Solaris' compiler. Until this is fixed in C/C++, silence this warning
.IF "$(OS)" == "SOLARIS"
    CFLAGSWARNCXX+=,wemptydecl
    CFLAGSWARNCXX:=$(CFLAGSWARNCXX:s/ ,/,/)
.ENDIF

#-------------------------------------------------------------------
.IF "$(ENABLE_MYSQLC)" != "YES"
@all:
    @echo "Mysql native driver extension build disabled."
.ENDIF

# use the static version
.IF "$(GUI)"=="WNT"
  MYSQL_LIBDIR=$(LIBMYSQL_PATH)$/lib
  MYSQL_INCDIR=$(LIBMYSQL_PATH)$/include
.ELSE
  .IF "$(SYSTEM_MYSQL)" != "YES"
    MYSQL_LIBDIR=$(LIBMYSQL_PATH)$/lib
    MYSQL_INCDIR=$(LIBMYSQL_PATH)$/include
  .ENDIF
.ENDIF

.IF "$(SYSTEM_MYSQL)" == "YES"
CFLAGS+=-DSYSTEM_MYSQL
.ENDIF

.IF "$(SYSTEM_MYSQL_CPPCONN)" == "YES"
CFLAGS+=-DSYSTEM_MYSQL_CPPCONN
.ENDIF

CDEFS+=-DCPPDBC_EXPORTS -DCPPCONN_LIB_BUILD
.IF "$(SYSTEM_MYSQL_CPPCONN)" != "YES"
CDEFS += -DCPPCONN_LIB=\"$(DLLPRE)mysqlcppconn$(DLLPOST)\"
.ENDIF

# --------------- MySQL settings ------------------
.IF "$(GUI)"=="WNT"
  MYSQL_INC=-I$(SOLARINCDIR)$/mysqlcppconn -I$(SOLARINCDIR)$/mysqlcppconn/cppconn -I$(MYSQL_INCDIR)
  MYSQL_LIB=$(MYSQL_LIBDIR)$/libmysql.lib
  MYSQL_LIBFILE=$(MYSQL_LIBDIR)$/libmysql.dll
  MYSQL_CPPCONNFILE=$(SOLARBINDIR)$/$(DLLPRE)mysqlcppconn$(DLLPOST)
.ELSE
  .IF "$(SYSTEM_MYSQL)" != "YES"
    MYSQL_INC+=-I$(MYSQL_INCDIR)
    .IF "$(OS)"=="MACOSX"
      MYSQL_LIBFILE=$(MYSQL_LIBDIR)$/$(DLLPRE)mysql.16$(DLLPOST)
    .ELSE
      MYSQL_LIBFILE=$(MYSQL_LIBDIR)$/$(DLLPRE)mysql$(DLLPOST).16
    .ENDIF

    # checkdll will complain if we do not point it to the libmysql lib
    EXTRALIBPATHS=-L$(MYSQL_LIBDIR)
  .ENDIF

  .IF "$(SYSTEM_MYSQL_CPPCONN)" != "YES"
    .IF "$(OS)"=="MACOSX" || "$(OS)" == "SOLARIS"
      MYSQL_LIB+=-lz -lm
    .ELSE
      MYSQL_LIB+=-rdynamic -lz -lcrypt -lnsl -lm
    .ENDIF
    MYSQL_CPPCONNFILE=$(SOLARLIBDIR)$/$(DLLPRE)mysqlcppconn$(DLLPOST)
  .ELSE
    MYSQL_CPPCONN_LIB+=-lmysqlcppconn
  .ENDIF
.ENDIF

.IF "$(SYSTEM_MYSQL_CPPCONN)" != "YES"
  MYSQL_INC+=-I$(SOLARINCDIR)$/mysqlcppconn -I$(SOLARINCDIR)$/mysqlcppconn/cppconn
.ENDIF

CFLAGS+=-I..$/..$/inc $(MYSQL_INC) \
    -DMYSQLC_VERSION_MAJOR=$(MYSQLC_VERSION_MAJOR) \
    -DMYSQLC_VERSION_MINOR=$(MYSQLC_VERSION_MINOR) \
    -DMYSQLC_VERSION_MICRO=$(MYSQLC_VERSION_MICRO) \
    $(MYSQL_DEFINES)

# provide the name of the MySQL client lib to the C++ code
.IF "$(SYSTEM_MYSQL)" != "YES"
CDEFS += -DMYSQL_LIB=\"$(MYSQL_LIBFILE:f)\"
.ENDIF

#--------------------------------------------------

SHL1DLLPRE=
SHL1TARGET=$(TARGET).uno
LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES= \
        $(SLO)$/mysqlc_driver.obj \
        $(SLO)$/mysqlc_services.obj  \
        $(SLO)$/mysqlc_connection.obj \
        $(SLO)$/mysqlc_resultset.obj \
        $(SLO)$/mysqlc_resultsetmetadata.obj \
        $(SLO)$/mysqlc_statement.obj \
        $(SLO)$/mysqlc_preparedstatement.obj \
        $(SLO)$/mysqlc_databasemetadata.obj \
        $(SLO)$/mysqlc_types.obj \
        $(SLO)$/mysqlc_general.obj \
        $(SLO)$/mysqlc_propertyids.obj

SHL1STDLIBS= \
        $(MYSQL_LIB)                \
        $(MYSQL_CPPCONN_LIB)        \
        $(CPPULIB)                  \
        $(SALLIB)                   \
        $(SALHELPERLIB)             \
        $(CPPUHELPERLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=OXT
DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# create Extension -----------------------------

# DESCRIPTION_SRC is the source file which is copied into the extension
# It is defaulted to "description.xml", but we want to pre-process it, so we use an intermediate
# file
DESCRIPTION_SRC = $(MISC)$/description.xml

COMPONENT_XCU=\
        registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu

COMPONENT_MERGED_XCU= \
        $(foreach,i,$(COMPONENT_XCU) $(EXTENSIONDIR)$/$i)

COMPONENT_LIBRARY = \
            $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)

COMPONENT_MYSQL_LIBFILE = \
            $(EXTENSIONDIR)$/$(MYSQL_LIBFILE:f) 

COMPONENT_MYSQL_CPPCONN_FILE=\
            $(EXTENSIONDIR)$/$(MYSQL_CPPCONNFILE:f)

COMPONENT_LIBRARIES=\
            $(COMPONENT_LIBRARY)

COMPONENT_IMAGES= \
    $(EXTENSIONDIR)$/images$/extension_32.png

# ........ component description ........
# one file for each WITH_LANG token
COMPONENT_DESCRIPTIONS=$/$(foreach,lang,$(alllangiso) description-$(lang).txt)
COMPONENT_DESCRIPTIONS_PACKDEP= \
    $(foreach,i,$(COMPONENT_DESCRIPTIONS) $(EXTENSIONDIR)$/$i)

# ........ dependencies for packaging the extension ........
EXTENSION_PACKDEPS=makefile.mk $(COMPONENT_IMAGES) $(COMPONENT_DESCRIPTIONS_PACKDEP) $(COMPONENT_MERGED_XCU)

.IF "$(SYSTEM_MYSQL)" != "YES"
EXTENSION_PACKDEPS+=$(COMPONENT_MYSQL_LIBFILE)
.ENDIF

.IF "$(SYSTEM_MYSQL_CPPCONN)" != "YES"
EXTENSION_PACKDEPS+=$(COMPONENT_MYSQL_CPPCONN_FILE)
.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

$(COMPONENT_IMAGES) : $(SRC_ROOT)/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

# xcu files: copy
# the following is a hack for now - need to find out the generic mechanism later

$(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu : .$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu
    @@-$(MKDIRHIER) $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess
    @$(COPY) $< $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu > $(NULLDEV)

$(COMPONENT_DESCRIPTIONS_PACKDEP) : $(DESCRIPTION)

$(DESCRIPTION_SRC): description.xml
    +-$(RM) $@
.IF "$(WITH_LANG)" != ""
    $(XRMEX) -p $(PRJNAME) -i $< -o $@ -m $(mktmp $(foreach,lang,$(subst,en-US, $(WITH_LANG_LIST)) $(SRCDIR)/translations/source/$(lang)/$(PRJNAME)$/$(PATH_IN_MODULE).po)) -l all
    $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" < $@ > $@.new
    mv $@.new $@
    @$(COPY) $(@:d)/description-*.txt $(EXTENSIONDIR)
.ELSE
    $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" < $< > $@
.ENDIF
    @$(COPY) description-en-US.txt $(EXTENSIONDIR)

.IF "$(SYSTEM_MYSQL)" != "YES" 
# --- the MySQL client lib needs to be copied
$(COMPONENT_MYSQL_LIBFILE): $(MYSQL_LIBFILE)
    @@-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@ > $(NULLDEV)
    .IF "$(OS)" == "MACOSX"
        $(XCRUN) install_name_tool -id @__________________________________________________OOO/$(MYSQL_LIBFILE:f) $@
    .ENDIF
.ENDIF

.IF "$(SYSTEM_MYSQL_CPPCONN)" != "YES"
# --- the MySQL cppconn lib needs to be copied
$(COMPONENT_MYSQL_CPPCONN_FILE): $(MYSQL_CPPCONNFILE)
    @@-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@ > $(NULLDEV)
    .IF "$(OS)" == "MACOSX"
        $(XCRUN) install_name_tool -change $(MYSQL_LIBFILE:f) @loader_path/$(MYSQL_LIBFILE:f) $@
    .ENDIF
.ENDIF

.IF "$(OS)" == "MACOSX"
$(EXTENSION_TARGET): adjust_libmysql_path

adjust_libmysql_path: $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)
    $(XCRUN) install_name_tool -change $(MYSQL_LIBFILE:f) @loader_path/$(MYSQL_LIBFILE:f) $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)
.ENDIF
