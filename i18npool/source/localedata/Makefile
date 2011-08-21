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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

all : $(patsubst %.xml,localedata_%.cxx,$(notdir $(wildcard $(SRC_ROOT)/i18npool/source/localedata/data/*.xml)))

include $(GBUILDDIR)/gbuild_simple.mk

my_file := file://$(if $(filter $(OS_FOR_BUILD),WNT),/)
my_components := component/sax/source/expatwrap/expwrap.inbuild.component

localedata_%.cxx : localedata_%_invis.cxx
	sed 's/\(^.*get[^;]*$$\)/SAL_DLLPUBLIC_EXPORT \1/' $< > $@

localedata_%_invis.cxx : $(realpath $(SRC_ROOT)/i18npool/source/localedata/data)/%.xml saxparser.rdb
ifeq ($(OS_FOR_BUILD),WNT)
	$(gb_Helper_execute)saxparser $* `cygpath -m $<` $@ \
		$(my_file)`cygpath -m $(WORKDIR)/CustomTarget/i18npool/source/localedata/saxparser.rdb` `cygpath -m $(OUTDIR)/bin/types.rdb` \
		-env:OOO_INBUILD_SHAREDLIB_DIR=$(my_file)`cygpath -m $(OUTDIR)/bin`
else
	$(gb_Helper_execute)saxparser $* $< $@ \
		$(my_file)$(WORKDIR)/CustomTarget/i18npool/source/localedata/saxparser.rdb $(OUTDIR)/bin/types.rdb \
		-env:OOO_INBUILD_SHAREDLIB_DIR=$(my_file)$(OUTDIR)/lib
endif

saxparser.rdb : saxparser.input
ifeq ($(OS_FOR_BUILD),WNT)
	$(gb_XSLTPROC) --nonet --stringparam prefix `cygpath -m $(OUTDIR)/xml/` -o $@ \
		$(SOLARENV)/bin/packcomponents.xslt saxparser.input
else
	$(gb_XSLTPROC) --nonet --stringparam prefix $(OUTDIR)/xml/ -o $@ \
		$(SOLARENV)/bin/packcomponents.xslt saxparser.input
endif

saxparser.input :
	echo '<list><filename>$(my_components)</filename></list>' > $@

.PHONY: all
# vim: set noet sw=4 ts=4:
