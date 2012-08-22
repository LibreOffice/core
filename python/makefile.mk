#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

PRJ=.

PRJNAME=so_python
TARGET=so_python

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(DISABLE_PYTHON)"!="TRUE"

.INCLUDE :      pyversion.mk

.IF "$(SYSTEM_PYTHON)" == "YES"
all:
	@echo "An already available installation of python should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=Python-$(PYVERSION)
TARFILE_MD5=e81c2f0953aa60f8062c05a4673f2be0
PATCH_FILES=\
    Python-$(PYVERSION).patch \
    Python-parallel-make.patch \
    Python-ssl.patch \
    Python-aix.patch \
    Python-2.6.1-urllib.patch \
    Python-2.6.1-sysbase.patch \
    Python-2.6.1-nohardlink.patch \
    Python-2.6.1-py2422.patch \
    Python-2.6.1-py4768.patch \
    Python-2.6.1-svn-1.7.patch \
    Python.mipsel-py4305.patch

CONFIGURE_DIR=

.IF "$(GUI)"=="UNX"
BUILD_DIR=
MYCWD=$(shell @pwd)/$(INPATH)/misc/build

.IF "$(SYSTEM_OPENSSL)" != "YES"
#we want to be sure we use our internal openssl if
#that's enabled
CC+:=-I$(SOLARINCDIR)$/external
python_LDFLAGS+=-L$(SOLARLIBDIR)
.ENDIF

# CLFLAGS get overwritten in Makefile.pre.in
.IF "$(SYSBASE)"!=""
CC+:=-I$(SYSBASE)$/usr$/include
python_LDFLAGS+=-L$(SYSBASE)/usr/lib
.IF "$(COMNAME)"=="sunpro5"
CC+:=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(CPU)"=="SOLARISU"
CC+:=$(ARCH_FLAGS)
python_LDFLAGS+=$(ARCH_FLAGS)
.ENDIF

.IF "$(OS)"=="AIX"
python_CFLAGS=-g0
.ENDIF

.IF "$(OS)" == "MACOSX"
my_prefix = @__________________________________________________$(EXTRPATH)
.ELSE
my_prefix = python-inst
.END

CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) ./configure --prefix=/$(my_prefix) --enable-shared CFLAGS="$(python_CFLAGS)" LDFLAGS="$(python_LDFLAGS)"

.IF "$(OS)$(CPUNAME)" == "SOLARISINTEL"
CONFIGURE_ACTION += --disable-ipv6
.ENDIF

.IF "$(OS)" == "MACOSX"
PATCH_FILES+=Python-2.6.1-py8067.patch
# don't build dual-arch version as OOo itself is not universal binary either
PATCH_FILES+=Python-2.6.1-arch_$(eq,$(CPU),I i386 ppc).patch

CONFIGURE_ACTION+=--enable-universalsdk=$(MACOSX_SDK_PATH) --with-universal-archs=32-bit --enable-framework=/$(my_prefix) --with-framework-name=OOoPython
ALLTAR: $(MISC)/OOoPython.framework.zip

.ENDIF

.IF "$(OS)"=="AIX"
CONFIGURE_ACTION += --disable-ipv6 --with-threads
.ENDIF
BUILD_ACTION=$(ENV_BUILD) $(GNUMAKE) -j$(EXTMAXPROCESS) && $(GNUMAKE) install DESTDIR=$(MYCWD) && chmod -R ug+w $(MYCWD)/$(my_prefix) && chmod g+w Include
.ELSE
# ----------------------------------
# WINDOWS
# ----------------------------------
.IF "$(COM)"=="GCC"
PATCH_FILES= \
	     Python-2.6.1-svn-1.7.patch \
	     Python-2.6.2-bdb48.patch \
	     Python-2.6.2-cross.patch \
	     Python-2.6.2-cross.fix-configure.patch \

.IF "$(SYSTEM_DB)" != "YES"
PATCH_FILES += \
	       Python-2.6.1-cross.berkeleydb.patch \

.ENDIF

BUILD_DIR=
MYCWD=$(shell @pwd)/$(INPATH)/misc/build
.IF "$(PYTHON_FOR_BUILD)" == ""
PYTHON_FOR_BUILD = $(OUTDIR_FOR_BUILD)/bin/python$(PYMAJOR).$(PYMINOR)
.ENDIF

.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
python_LDFLAGS=-shared-libgcc
.ENDIF
python_LDFLAGS=-shared-libgcc -Wl,--enable-runtime-pseudo-reloc-v2 -Wl,--export-all-symbols
CONFIGURE_ACTION=\
		 ./configure \
			--build="$(BUILD_PLATFORM)" \
			--host="$(HOST_PLATFORM)" \
			--prefix=$(MYCWD)/$(my_prefix) \
			--enable-shared \
			--with-threads \
			LDFLAGS="$(python_LDFLAGS)" \
			CC_FOR_BUILD="$(CC_FOR_BUILD)" \
			PYTHON_FOR_BUILD="$(PYTHON_FOR_BUILD)" \
			ac_cv_printf_zd_format=no \

BUILD_ACTION=$(ENV_BUILD) make && make install
.ELSE

.IF "$(CCNUMVER)" >= "001600000000"
PATCH_FILES+=Python-$(PYVERSION)-vc10.patch
PATCH_FILES+=Python-$(PYVERSION)-vc12.patch
BUILD_DIR=PC/VS10.0
.ELIF "$(CCNUMVER)" >= "001500000000"
BUILD_DIR=PCbuild
.ELIF "$(CCNUMVER)" >= "001400000000"
BUILD_DIR=PC/VS8.0
.ELIF "$(CCNUMVER)" >= "001310000000"
BUILD_DIR=PC/VS7.1
.ELSE
BUILD_DIR=PC/VC6
.ENDIF

