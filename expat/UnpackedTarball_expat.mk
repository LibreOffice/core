# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,expat))

$(eval $(call gb_UnpackedTarball_set_tarball,expat,$(EXPAT_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,expat,\
	expat/expat-2.1.0.patch \
))

# This is a bit hackish
# we need to compile it twice:
# with -DXML_UNICODE and without.

# This is a bit hackish too ;-)
# on windows 64 bit platform we need to link it twice:
# with $(LINK_X64_BINARY) and with $(gb_LINK).
$(eval $(call gb_UnpackedTarball_set_post_action,expat,\
	cp lib/xmlparse.c lib/unicode_xmlparse.c \
	$(if $(filter $(BUILD_X64),TRUE),      && \
	  cp lib/xmlparse.c lib/xmlparse_x64.c && \
	  cp lib/xmltok.c lib/xmltok_x64.c     && \
	  cp lib/xmlrole.c lib/xmlrole_x64.c) \
))

# vim: set noet sw=4 ts=4:
