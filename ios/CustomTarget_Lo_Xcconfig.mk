# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,ios/Lo_Xcconfig))

LO_XCCONFIG = $(BUILDDIR)/ios/lo.xcconfig

$(call gb_CustomTarget_get_target,ios/Lo_Xcconfig): $(LO_XCCONFIG)

.PHONY : $(LO_XCCONFIG)

$(LO_XCCONFIG) :
    # Edit the Xcode configuration file:
    # - the list of all our (static) libs
    # - compiler flags
    #
	all_libs=`$(SRCDIR)/bin/lo-all-static-libs`; \
	\
	sed -e "s,^\(LINK_LDFLAGS =\).*$$,\1 $$all_libs," \
		-e "s,^\(OTHER_CFLAGS =\).*$$,\1 $(gb_GLOBALDEFS)," \
		-e "s,^\(OTHER_CPLUSPLUSFLAGS =\).*$$,\1 $(gb_GLOBALDEFS)," \
			< $(LO_XCCONFIG) > $(LO_XCCONFIG).new && mv $(LO_XCCONFIG).new $(LO_XCCONFIG)

    # When SRCDIR!=BUILDDIR, Xcode is used on the project in the
    # *source* tree (because that is where the source files are). Copy
    # the configuration file to the corresponding source dir so that
    # Xcode can find it.
	if test $(SRCDIR) != $(BUILDDIR); then \
		cp $(LO_XCCONFIG) $(SRCDIR)/ios; \
	fi

# Do *not* remove $(LO_XCCONFIG) in a clean target. It is created
# during configury, not in this custom gbuild makefile.

# vim: set noet sw=4 ts=4:
