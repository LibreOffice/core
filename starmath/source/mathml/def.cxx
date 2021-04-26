/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mathml/attribute.hxx>

SmMlAttributePos starmathdatabase::MlAttributeListEmpty[] = {
    // clang-format off
    { SmMlAttributeValueType::NMlEmpty, 0 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMath[] = {
    // clang-format off
    { SmMlAttributeValueType::NMlEmpty, 0 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMi[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 },
    { SmMlAttributeValueType::MlDisplaystyle, 4 },
    { SmMlAttributeValueType::MlMathsize, 5 },
    { SmMlAttributeValueType::MlMathvariant, 6 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMerror[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlMathbackground, 1 },
    { SmMlAttributeValueType::MlMathcolor, 2 },
    { SmMlAttributeValueType::MlDisplaystyle, 3 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMn[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 },
    { SmMlAttributeValueType::MlDisplaystyle, 4 },
    { SmMlAttributeValueType::MlMathsize, 5 },
    { SmMlAttributeValueType::MlMathvariant, 6 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMo[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 },
    { SmMlAttributeValueType::MlDisplaystyle, 4 },
    { SmMlAttributeValueType::MlMathsize, 5 },
    { SmMlAttributeValueType::MlMathvariant, 6 },
    { SmMlAttributeValueType::MlFence, 7 },
    { SmMlAttributeValueType::MlMaxsize, 8 },
    { SmMlAttributeValueType::MlMinsize, 9 },
    { SmMlAttributeValueType::MlMovablelimits, 10 },
    { SmMlAttributeValueType::MlLspace, 11 },
    { SmMlAttributeValueType::MlRspace, 12 },
    { SmMlAttributeValueType::MlAccent, 13 },
    { SmMlAttributeValueType::MlStretchy, 14 },
    { SmMlAttributeValueType::MlSeparator, 15 },
    { SmMlAttributeValueType::MlSymmetric, 16 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMrow[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMtext[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 },
    { SmMlAttributeValueType::MlDisplaystyle, 4 },
    { SmMlAttributeValueType::MlMathsize, 5 },
    { SmMlAttributeValueType::MlMathvariant, 6 }
    // clang-format on
};

SmMlAttributePos starmathdatabase::MlAttributeListMstyle[] = {
    // clang-format off
    { SmMlAttributeValueType::MlHref, 0 },
    { SmMlAttributeValueType::MlDir, 1 },
    { SmMlAttributeValueType::MlMathbackground, 2 },
    { SmMlAttributeValueType::MlMathcolor, 3 },
    { SmMlAttributeValueType::MlDisplaystyle, 4 },
    { SmMlAttributeValueType::MlMathsize, 5 },
    { SmMlAttributeValueType::MlMathvariant, 6 },
    { SmMlAttributeValueType::MlFence, 7 },
    { SmMlAttributeValueType::MlMaxsize, 8 },
    { SmMlAttributeValueType::MlMinsize, 9 },
    { SmMlAttributeValueType::MlMovablelimits, 10 },
    { SmMlAttributeValueType::MlLspace, 11 },
    { SmMlAttributeValueType::MlRspace, 12 },
    { SmMlAttributeValueType::MlAccent, 13 },
    { SmMlAttributeValueType::MlStretchy, 14 },
    { SmMlAttributeValueType::MlSeparator, 15 },
    { SmMlAttributeValueType::MlSymmetric, 16 }
    // clang-format on
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
