# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mariadb-connector-c))

$(eval $(call gb_UnpackedTarball_set_tarball,mariadb-connector-c,$(MARIADB_CONNECTOR_C_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/mariadb_version.h,external/mariadb-connector-c/configs/mariadb_version.h))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/ma_config.h,external/mariadb-connector-c/configs/wnt_ma_config.h))
else
ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/ma_config.h,external/mariadb-connector-c/configs/mac_my_config.h))
else
$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/ma_config.h,external/mariadb-connector-c/configs/linux_my_config.h))
endif
endif # $(OS),WNT

$(eval $(call gb_UnpackedTarball_set_patchlevel,mariadb-connector-c,1))

$(eval $(call gb_UnpackedTarball_add_patches,mariadb-connector-c,\
    external/mariadb-connector-c/clang-cl.patch.0 \
))

# TODO are any "plugins" needed?
$(eval $(call gb_UnpackedTarball_set_post_action,mariadb-connector-c, \
	< libmariadb/ma_client_plugin.c.in sed \
		-e 's/@EXTERNAL_PLUGINS@/ \
			extern struct st_mysql_client_plugin pvio_socket_client_plugin\; \
			extern struct st_mysql_client_plugin caching_sha2_password_client_plugin\; \
			extern struct st_mysql_client_plugin mysql_native_password_client_plugin\; \
			extern struct st_mysql_client_plugin_AUTHENTICATION auth_gssapi_client_client_plugin\; \
			$(if $(filter WNT,$(OS)), \
				extern struct st_mysql_client_plugin pvio_shmem_client_plugin\; \
				extern struct st_mysql_client_plugin pvio_npipe_client_plugin\; \
			) \
			/' \
		-e 's/@BUILTIN_PLUGINS@/ \
			(struct st_mysql_client_plugin *)\&pvio_socket_client_plugin$(COMMA) \
			(struct st_mysql_client_plugin *)\&caching_sha2_password_client_plugin$(COMMA) \
			(struct st_mysql_client_plugin *)\&mysql_native_password_client_plugin$(COMMA) \
			(struct st_mysql_client_plugin *)\&auth_gssapi_client_client_plugin$(COMMA) \
			$(if $(filter WNT,$(OS)), \
				(struct st_mysql_client_plugin *)\&pvio_shmem_client_plugin$(COMMA) \
				(struct st_mysql_client_plugin *)\&pvio_npipe_client_plugin$(COMMA) \
			) \
			/' \
		> libmariadb/ma_client_plugin.c \
))


# vim: set noet sw=4 ts=4:
