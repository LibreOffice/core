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



#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#define __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <stdexcept>
#include <algorithm>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif
#include <macros/debug.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/Reference.hxx>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weakref.hxx>
#include <vos/ref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/evntpost.hxx>

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

typedef ::std::vector< css::uno::Reference< css::frame::XFrame > >  TFrameContainer     ;
typedef TFrameContainer::iterator                                   TFrameIterator      ;
typedef TFrameContainer::const_iterator                             TConstFrameIterator ;

/*-************************************************************************************************************//**
    @short          implement a container to hold childs of frame, task or desktop
    @descr          Every object of frame, task or desktop hold reference to his childs. These container is used as helper
                    to do this. Some helper-classe like OFrames or OTasksAccess use it to. They hold a pointer to an instance
                    of this class, which is a member of a frame, task or desktop! You can append and remove frames.
                    It's possible to set one of these frames as active or deactive. You could have full index-access to
                    container-items.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during boostrap!

    @devstatus      ready to use
    @threadsafe     yes
    @modified       01.07.2002 14:39, as96863
*//*-*************************************************************************************************************/
class FrameContainer : private ThreadHelpBase
{
    //_______________________________________
    // member

    private:

        /// list to hold all frames
        TFrameContainer m_aContainer;
        /// one container item can be the current active frame. Its necessary for Desktop or Frame implementation.
        css::uno::Reference< css::frame::XFrame > m_xActiveFrame;
/*DEPRECATEME
        /// indicates using of the automatic async quit feature in case last task will be closed
        sal_Bool m_bAsyncQuit;
        /// used to execute the terminate request asyncronous
        ::vcl::EventPoster m_aAsyncCall;
        /// used for async quit feature (must be weak to prevent us against strange situations!)
        css::uno::WeakReference< css::frame::XDesktop > m_xDesktop;
*/

    //_______________________________________
    // interface

    public:

        /// constructor / destructor
                 FrameContainer();
        virtual ~FrameContainer();

        /// add/remove/mark container items
        void                                      append     ( const css::uno::Reference< css::frame::XFrame >& xFrame );
        void                                      remove     ( const css::uno::Reference< css::frame::XFrame >& xFrame );
        void                                      setActive  ( const css::uno::Reference< css::frame::XFrame >& xFrame );
        css::uno::Reference< css::frame::XFrame > getActive  (                                                         ) const;

        /// checks and free memory
        sal_Bool exist      ( const css::uno::Reference< css::frame::XFrame >& xFrame ) const;
        void     clear      (                                                         );

        /// deprecated IndexAccess!
        sal_uInt32                                getCount  (                   ) const;
        css::uno::Reference< css::frame::XFrame > operator[]( sal_uInt32 nIndex ) const;

        /// replacement for deprectaed index access
        css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > getAllElements() const;

        /// special helper for Frame::findFrame()
        css::uno::Reference< css::frame::XFrame > searchOnAllChildrens   ( const ::rtl::OUString& sName ) const;
        css::uno::Reference< css::frame::XFrame > searchOnDirectChildrens( const ::rtl::OUString& sName ) const;

}; // class FrameContainer

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
