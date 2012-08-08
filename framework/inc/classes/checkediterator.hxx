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

#ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
#define __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_

#include <macros/debug.hxx>

#include <sal/types.h>

#include <iterator>

namespace framework{

/*-************************************************************************************************************//**
    @short          implement a iterator which support 2 end states!
    @descr          For our search methods we need a "walking" iterator object with special functionality!
                    We must check for 3 different states of an iterator - normal position, exact end, after end.
                    It's neccessary to detect if we have not found a entry and must return our default or
                    default already returned and we must break loop!
                    see using in class FilterCache too for further informations!

    @Attention      If your wish to debug this inline code ...
                    under windows and msdev you can use "set ENVCFLAGS=/Ob0" to do that!

    @implements     -
    @base           -

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/

template< class TContainer >
class CheckedIterator
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        // constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor
            @descr      Set default values on members.
                        We set it internal to E_UNKNOWN to detect uninitialized instances of this class.
                        If we found one - we know: "We must call initialize first!"

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        inline CheckedIterator()
                :   m_eEndState ( E_UNKNOWN )
                ,   m_pContainer( NULL      )
        {
        }

        //---------------------------------------------------------------------------------------------------------
        // interface methods
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      initialize instance with valid container
            @descr      Set new container at an instance of this class. The other member will set automaticly!
                        m_pPosition = first element in container
                        m_eEndState = BEFOREEND

            @seealso    -

            @param      "rContainer", must be a valid reference to an existing container.
            @return     -

            @onerror    An assertion is thrown.
        *//*-*****************************************************************************************************/

        inline void initialize( const TContainer& rContainer )
        {
            // Check incoming parameter. We don't accept all!
            LOG_ASSERT2( &rContainer==NULL      , "CheckedIterator::initialize()", "Invalid parameter detected!"                        )
            LOG_ASSERT2( m_eEndState!=E_UNKNOWN , "CheckedIterator::initialize()", "Instance already initialized! Don't do it again."   )

            if( m_eEndState == E_UNKNOWN )
            {
                // Set new container and update other member.
                m_pContainer = &rContainer          ;
                m_eEndState  = E_BEFOREEND          ;
                m_pPosition  = m_pContainer->begin();
            }
        }

        /*-****************************************************************************************************//**
            @short      set internal states to E_END
            @descr      Sometimes we need a "walking" check-iterator which is initialized with the END-state!
                        We need it to return one default value if no other ones exist ...

            @seealso    using in class FilterCache!

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        inline void setEnd()
        {
            m_pContainer = NULL  ;
            m_eEndState  = E_END ;
        }

        /*-****************************************************************************************************//**
            @short      set internal states to E_AFTEREND
            @descr      Sometimes we need a "walking" check-iterator which is initialized with AFTEREND-state!
                        We need it if we don't have a container but must prevent us against further searching!

            @seealso    using in class FilterCache!

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        inline void setAfterEnd()
        {
            m_pContainer = NULL       ;
            m_eEndState  = E_AFTEREND ;
        }

        /*-****************************************************************************************************//**
            @short      reset this iterator
            @descr      It must be called on an already initialized iterator.
                        Means the member m_pContainer must be valid. Otherwhise the reaction
                        isn't defined.

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        inline void reset()
        {
            m_eEndState  = E_UNKNOWN;
            m_pContainer = NULL;
        }

        /*-****************************************************************************************************//**
            @short      step to next element in container.
            @descr      If end of container is reached we change our internal "m_eEndState".
                        If end reached for first time; we set it to E_END;
                        If you step to next element again; we set it to E_AFTEREND.
                        So you have a chance to differ between "exact end" and "after end"!

            @seealso    method isEnd()
            @seealso    method isAfterEnd()

            @param      -
            @return     A reference to our changed object himself.

            @onerror    -
        *//*-*****************************************************************************************************/

        inline CheckedIterator& operator++()
        {
            // Warn programmer if he forget to initailize object!
            LOG_ASSERT2( m_pContainer==NULL, "CheckedIterator::operator++()", "Object not initialized!" )
            // Step to next element if any exist or set our end states.
            switch( m_eEndState )
            {
                case E_BEFOREEND:   {
                                        ++m_pPosition;
                                        // If iterator reaching end ... set right state!
                                        if( m_pPosition == m_pContainer->end() )
                                        {
                                            m_eEndState = E_END;
                                        }
                                    }
                                    break;
                case E_END      :   {
                                        // Set state only ... iterator already points to end of container!
                                        m_eEndState = E_AFTEREND;
                                    }
                                    break;
            }
            return *this;
        }

        /*-****************************************************************************************************//**
            @short      return true if internal iterator was not initialized before
            @descr      These will be true, if use start a new search by using these iterator mechanism!

            @seealso    class FilterCache

            @param      -
            @return     True if internalk state E_UNKNOWN - false otherwise.

            @onerror    -
        *//*-*****************************************************************************************************/

        inline sal_Bool isUninitialized()
        {
            return( m_eEndState == E_UNKNOWN );
        }

        /*-****************************************************************************************************//**
            @short      return true if internal iterator reached end of container
            @descr      These will be true if you step to the end of internal container.

            @seealso    method isAfterEnd()

            @param      -
            @return     True if end reached; false otherwise.

            @onerror    -
        *//*-*****************************************************************************************************/

        inline sal_Bool isEnd()
        {
            // Is true if one end state is set!
            return  (
                        ( m_eEndState == E_END      )   ||
                        ( m_eEndState == E_AFTEREND )
                    );
        }

        /*-****************************************************************************************************//**
            @short      return true if you call operator++ again and end already reached
            @descr      These indicate, that end already reached but you call operator++ again and again!

            @seealso    method isEnd()

            @param      -
            @return     True if end multiple reached; false otherwise.

            @onerror    -
        *//*-*****************************************************************************************************/

        inline sal_Bool isAfterEnd()
        {
            // Is true only, if special end state is set!
            return( m_eEndState == E_AFTEREND );
        }

        /*-****************************************************************************************************//**
            @short      support readonly access to container entry
            @descr      Use it to get the value of current container item.

            @seealso    -

            @param      -
            @return     A reference to value of container entry.

            @onerror    -
        *//*-*****************************************************************************************************/

        inline typename TContainer::const_iterator getEntry()
        {
            // Warn programmer if he forget to initialize these object ...
            LOG_ASSERT2( m_pContainer==NULL, "CheckedIterator::getEntry()", "Object not initialized!" )
            // or try to read a non existing element!
            LOG_ASSERT2( m_eEndState!=E_BEFOREEND, "CheckedIterator::getEntry()", "Wrong using of class detected!" )

            return m_pPosition;
        }

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        // These enum defines our four states for an iterator position in curent container.
        enum EEndState
        {
            E_UNKNOWN   ,
            E_BEFOREEND ,
            E_END       ,
            E_AFTEREND
        };

        const TContainer*           m_pContainer    ;   // pointer to current container
        EEndState                   m_eEndState     ;   // "position state" of iterator!
        typename TContainer::const_iterator  m_pPosition     ;   // point to actual element in container
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
