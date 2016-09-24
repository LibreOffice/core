# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,scripting_java_jars,$(WORKDIR)/Jar))

$(eval $(call gb_Package_add_file,scripting_java_jars,$(LIBO_SHARE_FOLDER)/Scripts/java/HelloWorld/HelloWorld.jar,HelloWorld.jar))
$(eval $(call gb_Package_add_file,scripting_java_jars,$(LIBO_SHARE_FOLDER)/Scripts/java/Highlight/Highlight.jar,Highlight.jar))
$(eval $(call gb_Package_add_file,scripting_java_jars,$(LIBO_SHARE_FOLDER)/Scripts/java/MemoryUsage/MemoryUsage.jar,MemoryUsage.jar))

# vim: set noet sw=4 ts=4:
