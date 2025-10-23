# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

$(eval $(call gb_Module_Module,officelabs))

$(eval $(call gb_Module_add_targets,officelabs,\
    Library_officelabs \
))

# vim: set noet sw=4 ts=4: