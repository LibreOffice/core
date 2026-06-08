# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoCrypto))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoCrypto,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoCrypto,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoCrypto))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoCrypto,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoCrypto,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,PocoCrypto,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
))

# OpenSSL comes from the engine's own externals, not POCO's bundled copy.
# A static library only compiles (no link), so the headers are enough; the
# actual libssl/libcrypto are linked by the final consumer (COOL).
$(eval $(call gb_StaticLibrary_use_externals,PocoCrypto,\
	openssl_headers \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoCrypto,\
	UnpackedTarball/poco/Crypto/src/Cipher \
	UnpackedTarball/poco/Crypto/src/CipherFactory \
	UnpackedTarball/poco/Crypto/src/CipherImpl \
	UnpackedTarball/poco/Crypto/src/CipherKey \
	UnpackedTarball/poco/Crypto/src/CipherKeyImpl \
	UnpackedTarball/poco/Crypto/src/CryptoException \
	UnpackedTarball/poco/Crypto/src/CryptoStream \
	UnpackedTarball/poco/Crypto/src/CryptoTransform \
	UnpackedTarball/poco/Crypto/src/DigestEngine \
	UnpackedTarball/poco/Crypto/src/ECDSADigestEngine \
	UnpackedTarball/poco/Crypto/src/ECKey \
	UnpackedTarball/poco/Crypto/src/ECKeyImpl \
	UnpackedTarball/poco/Crypto/src/Envelope \
	UnpackedTarball/poco/Crypto/src/EVPCipherImpl \
	UnpackedTarball/poco/Crypto/src/EVPPKey \
	UnpackedTarball/poco/Crypto/src/KeyPair \
	UnpackedTarball/poco/Crypto/src/KeyPairImpl \
	UnpackedTarball/poco/Crypto/src/OpenSSLInitializer \
	UnpackedTarball/poco/Crypto/src/PKCS12Container \
	UnpackedTarball/poco/Crypto/src/RSACipherImpl \
	UnpackedTarball/poco/Crypto/src/RSADigestEngine \
	UnpackedTarball/poco/Crypto/src/RSAKey \
	UnpackedTarball/poco/Crypto/src/RSAKeyImpl \
	UnpackedTarball/poco/Crypto/src/X509Certificate \
))

# vim: set noet sw=4 ts=4:
