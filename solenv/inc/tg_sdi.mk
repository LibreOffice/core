#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_sdi.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: vg $ $Date: 2006-11-21 15:10:57 $
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

#######################################################
# instructions for ???
# unroll begin

.IF "$(HIDSID$(TNR)PARTICLE)"!=""
$(HIDSID$(TNR)PARTICLE): $(SDI$(TNR)TARGET)
    @echo ------------------------------
    @echo Making: $@
    @+-$(RM) $@.$(ROUT).tmp $@
.IF "$(USE_SHELL)"=="4nt"
    @$(TYPE) $(MISC)$/$(SDI$(TNR)NAME).sid | $(AWK) "$$1==\"#define\" { print $$2, $$3 }" > $@.$(ROUT).tmp
.ELSE
    @$(TYPE) $(MISC)$/$(SDI$(TNR)NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
.ENDIF
    @+-$(RM) $@
    @+$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID$(TNR)PARTICLE)"!=""

.IF "$(SDI$(TNR)TARGET)"!=""
$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND) $(SDI$(TNR)NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @+-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)$/$(SDI$(TNR)NAME).hxx	\
    -fd$(INCCOMX)$/$(SDI$(TNR)NAME).ilb	\
    -fm$(MISC)$/$(SDI$(TNR)NAME).don	\
    -fl$(MISC)$/$(SDI$(TNR)NAME).lst         \
    -fx$(SDI$(TNR)EXPORT).sdi		\
    -fy$(MISC)$/xx$(PRJNAME).csv		\
    -fz$(MISC)$/$(SDI$(TNR)NAME).sid	\
    $(SDI$(TNR)NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI$(TNR)TARGET)"!=""

# Instruction for ???
# unroll end
#######################################################

