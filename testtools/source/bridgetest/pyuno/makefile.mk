#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2004-11-15 13:06:36 $
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
#
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
.INCLUDE :  sv.mk
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

.IF "$(GUI)"!="WNT"
REGCOMP_ENV=\
    setenv FOO file://$(shell pwd)$/$(DLLDEST)
.ELSE # "$(GUI)" != "WNT"
# aaaaaa, how to get the current working directory on windows ???
.IF "$(USE_SHELL)" == "tcsh"

# these are temporary (should go into the global env script)
##PYTHON=guw.pl -env $(SOLARVER)/$(UPD)/$(INPATH)/bin/python.exe
##REGCOMP=guw.pl -env `which regcomp.exe`

CWD_TMP=$(strip $(shell $(WRAPCMD) echo `pwd`))
REGCOMP_ENV=setenv FOO file:///$(strip $(subst,\,/ $(CWD_TMP)/$(DLLDEST)))
.ELSE
CWD_TMP=$(strip $(shell echo import os;print os.getcwd() | $(PYTHON)))
REGCOMP_ENV=set FOO=file:///$(strip $(subst,\,/ $(CWD_TMP)$/$(DLLDEST)))
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
    +cp $? $@

$(DLLDEST)$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    +cp $? $@

$(DLLDEST)$/regcomp$(EXECPOST) : $(SOLARBINDIR)$/regcomp$(EXECPOST)
    +cp $? $@

$(DLLDEST)$/pyuno_regcomp.rdb: $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb
    -rm -f $@
    $(WRAPCMD) regmerge $(DLLDEST)$/pyuno_regcomp.rdb / $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb

doc .PHONY:
    @echo start test with  dmake runtest

runtest : ALL
    +cd $(DLLDEST) && python main.py -env:UNO_TYPES=pyuno_regcomp.rdb -env:UNO_SERVICES=pyuno_regcomp.rdb
    +cd $(DLLDEST) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy.rdb \
            -l com.sun.star.loader.Python $(foreach,i,$(PYCOMPONENTS) -c vnd.openoffice.pymodule:$(i))
    +cd $(DLLDEST) && $(REGCOMP_ENV) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy2.rdb \
            -l com.sun.star.loader.Python -c vnd.sun.star.expand:$(DOLLAR_SIGN)FOO/samplecomponent.py

