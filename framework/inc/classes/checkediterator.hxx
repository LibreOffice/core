/*************************************************************************
 *
 *  $RCSfile: checkediterator.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: as $ $Date: 2001-04-04 13:28:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
#define __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef __SGI_STL_ITERATOR
#include <iterator>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

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
                :   m_eEndState( E_UNKNOWN )
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

            @param      "pContainer", must be a valid pointer to an existing container.
            @return     -

            @onerror    An assertion is thrown.
        *//*-*****************************************************************************************************/

        inline void initialize( const TContainer* pContainer, sal_Bool bReverse = sal_False )
        {
            // Check incoming parameter. We don't accept all!
            LOG_ASSERT2( pContainer ==NULL      , "CheckedIterator::initialize()", "Invalid parameter detected!"                        )
            LOG_ASSERT2( m_eEndState!=E_UNKNOWN , "CheckedIterator::initialize()", "Instance already initialized! Don't do it again."   )

            if( m_eEndState == E_UNKNOWN )
            {
                // Set new container and actualize other member.
                m_pContainer        = pContainer                ;
                m_bReverse          = bReverse                  ;
                m_eEndState         = E_BEFOREEND               ;
                m_nForward          = 0                         ;
                m_nBackward         = m_pContainer->size()-1    ;// -1 .. because first index is 0!
            }
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
            m_pContainer    = NULL          ;
            m_eEndState     = E_AFTEREND    ;
            m_bReverse      = sal_False     ;
            m_nForward      = 0             ;
            m_nBackward     = 0             ;
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
            LOG_ASSERT( !(m_pContainer==NULL), "CheckedIterator::operator++()\nObject not initialized!\n" )
            // Step to next element if any exist or set our end states.
            switch( m_eEndState )
            {
                case E_BEFOREEND:   {
                                        ++m_nForward    ;
                                        --m_nBackward   ;
                                        // If one iterator reaching end ... other iterator must do the same automaticly!
                                        if  (
                                                ( m_nForward    >=  (sal_Int32)m_pContainer->size() )   ||
                                                ( m_nBackward   <   0                               )
                                            )
                                        {
                                            m_eEndState = E_END;
                                        }
                                    }
                                    break;
                case E_END      :   {
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
            return  ( m_eEndState == E_UNKNOWN );
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
            LOG_ASSERT( !(m_pContainer==NULL)       , "CheckedIterator::getEntry()\nObject not initialized!\n"          )
            // or try to read a non existing element!
            LOG_ASSERT( !(m_eEndState!=E_BEFOREEND) , "CheckedIterator::getEntry()\nWrong using of class detected!\n"   )

            typename TContainer::const_iterator pEntry = m_pContainer->begin();
             if( m_bReverse == sal_True )
            {
                for( sal_Int32 i=0; i<m_nBackward; ++i )
                {
                    ++pEntry;
                }
            }
            else
            {
                for( sal_Int32 i=0; i<m_nForward; ++i )
                {
                    ++pEntry;
                }
            }
            return pEntry;
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

        const TContainer*                   m_pContainer    ;               // pointer to current container
        sal_Int32                           m_nForward      ;               // current position in container for forward orientation
        sal_Int32                           m_nBackward     ;               // current position in container for backward orientation
        EEndState                           m_eEndState     ;               // position state of iterator!
        sal_Bool                            m_bReverse      ;               // orientation of stepping!
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
