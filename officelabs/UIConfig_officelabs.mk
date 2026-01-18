# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of OfficeLabs.
#

$(eval $(call gb_UIConfig_UIConfig,officelabs))

$(eval $(call gb_UIConfig_add_uifiles,officelabs,\
    officelabs/ui/agenticpanel \
))

# vim: set noet sw=4 ts=4:
