/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_TEXT_CHANGED_QUEUE_HXX
#define _SVX_TEXT_CHANGED_QUEUE_HXX

#include <memory>
#include <list>
#include <algorithm>
#include <tools/solar.h>
#include <tools/rtti.hxx>

class SfxHint;
class SdrHint;
class SfxSimpleHint;
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
        void Append( const SfxHint& rHint );
        /// Append event to end of queue
        void Append( const SdrHint& rHint );
        /// Append event to end of queue
        void Append( const SfxSimpleHint& rHint );
        /// Append event to end of queue
        void Append( const TextHint& rHint );
        /// Append event to end of queue
        void Append( const SvxViewHint& rHint );
        /// Append event to end of queue
        void Append( const SvxEditSourceHint& rHint );

        /** Pop first queue element

            return first queue element, ownership transfers to caller
        */
        ::std::auto_ptr< SfxHint > PopFront();

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

#endif /* _SVX_TEXT_CHANGED_QUEUE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