.IF "$(CPUNAME)" == "INTEL"
ARCH=Win32
.ELSE
ARCH=x64
.ENDIF

.IF "$(debug)"!=""
CONF=Debug
.ELSE
CONF=Release
.ENDIF

# Build python executable and then runs a minimal script. Running the minimal script
# ensures that certain *.pyc files are generated which would otherwise be created on
# solver during registration in insetoo_native
.IF "$(CCNUMVER)" >= "001700000000"
BUILD_ACTION=MSBuild.exe pcbuild.sln /t:Build /p:Configuration=$(CONF) /p:PlatformToolset=v110
.ELIF "$(CCNUMVER)" >= "001600000000"
BUILD_ACTION=MSBuild.exe pcbuild.sln /t:Build /p:Configuration=$(CONF) /ToolsVersion:4.0
.ELSE
BUILD_ACTION=$(COMPATH)$/vcpackages$/vcbuild.exe pcbuild.sln "$(CONF)|$(ARCH)"
.ENDIF
.ENDIF
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk
PYVERSIONGFILE=$(MISC)$/pyversion.Makefile

# --- Targets ------------------------------------------------------


.INCLUDE : set_ext.mk
.ENDIF # DISABLE_PYTHON != TRUE
.INCLUDE : target.mk
.IF "$(DISABLE_PYTHON)"!="TRUE"
.INCLUDE : tg_ext.mk

.IF "$(L10N_framework)"==""
.IF "$(GUI)" != "UNX"
.IF "$(COM)"!="GCC"
PYCONFIG:=$(MISC)$/build$/pyconfig.h
$(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)

$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PYCONFIG)

$(PYCONFIG) : $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@
# We know that the only thing guarded with #ifdef _DEBUG in PC/pyconfig.h is
# the line defining Py_DEBUG.
.IF "$(debug)"!=""
# If Python is built with debugging, then the modules we build need to be built with
# Py_DEBUG defined too because of the Py_InitModule4 redefining magic in modsupport.h
    sed -e 's/^#ifdef _DEBUG$/#if 1/' <$@ >$@.new && mv $@.new $@
.ELSE
# Correspondingly, if Python is not built with debugging, it won't use the Py_InitModule4 redefining
# magic, so our Python modules should not be built to provide that either.
    sed -e 's/^#ifdef _DEBUG$/#if 0/' <$@ >$@.new && mv $@.new $@
.ENDIF
.ENDIF
.ENDIF

ALLTAR : $(PYVERSIONFILE) $(PYVERSIONGFILE)
.ENDIF          # "$(L10N_framework)"==""

# rule to allow relocating the whole framework, removing reference to buildinstallation directory
$(PACKAGE_DIR)/fixscripts: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
	@echo remove build installdir from scripts
	$(COMMAND_ECHO)for file in \
	        $(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/bin/2to3 \
	        $(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/bin/idle$(PYMAJOR).$(PYMINOR) \
	        $(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/bin/pydoc$(PYMAJOR).$(PYMINOR) \
	        $(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/bin/python$(PYMAJOR).$(PYMINOR)-config \
	        $(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/bin/smtpd$(PYMAJOR).$(PYMINOR).py ; do \
	{{ rm "$$file" && awk '\
		BEGIN {{print "\
#!/bin/bash\n\
origpath=$$(pwd)\n\
bindir=$$(cd $$(dirname \"$$0\") ; pwd)\n\
cd \"$$origpath\"\n\
\"$$bindir/../Resources/Python.app/Contents/MacOS/OOoPython\" - $$@ <<EOF"}} \
		FNR==1{{next}} \
		      {{print}} \
		END   {{print "EOF"}}' > "$$file" ; }} < "$$file" ; chmod +x "$$file" ; done
	@touch $@

$(PACKAGE_DIR)/fixinstallnames: $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
	@echo remove build installdir from OOoPython
	$(COMMAND_ECHO)install_name_tool -change \
		/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/OOoPython \
		@executable_path/../../../../OOoPython \
		$(MYCWD)/$(my_prefix)/OOoPython.framework/Versions/$(PYMAJOR).$(PYMINOR)/Resources/Python.app/Contents/MacOS/OOoPython
	@touch $@

$(MISC)/OOoPython.framework.zip: $(PACKAGE_DIR)/fixinstallnames $(PACKAGE_DIR)/fixscripts
	@-rm -f $@
	@echo creating $@
	$(COMMAND_ECHO)cd $(MISC)/build/$(my_prefix) && find OOoPython.framework \
		-not -type l -not -name Info.plist.in \
		-not -name pythonw$(PYMAJOR).$(PYMINOR) \
		-not -name python$(PYMAJOR).$(PYMINOR) -print0 | \
		xargs -0 zip $(ZIP_VERBOSITY) ../../$(@:f)

$(PYVERSIONFILE) : pyversion.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
	@-rm -f $@
	$(COMMAND_ECHO)$(COPY) pyversion.mk $@

$(PYVERSIONGFILE) : pyversion.Makefile $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
	@-rm -f $@
	$(COMMAND_ECHO)$(COPY) pyversion.Makefile $@

.ENDIF # DISABLE_PYTHON != TRUE
