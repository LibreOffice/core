#*************************************************************************
#
#   $RCSfile: tg_sdi.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 14:47:38 $
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************



# ------------------
# - SDITARGETS     -
# ------------------

.IF "$(MULTI_SDI_FLAG)" == ""

$(SDITARGET) .NULL	: SDI0

$(SDI1TARGET) .NULL : SDI1

$(SDI2TARGET) .NULL : SDI2

$(SDI3TARGET) .NULL : SDI3

$(SDI4TARGET) .NULL : SDI4

$(SDI5TARGET) .NULL : SDI5

.ENDIF


.IF "$(MULTI_SDI_FLAG)" == ""
SDI0 SDI1 SDI2 SDI3 SDI4 SDI5 :
    @+dmake $(SDI$(TNR)TARGET) $(HIDSID$(TNR)PARTICLE) MULTI_SDI_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE			# "$(MULTI_SDI_FLAG)" == ""

.IF "$(make_srs_deps)"==""
dttt:
    echo $(TNR)------------

$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND) $(SDI$(TNR)NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @+-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)$/$(SDI$(TNR)NAME).hxx	\
    -fd$(INCCOMX)$/$(SDI$(TNR)NAME).ilb	\
    -fm$(MISCX)$/$(SDI$(TNR)NAME).don	\
    -fl$(MISC)$/$(SDI$(TNR)NAME).lst         \
    -fx$(SDI$(TNR)EXPORT).sdi		\
    -fy$(MISCX)$/xx$(PRJNAME).csv		\
    -fz$(MISCX)$/$(SDI$(TNR)NAME).sid	\
    $(SDI$(TNR)NAME).sdi -I$(MISCX) -I$(SVSDIINC) -I$(INC) -I$(INCLUDE) -I$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc )

$(HIDSID$(TNR)PARTICLE): $(MISCX)$/$(SDI$(TNR)NAME).sid
    @echo ------------------------------
    @echo Making: $@
.IF "$(USE_SHELL)"=="4nt"
    @$(TYPE) $(MISCX)$/$(SDI$(TNR)NAME).sid | $(AWK) "$$1==\"#define\" { print $$2, $$3 }" > $@.$(ROUT).tmp
.ELSE
    @$(TYPE) $(MISCX)$/$(SDI$(TNR)NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
.ENDIF
    @+$(RENAME) $@.$(ROUT).tmp $@


.ELSE			# "$(make_srs_deps)"==""
$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND)
    @+echo jetzt nicht...
$(HIDSID$(TNR)PARTICLE):
    @+echo jetzt nicht...
.ENDIF			# "$(make_srs_deps)"==""
.ENDIF			# "$(MULTI_SDI_FLAG)" == ""

