/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_INC_EMFPLUSHELPER_HXX
#define INCLUDED_DRAWINGLAYER_INC_EMFPLUSHELPER_HXX

#include <sal/types.h>
#include <sal/config.h>
#include <memory>
#include <wmfemfhelper.hxx>

namespace emfplushelper { struct EmfPlusHelperData; }
namespace wmfemfhelper { class TargetHolders; }
namespace drawinglayer { namespace geometry { class ViewInformation2D; }}
class SvMemoryStream;

namespace emfplushelper
{
    /// EMF+ data holder
    class EmfPlusHelper
    {
    private:
        const std::unique_ptr<EmfPlusHelperData>  mpD;

    public:
        EmfPlusHelper(
            SvMemoryStream& rMemoryStream,
            wmfemfhelper::TargetHolders& rTargetHolders,
            wmfemfhelper::PropertyHolders& rPropertyHolders);
        ~EmfPlusHelper();

        void processEmfPlusData(
            SvMemoryStream& rMemoryStream,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
