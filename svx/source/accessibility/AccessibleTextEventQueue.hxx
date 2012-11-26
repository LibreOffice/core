/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVX_TEXT_CHANGED_QUEUE_HXX
#define _SVX_TEXT_CHANGED_QUEUE_HXX

#include <memory>
#include <list>
#include <algorithm>
#include <tools/solar.h>

class SfxHint;
class SdrBaseHint;
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
        void Append( const SdrBaseHint& rHint );
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
