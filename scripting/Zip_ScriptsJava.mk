# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,ScriptsJava,$(WORKDIR)/Zip/ScriptsJava))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/HelloWorld/HelloWorld.jar,\
	$(OUTDIR)/bin/HelloWorld.jar \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/HelloWorld/HelloWorld.java,\
	$(SRCDIR)/scripting/examples/java/HelloWorld/HelloWorld.java \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/HelloWorld/parcel-descriptor.xml,\
	$(SRCDIR)/scripting/examples/java/HelloWorld/parcel-descriptor.xml \
))


$(eval $(call gb_Zip_add_file,ScriptsJava,java/Highlight/Highlight.jar,\
	$(OUTDIR)/bin/Highlight.jar \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/Highlight/HighlightText.java,\
	$(SRCDIR)/scripting/examples/java/Highlight/HighlightText.java \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/Highlight/parcel-descriptor.xml,\
	$(SRCDIR)/scripting/examples/java/Highlight/parcel-descriptor.xml \
))


$(eval $(call gb_Zip_add_file,ScriptsJava,java/MemoryUsage/MemoryUsage.jar,\
	$(OUTDIR)/bin/MemoryUsage.jar \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/MemoryUsage/MemoryUsage.java,\
	$(SRCDIR)/scripting/examples/java/MemoryUsage/MemoryUsage.java \
))

$(eval $(call gb_Zip_add_file,ScriptsJava,java/MemoryUsage/parcel-descriptor.xml,\
	$(SRCDIR)/scripting/examples/java/MemoryUsage/parcel-descriptor.xml \
))

# vim: set noet sw=4 ts=4:
