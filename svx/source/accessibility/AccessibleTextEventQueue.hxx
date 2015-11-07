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

#ifndef INCLUDED_SVX_SOURCE_ACCESSIBILITY_ACCESSIBLETEXTEVENTQUEUE_HXX
#define INCLUDED_SVX_SOURCE_ACCESSIBILITY_ACCESSIBLETEXTEVENTQUEUE_HXX

#include <memory>
#include <list>
#include <algorithm>

class SfxHint;
class SdrHint;
class TextHint;
class SvxViewHint;
class SvxEditSourceHint;

namespace accessibility
{
    /** This class handles the notification events for the
        AccessibleTextHelper class.

        For various reasons, we cannot process EditEngine events as
        they arrive, but have to queue and handle them in a batch.
     */
    class AccessibleTextEventQueue
    {
    public:
        typedef ::std::list< SfxHint* > EventQueue;

        AccessibleTextEventQueue();
        ~AccessibleTextEventQueue();

        /// Append event to end of queue
        void Append( const SdrHint& rHint );
        /// Append event to end of queue
        void Append( const TextHint& rHint );
        /// Append event to end of queue
        void Append( const SvxViewHint& rHint );
        /// Append event to end of queue
        void Append( const SvxEditSourceHint& rHint );

        /** Pop first queue element

            return first queue element, ownership transfers to caller
        */
        ::std::unique_ptr< SfxHint > PopFront();

        /** Apply functor to every queue member

            @param rFunctor
            Functor to apply. Functor receives queue element as
            parameter: void func( const SfxHint* );
        */
        template < typename Functor > void ForEach( Functor& rFunctor ) const
        {
            // #109864# Make sure results are put back into rFunctor
            rFunctor = ::std::for_each( maEventQueue.begin(), maEventQueue.end(), rFunctor );
        }

        /// Query whether queue is empty
        bool IsEmpty() const;

        /// Clear event queue
        void Clear();

    private:
        EventQueue maEventQueue;
    };

} // end of namespace accessibility

#endif // INCLUDED_SVX_SOURCE_ACCESSIBILITY_ACCESSIBLETEXTEVENTQUEUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
