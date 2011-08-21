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

#######################################################
# instructions for ???
# unroll begin

.IF "$(SDI$(TNR)TARGET)"!=""
$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND) $(SDI$(TNR)NAME).sdi
    @echo "Making:   " $(@:f)
    @-$(RM) $@
    $(COMMAND_ECHO)$(SVIDL) @$(mktmp \
    $(VERBOSITY) \
    -fs$(INCCOMX)/$(SDI$(TNR)NAME).hxx	\
    -fd$(INCCOMX)/$(SDI$(TNR)NAME).ilb	\
    -fm$(MISC)/$(SDI$(TNR)NAME).don	\
    -fl$(MISC)/$(SDI$(TNR)NAME).lst         \
    -fx$(SDI$(TNR)EXPORT).sdi		\
    -fz$(MISC)/$(SDI$(TNR)NAME).sid	\
    $(SDI$(TNR)NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI$(TNR)TARGET)"!=""

# Instruction for ???
# unroll end
#######################################################

