#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 17:34:17 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
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
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
#
#*************************************************************************

PRJ=.

PRJNAME=so_python
TARGET=so_python

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :      pyversion.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=Python-$(PYVERSION)
PATCH_FILE_NAME=Python-$(PYVERSION).patch

CONFIGURE_DIR=

BUILD_DIR=

.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
.IF "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
CC:=$(COMPATH)$/bin$/cc
CXX:=$(COMPATH)$/bin$/CC
.ENDIF          # "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
.ENDIF          # "$(COMNAME)"=="sunpro5"
.ELSE
.ENDIF          # "$(GUI)"=="UNX"


.IF "$(USE_SHELL)"!="4nt"
CONVERT=convert.sh
BUILD_ACTION_SEP=;
.ELSE # "$(USE_SHELL)"!="4nt"
CONVERT=convert.bat
BUILD_ACTION_SEP=^
.ENDIF # "$(USE_SHELL)"!="4nt"

DSP_DIR=PCbuild
.IF "$(GUI)" == "UNX"
CONFIGURE_ACTION= ./configure --prefix=../python-inst
.IF "$(OS)" == "IRIX"
BUILD_ACTION=gmake ; gmake install
.ELSE
BUILD_ACTION=make ; make install
.ENDIF
.IF "$(OS)"=="MACOSX"
PYTHONCORESHL=$(OUT)$/lib$/$(DLLPRE)python$(PYMAJOR)$(PYMINOR)$(DLLPOST)
PYTHONCORELINK1=$(OUT)$/lib$/$(DLLPRE)python$(PYMAJOR)$(DLLPOST)
PYTHONCORELINK2=$(OUT)$/lib$/$(DLLPRE)python$(DLLPOST)
.ELSE
PYTHONCORESHL=$(OUT)$/lib$/$(DLLPRE)python$(DLLPOST).$(PYVERSION)
PYTHONCORELINK1=$(OUT)$/lib$/$(DLLPRE)python$(DLLPOST).$(PYMAJOR)
PYTHONCORELINK2=$(OUT)$/lib$/$(DLLPRE)python$(DLLPOST)
.ENDIF
.ELSE
BUILD_DIR=$(DSP_DIR)
CONFIGURE_DIR=$(DSP_DIR)

.IF "$(COMEX)"=="8"
CONFIGURE_ACTION=wdevenv pcbuild Release
BUILD_ACTION=devenv /build Release /project winsound pcbuild.sln /useenv \
    $(BUILD_ACTION_SEP) devenv /build Release /project winreg pcbuild.sln /useenv \
    $(BUILD_ACTION_SEP) devenv /build Release /project unicodedata pcbuild.sln /useenv 	\
    $(BUILD_ACTION_SEP) devenv /build Release /project select pcbuild.sln /useenv  \
    $(BUILD_ACTION_SEP) devenv /build Release /project parser pcbuild.sln /useenv  \
    $(BUILD_ACTION_SEP) devenv /build Release /project mmap pcbuild.sln /useenv    \
    $(BUILD_ACTION_SEP) devenv /build Release /project _symtable pcbuild.sln /useenv \
    $(BUILD_ACTION_SEP) devenv /build Release /project _socket pcbuild.sln /useenv  \
    $(BUILD_ACTION_SEP) devenv /build Release /project _sre pcbuild.sln /useenv  \
    $(BUILD_ACTION_SEP) devenv /build Release /project python pcbuild.sln /useenv
.ELSE
BUILD_ACTION=msdev pcbuild.dsw /USEENV /MAKE 	\
    "python - Win32 Release" 	\
    "_sre - Win32 Release" 		\
    "_socket - Win32 Release"	\
    "_symtable - Win32 Release" 	\
    "mmap - Win32 Release" 		\
    "parser - Win32 Release" 	\
    "select - Win32 Release" 	\
    "unicodedata - Win32 Release" 	\
    "winreg - Win32 Release" 	\
    "winsound - Win32 Release"
.ENDIF
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk

# --- Targets ------------------------------------------------------

.IF "$(GUI)" != "UNX"
PYCONFIG=$(MISC)$/build$/pyconfig.h
.ENDIF

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

ALLTAR : $(PYCONFIG) $(PYTHONCORESHL) $(PYVERSIONFILE) $(PYTHONCORELINK1) $(PYTHONCORELINK2)

$(MISC)$/convert_unx_flag :  $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    +$(CONVERT) unx $(PACKAGE_DIR)$/$(TARFILE_NAME)$/$(DSP_DIR)
    +$(TOUCH) $(MISC)$/convert_unx_flag

$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(MISC)$/convert_unx_flag

$(MISC)$/convert_dos_flag : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    +$(CONVERT) dos $(PACKAGE_DIR)$/$(TARFILE_NAME)$/$(DSP_DIR)
    +$(TOUCH) $(MISC)$/convert_dos_flag

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(MISC)$/convert_dos_flag

$(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)

#.IF "$(GUI)" !="UNX"
$(MISC)$/build$/pyconfig.h : $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@
#.ENDIF

.IF "$(GUI)" == "UNX"
$(PYTHONCORESHL) : makefile.mk $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.IF "$(OS)" == "SOLARIS"
    ld -G -o $@ -u Py_Main -u Py_FrozenMain -u PyFPE_dummy $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a -h libpython.so.$(PYMAJOR) -lm -ldl -lc -lpthread
.ELSE
.IF "$(OS)" == "FREEBSD"
    ld -shared -o $@ --whole-archive $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a --no-whole-archive -soname libpython.so.$(PYMAJOR)  -lm -lutil ${PTHREAD_LIBS}
.ELSE
.IF "$(OS)" == "IRIX"
    ld -shared -o $@ -all $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a -notall -soname libpython.so.$(PYMAJOR)  -lm -ldl -lc -lpthread
.ELSE	
.IF "$(OS)" == "MACOSX"
    cp $(MISC)$/build$/$(TARFILE_NAME)$/$(DLLPRE)python$(PYMAJOR).$(PYMINOR)$(DLLPOST) $(OUT)$/lib
.ELSE
    echo "$(OS)"
    ld -shared -o $@ --whole-archive $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a --no-whole-archive -soname libpython.so.$(PYMAJOR)  -lm -ldl -lutil -lc -lpthread
.ENDIF # MACOSX
.ENDIF # IRIX
.ENDIF # FREEBSD
.ENDIF # SOLARIS

$(PYTHONCORELINK1) : makefile.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    -rm -f $@
.IF "$(OS)" == "MACOSX"
    cd $(OUT)$/lib && ln -s $(DLLPRE)python$(PYMAJOR).$(PYMINOR)$(DLLPOST) $(DLLPRE)python$(PYMAJOR)$(DLLPOST)
.ELSE
    cd $(OUT)$/lib && ln -s $(DLLPRE)python$(DLLPOST).$(PYVERSION) $(DLLPRE)python$(DLLPOST).$(PYMAJOR)
.ENDIF

$(PYTHONCORELINK2) : makefile.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    -rm -f $@
.IF "$(OS)" == "MACOSX"
    cd $(OUT)$/lib && ln -s $(DLLPRE)python$(PYMAJOR).$(PYMINOR)$(DLLPOST) $(DLLPRE)python$(DLLPOST)
.ELSE
    cd $(OUT)$/lib && ln -s $(DLLPRE)python$(DLLPOST).$(PYVERSION) $(DLLPRE)python$(DLLPOST)
.ENDIF
.ENDIF

$(PYVERSIONFILE) : pyversion.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    -rm -f $@
    cat $? > $@

