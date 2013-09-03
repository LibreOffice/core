# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,openssl))

$(eval $(call gb_UnpackedTarball_set_tarball,openssl,$(OPENSSL_TARBALL),,openssl))

# hack to fix symlinks with MSVC
$(eval $(call gb_UnpackedTarball_fix_end_of_line,openssl,\
	include/openssl/aes.h \
	include/openssl/asn1.h \
	include/openssl/asn1_mac.h \
	include/openssl/asn1t.h \
	include/openssl/bio.h \
	include/openssl/blowfish.h \
	include/openssl/bn.h \
	include/openssl/buffer.h \
	include/openssl/camellia.h \
	include/openssl/cast.h \
	include/openssl/cmac.h \
	include/openssl/cms.h \
	include/openssl/comp.h \
	include/openssl/conf.h \
	include/openssl/conf_api.h \
	include/openssl/crypto.h \
	include/openssl/des.h \
	include/openssl/des_old.h \
	include/openssl/dh.h \
	include/openssl/dsa.h \
	include/openssl/dso.h \
	include/openssl/dtls1.h \
	include/openssl/e_os2.h \
	include/openssl/ebcdic.h \
	include/openssl/ec.h \
	include/openssl/ecdh.h \
	include/openssl/ecdsa.h \
	include/openssl/engine.h \
	include/openssl/err.h \
	include/openssl/evp.h \
	include/openssl/hmac.h \
	include/openssl/idea.h \
	include/openssl/krb5_asn.h \
	include/openssl/kssl.h \
	include/openssl/lhash.h \
	include/openssl/md4.h \
	include/openssl/md5.h \
	include/openssl/mdc2.h \
	include/openssl/modes.h \
	include/openssl/obj_mac.h \
	include/openssl/objects.h \
	include/openssl/ocsp.h \
	include/openssl/opensslconf.h \
	include/openssl/opensslv.h \
	include/openssl/ossl_typ.h \
	include/openssl/pem.h \
	include/openssl/pem2.h \
	include/openssl/pkcs12.h \
	include/openssl/pkcs7.h \
	include/openssl/pqueue.h \
	include/openssl/rand.h \
	include/openssl/rc2.h \
	include/openssl/rc4.h \
	include/openssl/ripemd.h \
	include/openssl/rsa.h \
	include/openssl/safestack.h \
	include/openssl/seed.h \
	include/openssl/sha.h \
	include/openssl/srp.h \
	include/openssl/srtp.h \
	include/openssl/ssl.h \
	include/openssl/ssl2.h \
	include/openssl/ssl23.h \
	include/openssl/ssl3.h \
	include/openssl/stack.h \
	include/openssl/symhacks.h \
	include/openssl/tls1.h \
	include/openssl/ts.h \
	include/openssl/txt_db.h \
	include/openssl/ui.h \
	include/openssl/ui_compat.h \
	include/openssl/whrlpool.h \
	include/openssl/x509.h \
	include/openssl/x509_vfy.h \
	include/openssl/x509v3.h \
))

$(eval $(call gb_UnpackedTarball_add_patches,openssl,\
	$(if $(filter LINUX FREEBSD ANDROID,$(OS)),openssl/openssllnx.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),openssl/opensslmingw.patch) \
	$(if $(filter MSC,$(COM)),openssl/opensslwnt.patch) \
	$(if $(filter SOLARIS,$(OS)),openssl/opensslsol.patch) \
	$(if $(filter IOS,$(OS)),openssl/opensslios.patch) \
))

# vim: set noet sw=4 ts=4:
