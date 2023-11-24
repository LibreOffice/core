#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ = ../../..
SETTINGS = $(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

FILES = \
    Addons.xcu \
    META-INF/manifest.xml \
    description.xml \
    pkg-description/pkg-description.en \
    registration/license.txt \
    main.py

$(OUT_BIN)/minimal-python.$(UNOOXT_EXT): $(FILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $^
