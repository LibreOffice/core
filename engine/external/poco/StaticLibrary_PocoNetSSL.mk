# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoNetSSL))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoNetSSL,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoNetSSL,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoNetSSL))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoNetSSL,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoNetSSL,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,PocoNetSSL,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
))

# Builds on PocoNet and PocoCrypto; OpenSSL headers come from the engine's
# externals (the static archive only compiles; COOL links libssl/libcrypto).
$(eval $(call gb_StaticLibrary_use_externals,PocoNetSSL,\
	openssl_headers \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoNetSSL,\
	UnpackedTarball/poco/NetSSL_OpenSSL/src/AcceptCertificateHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/CertificateHandlerFactory \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/CertificateHandlerFactoryMgr \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/ConsoleCertificateHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/Context \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/FTPSClientSession \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/FTPSStreamFactory \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/HTTPSClientSession \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/HTTPSSessionInstantiator \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/HTTPSStreamFactory \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/InvalidCertificateHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/KeyConsoleHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/KeyFileHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/PrivateKeyFactory \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/PrivateKeyFactoryMgr \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/PrivateKeyPassphraseHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/RejectCertificateHandler \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureServerSocket \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureServerSocketImpl \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureSMTPClientSession \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureSocketImpl \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureStreamSocket \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SecureStreamSocketImpl \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/Session \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SSLException \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/SSLManager \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/Utility \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/VerificationErrorArgs \
	UnpackedTarball/poco/NetSSL_OpenSSL/src/X509Certificate \
))

# vim: set noet sw=4 ts=4:
