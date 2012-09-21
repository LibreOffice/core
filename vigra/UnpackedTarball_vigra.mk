# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,vigra))

$(eval $(call gb_UnpackedTarball_set_tarball,vigra,$(VIGRA_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,vigra,\
	vigra/vigra1.4.0.patch \
	$(if $(filter GCC,$(COM)), \
		vigra/vigra1.4.0-unused-parameters.patch) \
	vigra/vigra1.4.0-enumwarn.patch \
))

$(eval $(call gb_UnpackedTarball_mark_output_files,vigra,\
	include/vigra/accessor.hxx \
	include/vigra/array_vector.hxx \
	include/vigra/basicgeometry.hxx \
	include/vigra/basicimage.hxx \
	include/vigra/basicimageview.hxx \
	include/vigra/bordertreatment.hxx \
	include/vigra/boundarytensor.hxx \
	include/vigra/codec.hxx \
	include/vigra/colorconversions.hxx \
	include/vigra/combineimages.hxx \
	include/vigra/config.hxx \
	include/vigra/contourcirculator.hxx \
	include/vigra/convolution.hxx \
	include/vigra/copyimage.hxx \
	include/vigra/cornerdetection.hxx \
	include/vigra/diff2d.hxx \
	include/vigra/distancetransform.hxx \
	include/vigra/edgedetection.hxx \
	include/vigra/eigensystem.hxx \
	include/vigra/error.hxx \
	include/vigra/fftw.hxx \
	include/vigra/fftw3.hxx \
	include/vigra/fixedpoint.hxx \
	include/vigra/flatmorphology.hxx \
	include/vigra/functorexpression.hxx \
	include/vigra/functortraits.hxx \
	include/vigra/gaborfilter.hxx \
	include/vigra/gaussians.hxx \
	include/vigra/gradient_energy_tensor.hxx \
	include/vigra/imagecontainer.hxx \
	include/vigra/imageinfo.hxx \
	include/vigra/imageiterator.hxx \
	include/vigra/imageiteratoradapter.hxx \
	include/vigra/impex.hxx \
	include/vigra/initimage.hxx \
	include/vigra/inspectimage.hxx \
	include/vigra/interpolating_accessor.hxx \
	include/vigra/iteratoradapter.hxx \
	include/vigra/iteratortags.hxx \
	include/vigra/iteratortraits.hxx \
	include/vigra/labelimage.hxx \
	include/vigra/linear_algebra.hxx \
	include/vigra/linear_solve.hxx \
	include/vigra/localminmax.hxx \
	include/vigra/mathutil.hxx \
	include/vigra/matrix.hxx \
	include/vigra/memory.hxx \
	include/vigra/metaprogramming.hxx \
	include/vigra/multi_array.hxx \
	include/vigra/multi_convolution.hxx \
	include/vigra/multi_impex.hxx \
	include/vigra/multi_iterator.hxx \
	include/vigra/multi_pointoperators.hxx \
	include/vigra/navigator.hxx \
	include/vigra/nonlineardiffusion.hxx \
	include/vigra/numerictraits.hxx \
	include/vigra/orientedtensorfilters.hxx \
	include/vigra/pixelneighborhood.hxx \
	include/vigra/polynomial.hxx \
	include/vigra/rational.hxx \
	include/vigra/recursiveconvolution.hxx \
	include/vigra/resampling_convolution.hxx \
	include/vigra/resizeimage.hxx \
	include/vigra/rfftw.hxx \
	include/vigra/rgbvalue.hxx \
	include/vigra/seededregiongrowing.hxx \
	include/vigra/separableconvolution.hxx \
	include/vigra/sized_int.hxx \
	include/vigra/splineimageview.hxx \
	include/vigra/splines.hxx \
	include/vigra/static_assert.hxx \
	include/vigra/stdconvolution.hxx \
	include/vigra/stdimage.hxx \
	include/vigra/stdimagefunctions.hxx \
	include/vigra/symmetry.hxx \
	include/vigra/tensorutilities.hxx \
	include/vigra/tiff.hxx \
	include/vigra/tinyvector.hxx \
	include/vigra/transformimage.hxx \
	include/vigra/tuple.hxx \
	include/vigra/utilities.hxx \
	include/vigra/watersheds.hxx \
	include/vigra/windows.h \
))

# vim: set noet sw=4 ts=4:
