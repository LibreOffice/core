/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <skia_opts.hxx>
#define SK_OPTS_NS ssse3
#include "skia_opts_internal.hxx"

namespace SkLoOpts {
    void Init_ssse3() {
        RGB1_to_RGB           = ssse3::RGB1_to_RGB;
        RGB1_to_R             = ssse3::RGB1_to_R;
    }
}
