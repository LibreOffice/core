# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoJSON))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoJSON,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoJSON,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoJSON))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoJSON,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoJSON,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	-I$(gb_UnpackedTarball_workdir)/poco/JSON/include/Poco/JSON \
	-I$(gb_UnpackedTarball_workdir)/poco/dependencies/pdjson/src \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,PocoJSON,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoJSON,\
	UnpackedTarball/poco/JSON/src/Array \
	UnpackedTarball/poco/JSON/src/Handler \
	UnpackedTarball/poco/JSON/src/JSONException \
	UnpackedTarball/poco/JSON/src/Object \
	UnpackedTarball/poco/JSON/src/ParseHandler \
	UnpackedTarball/poco/JSON/src/Parser \
	UnpackedTarball/poco/JSON/src/ParserImpl \
	UnpackedTarball/poco/JSON/src/PrintHandler \
	UnpackedTarball/poco/JSON/src/Query \
	UnpackedTarball/poco/JSON/src/Stringifier \
	UnpackedTarball/poco/JSON/src/Template \
	UnpackedTarball/poco/JSON/src/TemplateCache \
))

# pdjson is the bundled streaming JSON parser that Poco::JSON::Parser is built
# on top of; compiled in here, consistent with the bundled zlib/pcre2/utf8proc.
$(eval $(call gb_StaticLibrary_add_generated_cobjects,PocoJSON,\
	UnpackedTarball/poco/dependencies/pdjson/src/pdjson \
))

# vim: set noet sw=4 ts=4:
