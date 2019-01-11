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



PRJ=..
PRJNAME=mysqlc

.INCLUDE : $(PRJ)$/version.mk

TARGET=mysqlc
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
EXTENSIONNAME:=mysql-connector-ooo

.IF "$(SYSTEM_MYSQL)" == "YES"
EXTERNAL_WARNINGS_NOT_ERRORS = TRUE
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

#---- extension version
EXTENSION_VERSION_BASE=$(MYSQLC_VERSION_MAJOR).$(MYSQLC_VERSION_MINOR).$(MYSQLC_VERSION_MICRO)
.IF ( "$(CWS_WORK_STAMP)" == "" ) || ( "$(UPDATER)" == "YES" )
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)
.ELSE
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE).cws.$(CWS_WORK_STAMP)
.ENDIF

#---- extension title package name
.IF "$(MYSQLC_STATUS)" == "final"
    EXTENSION_TITLE:=$(MYSQLC_TITLE)
    EXTENSION_ZIPNAME:=$(EXTENSIONNAME)
.ELSE
    EXTENSION_TITLE:=$(MYSQLC_TITLE) ($(MYSQLC_STATUS))
    EXTENSION_ZIPNAME:=$(EXTENSIONNAME)
.ENDIF

#-------------------------------------------------------------------

# set default symbol visibility / scope to hidden
.IF "$(COMNAME)" == "gcc3"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
 CFLAGS += -fvisibility=hidden
.ENDIF # HAVE_GCC_VISIBILITY_FEATURE
.ENDIF # gcc3

.IF "$(COMNAME)" == "sunpro5"
.IF "$(CCNUMVER)" >= "00050005"
 CFLAGS += -xldscope=hidden
.ENDIF # 5.5
.ENDIF # sunpro5

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
.IF "$(SYSTEM_MYSQL_CPPCONN)" == "YES"
CDEFS += -DCPPCONN_LIB=\"$(shell readlink /usr/lib/libmysqlcppconn.so)\"
.IF "$(USE_SYSTEM_STL)"!="YES"
CDEFS += -DADAPT_EXT_STL
.ENDIF
.ENDIF

# --------------- MySQL settings ------------------
.IF "$(GUI)"!="WNT"
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

    MYSQL_CPPCONN_LIB+=-lmysqlcppconn
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

SHL1VERSIONMAP=$(TARGET).map
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=OXT
DEF1NAME=	$(SHL1TARGET)

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
    $(EXTENSIONDIR)$/images$/extension_32.png \
    $(EXTENSIONDIR)$/images$/extension_32_h.png


# ........ component description ........
# one file for each WITH_LANG token
DESC_LANGS=$(WITH_LANG)
.IF "$(DESC_LANGS)" == ""
    DESC_LANGS=en-US
.ENDIF
COMPONENT_DESCRIPTIONS=$/$(foreach,lang,$(DESC_LANGS) description$/description_$(lang).txt)
COMPONENT_DESCRIPTIONS_PACKDEP= \
    $(foreach,i,$(COMPONENT_DESCRIPTIONS) $(EXTENSIONDIR)$/$i)

# WITH_LANG might contain languages which we actually do not have a description for (yet)
# Find those, and treat the specially

# first, find those locales which we actually have a description file for
EXISTING_DESCRIPTIONS=$(foreach,i,$(shell $(FIND) .$/description -name "description_*.txt") $(i:f))
EXISTING_LANGS=$(foreach,i,$(EXISTING_DESCRIPTIONS) $(i:s,description_,,:s,.txt,,))

# then, create a version of WITH_LANG where we stripped all those locales
EXISTING_LANGS_NORMALIZED=$(strip $(EXISTING_LANGS))
EXISTING_LANGS_NORMALIZED:=+$(EXISTING_LANGS_NORMALIZED:s/ /+/)+
MISSING_LANGS=$(foreach,lang,$(WITH_LANG) $(eq,$(EXISTING_LANGS_NORMALIZED:s/+$(lang)+//),$(EXISTING_LANGS_NORMALIZED) $(lang)  ))

# ........ dependencies for packaging the extension ........
EXTENSION_PACKDEPS=makefile.mk $(COMPONENT_IMAGES) $(COMPONENT_DESCRIPTIONS_PACKDEP) $(COMPONENT_MERGED_XCU)

.IF "$(SYSTEM_MYSQL)" != "YES"
EXTENSION_PACKDEPS+=$(COMPONENT_MYSQL_LIBFILE)
.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

$(COMPONENT_IMAGES) : $(SOLARSRC)$/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

# existing descriptions: just copy
$(EXTENSIONDIR)$/description$/%.txt: .$/description$/%.txt
    @@-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@ > $(NULLDEV)

# xcu files: copy
# the following is a hack for now - need to find out the generic mechanism later

$(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu : .$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu
    @@-$(MKDIRHIER) $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess
    @$(COPY) $< $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess$/Drivers.xcu > $(NULLDEV)

# The below doesn't work - it's completely beyond me .... $@ is some strange path when echoed, so the whole construct
# doesn't work .... :(

#$(foreach,i,$(COMPONENT_XCU) $(EXTENSIONDIR)$/$i : $i)
#    @echo -----------------
#    @echo $@
#    @echo $<
#    @echo -----------------
#    -$(MKDIRHIER) $(@:d)
#    (COPY) $< $@
#    @echo -----------------

# non-existing descriptions: copy from the English version
.IF "$(strip $(MISSING_LANGS))" != ""
$(foreach,i,$(MISSING_LANGS) $(EXTENSIONDIR)$/description$/description_$i.txt): .$/description$/description_en-US.txt
    @echo ------ WARNING: .$/description$/$(@:f) not found, falling back to English description
    @@-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@ > $(NULLDEV)
.ENDIF

$(DESCRIPTION_SRC): description.xml
    +-$(RM) $@
    $(TYPE) description.xml | $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" | $(SED) "s,#TITLE#,$(EXTENSION_TITLE)," > $@

.IF "$(SYSTEM_MYSQL)" != "YES" 
# --- the MySQL client lib needs to be copied
$(COMPONENT_MYSQL_LIBFILE): $(MYSQL_LIBFILE)
    @@-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@ > $(NULLDEV)
    .IF "$(OS)" == "MACOSX"
        install_name_tool -id @_______OOO/$(MYSQL_LIBFILE:f) $@
    .ENDIF
.ENDIF

.IF "$(OS)" == "MACOSX"
$(EXTENSION_TARGET): adjust_libmysql_path

adjust_libmysql_path: $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)
    install_name_tool -change $(MYSQL_LIBFILE:f) @loader_path/$(MYSQL_LIBFILE:f) $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)
.ENDIF
