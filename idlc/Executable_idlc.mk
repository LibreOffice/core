# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,idlc))

$(eval $(call gb_Executable_set_include,idlc,\
    -I$(SRCDIR)/idlc/inc \
    -I$(SRCDIR)/idlc/source \
    $$(INCLUDE) \
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
    -DUCPP=\"file:$(SYSTEM_UCPP)\" \
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
