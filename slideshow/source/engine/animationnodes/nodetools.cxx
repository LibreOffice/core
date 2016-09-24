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


#include <com/sun/star/animations/Timing.hpp>

#include <tools.hxx>
#include <nodetools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
#if defined(DBG_UTIL)
        static sal_Int32 lcl_nOffset = 0;

        OUString debugGetNodeName( const BaseNode *pNode )
        {
            OUStringBuffer aBuf;
            aBuf.append(lcl_nOffset);
            aBuf.append(" - 0x");
            aBuf.append(reinterpret_cast<sal_Int64>(pNode), 16);
            return aBuf.makeStringAndClear();
        }

        void debugNodesShowTree( const BaseNode* pNode )
        {
            if( pNode )
                pNode->showState();

            ++lcl_nOffset;
        }
#endif

        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const uno::Reference< drawing::XShape >&    xShape          )
        {
            ENSURE_OR_THROW( rShapeManager,
                              "lookupAttributableShape(): invalid ShapeManager" );

            ShapeSharedPtr pShape( rShapeManager->lookupShape( xShape ) );

            ENSURE_OR_THROW( pShape,
                              "lookupAttributableShape(): no shape found for given XShape" );

            AttributableShapeSharedPtr pRes(
                ::std::dynamic_pointer_cast< AttributableShape >( pShape ) );

            // TODO(E3): Cannot throw here, people might set animation info
            // for non-animatable shapes from the API. AnimationNodes must catch
            // the exception and handle that differently
            ENSURE_OR_THROW( pRes,
                              "lookupAttributableShape(): shape found does not implement AttributableShape interface" );

            return pRes;
        }

        bool isIndefiniteTiming( const uno::Any& rAny )
        {
            if( !rAny.hasValue() )
                return true;

            animations::Timing eTiming;

            if( !(rAny >>= eTiming) ||
                eTiming != animations::Timing_INDEFINITE )
            {
                return false;
            }

            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
