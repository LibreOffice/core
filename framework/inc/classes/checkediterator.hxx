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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_CHECKEDITERATOR_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_CHECKEDITERATOR_HXX

#include <sal/config.h>

#include <sal/log.hxx>
#include <sal/types.h>

#include <iterator>

namespace framework{

/*-************************************************************************************************************
    @short          implement a iterator which support 2 end states!
    @descr          For our search methods we need a "walking" iterator object with special functionality!
                    We must check for 3 different states of an iterator - normal position, exact end, after end.
                    It's necessary to detect if we have not found a entry and must return our default or
                    default already returned and we must break loop!
                    see using in class FilterCache too for further information!

    @Attention      If your wish to debug this inline code ...
                    under windows and msdev you can use "set ENVCFLAGS=/Ob0" to do that!
    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/

template< class TContainer >
class CheckedIterator
{

    //  public methods

    public:

        // constructor / destructor

        /*-****************************************************************************************************
            @short      standard constructor
            @descr      Set default values on members.
                        We set it internal to E_UNKNOWN to detect uninitialized instances of this class.
                        If we found one - we know: "We must call initialize first!"
        *//*-*****************************************************************************************************/

        inline CheckedIterator()
                :   m_eEndState ( E_UNKNOWN )
                ,   m_pContainer( NULL      )
        {
        }

        // interface methods

        /*-****************************************************************************************************
            @short      step to next element in container.
            @descr      If end of container is reached we change our internal "m_eEndState".
                        If end reached for first time; we set it to E_END;
                        If you step to next element again; we set it to E_AFTEREND.
                        So you have a chance to differ between "exact end" and "after end"!

            @return     A reference to our changed object himself.
        *//*-*****************************************************************************************************/

        inline CheckedIterator& operator++()
        {
            // Warn programmer if he forget to initailize object!
            SAL_WARN_IF( m_pContainer==NULL, "fwk", "CheckedIterator::operator++(): Object not initialized!" );
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

    //  private member

    private:

        // This enum defines our four states for an iterator position in current container.
        enum EEndState
        {
            E_UNKNOWN   ,
            E_BEFOREEND ,
            E_END       ,
            E_AFTEREND
        };

        const TContainer*           m_pContainer;   // pointer to current container
        EEndState                   m_eEndState;   // "position state" of iterator!
        typename TContainer::const_iterator  m_pPosition;   // point to actual element in container
};

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_CHECKEDITERATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
