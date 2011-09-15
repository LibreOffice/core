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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

include $(GBUILDDIR)/gbuild_simple.mk

BASEDIR := $(dir $(firstword $(MAKEFILE_LIST)))

define filter_header
$(1).hxx : $(3)/$(2)
	$(PERL) $(BASEDIR)/filter.pl $(3) $(2) $(1) $(4)

endef

$(eval $(call filter_header,classes,rcontrol.hxx,$(realpath $(BASEDIR)/../inc),M_))
$(eval $(call filter_header,keycodes,vcl/keycodes.hxx,$(OUTDIR)/inc,KEY_))
$(eval $(call filter_header,r_cmds,rcontrol.hxx,$(realpath $(BASEDIR)/../inc),RC_))
$(eval $(call filter_header,res_type,tools/wintypes.hxx,$(OUTDIR)/inc,WINDOW_))

$(CURDIR)/testtool.ini : $(BASEDIR)/testtool.ini
ifeq ($(GUI),UNX)
	sed -e s/!INIFILESUFFIX!/rc/ -e s/!BRANDPATH!// $< > $@
else ifeq ($(GUI),WNT)
	sed -e s/!INIFILESUFFIX!/.ini/ -e s/!BRANDPATH!/../ $< > $@
endif

.DEFAULT_GOAL := all
.PHONY : all
all : \
	classes.hxx \
	keycodes.hxx \
	r_cmds.hxx \
	res_type.hxx \
	$(CURDIR)/testtool.ini

# vim: set ts=4 sw=4 noet:
