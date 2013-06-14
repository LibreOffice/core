# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,firebird,firebird))

$(eval $(call gb_ExternalPackage_use_external_project,firebird,firebird))

#$(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,inc/external/firebird,\
#	gen/firebird/include/ib_util.h \
#	gen/firebird/include/ibase.h \
#	gen/firebird/include/iberror.h \
#	gen/firebird/include/perf.h \
#))

# Need to be added?
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,share/firebird,\
# 	gen/install/misc/firebird.conf \
# ))
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,share/firebird,\
# 	gen/firebird/firebird.msg \
# ))
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,bin/firebird,\
# 	gen/firebird/bin/isql \
# 	gen/firebird/bin/isql_static \
# ))
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,share/firebird,\
# 	gen/install/misc/fbintl.conf \
# ))
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,share/firebird,\
# 	gen/firebird/intl/libfbintl.so \
# ))
# $(eval $(call gb_ExternalPackage_add_unpacked_files,firebird,share/firebird,\
# 	gen/firebird/security2.fdb \
# ))


$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.so.2.5.2,gen/firebird/lib/libfbembed.so.2.5.2))
$(eval $(call gb_ExternalPackage_add_library_for_install,firebird,lib/libfbembed.so.2.5.2,gen/firebird/lib/libfbembed.so.2.5.2))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.so,gen/firebird/lib/libfbembed.so.2.5.2))

# vim: set noet sw=4 ts=4:
