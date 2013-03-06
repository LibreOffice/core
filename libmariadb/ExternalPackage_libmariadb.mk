# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libmariadb_inc,mariadb))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libmariadb_inc,inc/external/libmariadb,\
    include/config-win.h \
    include/dbug.h \
    include/errmsg.h \
    include/getopt.h \
    include/global.h \
    include/hash.h \
    include/m_ctype.h \
    include/m_string.h \
    include/my_alarm.h \
    include/my_base.h \
    include/my_dir.h \
    include/my_global.h \
    include/my_list.h \
    include/my_net.h \
    include/my_no_pthread.h \
    include/my_pthread.h \
    include/my_secure.h \
    include/mysql_com.h \
    include/mysqld_error.h \
    include/mysql.h \
    include/mysql_io.h \
    include/mysql_mm.h \
    include/mysql_priv.h \
    include/mysql_wireprotocol.h \
    include/my_stmt.h \
    include/mysys_err.h \
    include/my_sys.h \
    include/sha1.h \
    include/thr_alarm.h \
    include/violite.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libmariadb_inc,inc/external/libmariadb/mysql,\
    include/mysql/client_plugin.h \
    include/mysql/plugin_auth_common.h \
    include/mysql/plugin_auth.h \
))

# vim: set noet sw=4 ts=4:
