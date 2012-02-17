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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

gb_PARTIALBUILD := T
include $(GBUILDDIR)/gbuild_simple.mk

done :
	$(call gb_Helper_abbreviate_dirs_native, \
	$(call gb_Helper_execute,cppumaker -C -BUCR -O. \
		-Tcom.sun.star.beans.NamedValue \
		-Tcom.sun.star.beans.XPropertySet \
		-Tcom.sun.star.container.XHierarchicalNameAccess \
		-Tcom.sun.star.lang.DisposedException \
		-Tcom.sun.star.lang.IllegalArgumentException \
		-Tcom.sun.star.lang.XComponent \
		-Tcom.sun.star.lang.XInitialization \
		-Tcom.sun.star.lang.XMultiComponentFactory \
		-Tcom.sun.star.lang.XMultiServiceFactory \
		-Tcom.sun.star.lang.XServiceInfo \
		-Tcom.sun.star.lang.XSingleComponentFactory \
		-Tcom.sun.star.lang.XSingleServiceFactory \
		-Tcom.sun.star.lang.XTypeProvider \
		-Tcom.sun.star.loader.XImplementationLoader \
		-Tcom.sun.star.registry.XRegistryKey \
		-Tcom.sun.star.registry.XSimpleRegistry \
		-Tcom.sun.star.uno.TypeClass \
		-Tcom.sun.star.uno.XAggregation \
		-Tcom.sun.star.uno.XComponentContext \
		-Tcom.sun.star.uno.XWeak \
		$(OUTDIR)/bin/udkapi.rdb) && \
	touch $@)

.DEFAULT_GOAL := all
.PHONY : all
all : done

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
