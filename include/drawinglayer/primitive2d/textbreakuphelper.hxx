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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace drawinglayer::primitive2d { class TextSimplePortionPrimitive2D; }


namespace drawinglayer::primitive2d
{
        enum class BreakupUnit
        {
            Character,
            Word
        };

        class DRAWINGLAYER_DLLPUBLIC TextBreakupHelper
        {
        private:
            const TextSimplePortionPrimitive2D&     mrSource;
            Primitive2DContainer                       mxResult;
            TextLayouterDevice                      maTextLayouter;
            basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose maDecTrans;

            bool                                    mbNoDXArray : 1;

            /// create a portion from nIndex to nLength and append to rTempResult
            void breakupPortion(Primitive2DContainer& rTempResult, sal_Int32 nIndex, sal_Int32 nLength, bool bWordLineMode);

            /// breakup complete primitive
            void breakup(BreakupUnit aBreakupUnit);

        protected:
            /// allow user callback to allow changes to the new TextTransformation. Default
            /// does nothing. Retval defines if a primitive gets created, e.g. return false
            /// to suppress creation
            virtual bool allowChange(sal_uInt32 nCount, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 nIndex, sal_uInt32 nLength);

            /// allow read access to evtl. useful local parts
            const TextLayouterDevice& getTextLayouter() const { return maTextLayouter; }
            const TextSimplePortionPrimitive2D& getSource() const { return mrSource; }

        public:
            TextBreakupHelper(const TextSimplePortionPrimitive2D& rSource);
            virtual ~TextBreakupHelper();

            /// get result
            const Primitive2DContainer& getResult(BreakupUnit aBreakupUnit = BreakupUnit::Character) const;
        };

} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
