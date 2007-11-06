#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:43:06 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
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
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=pyuno
TARGET=test
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
# --- Files --------------------------------------------------------

PYEXC=$(DLLDEST)$/python$(EXECPOST)
REGEXC=$(DLLDEST)$/regcomp$(EXECPOST)

DOLLAR_SIGN=\$$
.IF "$(USE_SHELL)" != "tcsh"
DOLLAR_SIGN=$$
.ENDIF

#these are temporary
REGCOMP=$(WRAPCMD) regcomp
PYTHON=$(WRAPCMD) python

.IF "$(GUI)"!="WNT" && "$(GUI)"!="OS2"
TEST_ENV=\
    setenv FOO file://$(shell @pwd)$/$(DLLDEST) && \
        setenv UNO_TYPES pyuno_regcomp.rdb && setenv UNO_SERVICES pyuno_regcomp.rdb
.ELSE # "$(GUI)" != "WNT"
# aaaaaa, how to get the current working directory on windows ???
.IF "$(USE_SHELL)" == "tcsh"

# these are temporary (should go into the global env script)
##PYTHON=guw.pl -env $(SOLARVER)/$(UPD)/$(INPATH)/bin/python.exe
##REGCOMP=guw.pl -env `which regcomp.exe`

CWD_TMP=$(strip $(shell @$(WRAPCMD) echo `pwd`))
TEST_ENV=setenv FOO file:///$(strip $(subst,\,/ $(CWD_TMP)/$(DLLDEST))) && \
        setenv UNO_TYPES pyuno_regcomp.rdb && setenv UNO_SERVICES pyuno_regcomp.rdb
.ELSE
CWD_TMP=$(strip $(shell @echo import os;print os.getcwd() | $(PYTHON)))
TEST_ENV=set FOO=file:///$(strip $(subst,\,/ $(CWD_TMP)$/$(DLLDEST))) && \
        set UNO_TYPES=pyuno_regcomp.rdb && set UNO_SERVICES=pyuno_regcomp.rdb
.ENDIF "$(USE_SHELL)" == "tcsh"

.ENDIF  # "$(GUI)"!="WNT"
PYFILES = \
    $(DLLDEST)$/core.py			\
    $(DLLDEST)$/importer.py			\
    $(DLLDEST)$/main.py			\
    $(DLLDEST)$/impl.py			\
    $(DLLDEST)$/samplecomponent.py		\
    $(DLLDEST)$/testcomp.py			\

PYCOMPONENTS = \
    samplecomponent

ALL : 	\
    $(PYFILES)				\
    $(DLLDEST)$/pyuno_regcomp.rdb		\
    doc					\
    ALLTAR

.INCLUDE :  target.mk

$(DLLDEST)$/%.py: %.py
    cp $? $@

$(DLLDEST)$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    cp $? $@

$(DLLDEST)$/regcomp$(EXECPOST) : $(SOLARBINDIR)$/regcomp$(EXECPOST)
    cp $? $@

$(DLLDEST)$/pyuno_regcomp.rdb: $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb
    -rm -f $@
    $(WRAPCMD) regmerge $(DLLDEST)$/pyuno_regcomp.rdb / $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb

doc .PHONY:
    @echo start test with  dmake runtest

runtest : ALL
    cd $(DLLDEST) && $(TEST_ENV) && python main.py 
    cd $(DLLDEST) && $(TEST_ENV) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy.rdb \
            -l com.sun.star.loader.Python $(foreach,i,$(PYCOMPONENTS) -c vnd.openoffice.pymodule:$(i))
    cd $(DLLDEST) && $(TEST_ENV) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy2.rdb \
            -l com.sun.star.loader.Python -c vnd.sun.star.expand:$(DOLLAR_SIGN)FOO/samplecomponent.py

