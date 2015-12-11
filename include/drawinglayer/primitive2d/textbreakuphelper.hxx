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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTBREAKUPHELPER_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTBREAKUPHELPER_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        enum BreakupUnit
        {
            BreakupUnit_character,
            BreakupUnit_word,
            BreakupUnit_sentence
        };

        class DRAWINGLAYER_DLLPUBLIC TextBreakupHelper
        {
        private:
            const TextSimplePortionPrimitive2D&     mrSource;
            Primitive2DContainer                       mxResult;
            TextLayouterDevice                      maTextLayouter;
            basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose maDecTrans;

            /// bitfield
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
            const Primitive2DContainer& getResult(BreakupUnit aBreakupUnit = BreakupUnit_character) const;
        };

    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTBREAKUPHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
