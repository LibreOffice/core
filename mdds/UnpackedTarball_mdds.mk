# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mdds))

$(eval $(call gb_UnpackedTarball_set_tarball,mdds,$(MDDS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,mdds,3))

$(eval $(call gb_UnpackedTarball_add_patches,mdds,\
	mdds/mdds_0.6.0.patch \
	mdds/0001-Workaround-for-gcc-bug.patch \
))

$(eval $(call gb_UnpackedTarball_mark_output_files,mdds,\
	include/mdds/compat/unique_ptr.hpp \
	include/mdds/default_deleter.hpp \
	include/mdds/flat_segment_tree.hpp \
	include/mdds/flat_segment_tree_def.inl \
	include/mdds/flat_segment_tree_itr.hpp \
	include/mdds/global.hpp \
	include/mdds/hash_container/map.hpp \
	include/mdds/mixed_type_matrix.hpp \
	include/mdds/mixed_type_matrix_def.inl \
	include/mdds/mixed_type_matrix_element.hpp \
	include/mdds/mixed_type_matrix_flag_storage.hpp \
	include/mdds/mixed_type_matrix_storage.hpp \
	include/mdds/mixed_type_matrix_storage_filled_linear.inl \
	include/mdds/mixed_type_matrix_storage_sparse.inl \
	include/mdds/multi_type_matrix.hpp \
	include/mdds/multi_type_matrix_def.inl \
	include/mdds/multi_type_vector.hpp \
	include/mdds/multi_type_vector_def.inl \
	include/mdds/multi_type_vector_itr.hpp \
	include/mdds/multi_type_vector_macro.hpp \
	include/mdds/multi_type_vector_trait.hpp \
	include/mdds/multi_type_vector_types.hpp \
	include/mdds/node.hpp \
	include/mdds/point_quad_tree.hpp \
	include/mdds/quad_node.hpp \
	include/mdds/rectangle_set.hpp \
	include/mdds/rectangle_set_def.inl \
	include/mdds/segment_tree.hpp \
))

# vim: set noet sw=4 ts=4:
