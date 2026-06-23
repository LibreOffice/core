# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,poco))

$(eval $(call gb_UnpackedTarball_set_tarball,poco,$(POCO_TARBALL)))

# All patches are no-ops where they don't apply:
# - emscripten-no-epoll: Emscripten looks like Linux to POCO but has no
#   <sys/epoll.h> (guarded by POCO_EMSCRIPTEN).
# - emscripten-no-prctl: Emscripten has no <sys/prctl.h>/prctl, which POCO uses
#   for thread naming on Linux (guarded by POCO_EMSCRIPTEN).
# - no-eventlogchannel: drop the Windows EventLogChannel, which would need a
#   message-compiler-generated pocomsg.h (only touches POCO_OS_FAMILY_WINDOWS
#   code).
$(eval $(call gb_UnpackedTarball_add_patches,poco,\
	external/poco/emscripten-no-epoll.patch \
	external/poco/emscripten-no-prctl.patch \
	external/poco/no-eventlogchannel.patch \
))

# POCO keeps each component's headers under <Component>/include/Poco/...;
# assemble them into a single include/ tree so that consumers (and our own
# static libraries) can use one -I path, and so that the engine delivers the
# headers to workdir/UnpackedTarball/poco/include as expected.
$(eval $(call gb_UnpackedTarball_set_post_action,poco,\
	mkdir -p include && \
	cp -RPf Foundation/include/. include/ && \
	cp -RPf XML/include/. include/ && \
	cp -RPf JSON/include/. include/ && \
	cp -RPf Util/include/. include/ && \
	cp -RPf Crypto/include/. include/ && \
	cp -RPf Net/include/. include/ && \
	cp -RPf NetSSL_OpenSSL/include/. include/ && \
	cp -RPf Zip/include/. include/ && \
	cp -RPf dependencies/tessil/include/. include/ \
))

# vim: set noet sw=4 ts=4:
