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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SUBSETTABLESHAPEMANAGER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SUBSETTABLESHAPEMANAGER_HXX

#include "shapemanager.hxx"
#include "intrinsicanimationeventhandler.hxx"
#include <boost/shared_ptr.hpp>

/* Definition of SubsettableShapeManager interface */

namespace slideshow
{
    namespace internal
    {
        class DocTreeNode;
        class AttributableShape;

        /** SubsettableShapeManager interface

            Implementers of this interface manage creation and
            revocation of shape subsets. Shape subsets are shapes that
            represent (and animate) only parts of an original's shape
            content.
         */
        class SubsettableShapeManager : public ShapeManager
        {
        public:
            /** Query a subset of the given original shape

                This method queries a new (but not necessarily unique)
                shape, which displays only the given subset of the
                original one. Calling this method multiple times with
                the same original shape and DocTreeNode content always
                returns the same shape.

                Requesting a subset from an original shape leads to
                the original shape ceasing to display the subsetted
                content. In other words, shape content is always
                displayed in exactly one shape.

                @param rOrigShape
                The shape the subset is to be created for

                @param rSubsetShape
                The subset to display in the generated shape.
             */
            virtual boost::shared_ptr<AttributableShape> getSubsetShape(
                const boost::shared_ptr<AttributableShape>& rOrigShape,
                const DocTreeNode&                          rTreeNode ) = 0;

            /** Revoke a previously queried subset shape.

                With this method, a previously requested subset shape
                is revoked again. If the last client revokes a given
                subset, it will cease to be displayed, and the
                original shape will again show the subset data.

                @param rOrigShape
                The shape the subset was created from

                @param rSubsetShape
                The subset created from rOrigShape
             */
            virtual void revokeSubset(
                const boost::shared_ptr<AttributableShape>& rOrigShape,
                const boost::shared_ptr<AttributableShape>& rSubsetShape ) = 0;

            // Evil hackish way of getting intrinsic animation slide-wise

            /** Register an event handler that will be called when
                user paint parameters change.

                @param rHandler
                Handler to call when a shape listener changes
            */
            virtual void addIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler ) = 0;
            virtual void removeIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler ) = 0;

            /** Notify that shape-intrinsic animations are now enabled.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
            */
            virtual bool notifyIntrinsicAnimationsEnabled() = 0;

            /** Notify that shape-intrinsic animations are now disabled.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
            */
            virtual bool notifyIntrinsicAnimationsDisabled() = 0;
        };

        typedef ::boost::shared_ptr< SubsettableShapeManager > SubsettableShapeManagerSharedPtr;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SUBSETTABLESHAPEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
