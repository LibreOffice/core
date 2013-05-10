# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,idlc))

$(eval $(call gb_Executable_set_include,idlc,\
    -I$(SRCDIR)/idlc/inc \
    -I$(SRCDIR)/idlc/source \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_externals,idlc,\
	boost_headers \
))

$(eval $(call gb_Executable_use_libraries,idlc,\
    reg \
    sal \
    salhelper \
))

$(eval $(call gb_Executable_add_grammars,idlc,\
    idlc/source/parser \
))

$(eval $(call gb_Executable_add_scanners,idlc,\
    idlc/source/scanner \
))

ifneq (,$(SYSTEM_UCPP))
$(eval $(call gb_Executable_add_defs,idlc,\
    -DSYSTEM_UCPP \
    -DUCPP=\"file://$(SYSTEM_UCPP)\" \
))
endif

$(eval $(call gb_Executable_add_exception_objects,idlc,\
    idlc/source/idlcmain \
    idlc/source/idlc \
    idlc/source/idlccompile \
    idlc/source/idlcproduce \
    idlc/source/errorhandler \
    idlc/source/options \
    idlc/source/fehelper \
    idlc/source/astdeclaration \
    idlc/source/astscope \
    idlc/source/aststack \
    idlc/source/astdump \
    idlc/source/astinterface \
    idlc/source/aststruct \
    idlc/source/aststructinstance \
    idlc/source/astoperation \
    idlc/source/astconstant \
    idlc/source/astenum \
    idlc/source/astarray \
    idlc/source/astunion \
    idlc/source/astexpression \
    idlc/source/astservice \
))

# vim:set noet sw=4 ts=4:
