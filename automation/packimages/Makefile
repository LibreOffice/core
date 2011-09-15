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

BINDIR := $(dir $(GBUILDDIR))bin
BASEDIR := $(dir $(firstword $(MAKEFILE_LIST)))

ILSTS := \
    $(OUTDIR)/res/img/fps_officeen-US.ilst \
    $(OUTDIR)/res/img/sben-US.ilst \
    $(OUTDIR)/res/img/stten-US.ilst \
    $(OUTDIR)/res/img/svten-US.ilst \
    $(OUTDIR)/res/img/vclen-US.ilst \

images_tt.zip :
	cp $(ILSTS) . \
	&& $(PERL) $(BINDIR)/packimages.pl -g $(SRCDIR)/default_images -m $(SRCDIR)/default_images -c $(CURDIR) -l $(CURDIR) -o $@

.DEFAULT_GOAL := all
.PHONY : all
all : images_tt.zip

# vim: set ts=4 sw=4 noet:
