# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,brotli))

$(eval $(call gb_StaticLibrary_use_unpacked,brotli,brotli))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,brotli))

$(eval $(call gb_StaticLibrary_set_include,brotli,\
    -I$(WORKDIR)/UnpackedTarball/brotli \
    -I$(WORKDIR)/UnpackedTarball/brotli/c/include \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,brotli,\
    $(addprefix UnpackedTarball/brotli/c/common/, \
        constants \
        context \
        dictionary \
        platform \
        shared_dictionary \
        transform \
    ) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,brotli,\
    $(addprefix UnpackedTarball/brotli/c/enc/, \
        backward_references \
        backward_references_hq \
        bit_cost \
        block_splitter \
        brotli_bit_stream \
        cluster \
        command \
        compound_dictionary \
        compress_fragment \
        compress_fragment_two_pass \
        dictionary_hash \
        encode \
        encoder_dict \
        entropy_encode \
        fast_log \
        histogram \
        literal_cost \
        memory \
        metablock \
        static_dict \
        static_dict_lut \
        static_init \
        utf8_util \
    ) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,brotli,\
    $(addprefix UnpackedTarball/brotli/c/dec/, \
        bit_reader \
        decode \
        huffman \
        prefix \
        state \
        static_init \
    ) \
))

# vim: set noet sw=4 ts=4:
