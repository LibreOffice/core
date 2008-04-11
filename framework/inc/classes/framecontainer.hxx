/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: framecontainer.hxx,v $
 * $Revision: 1.18 $
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
#include <classes/targetfinder.hxx>
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
        /// one container item can be the current active frame. Its neccessary for Desktop or Frame implementation.
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
        sal_Bool hasElements(                                                         ) const;
        void     clear      (                                                         );

        /// deprecated IndexAccess!
        sal_uInt32                                getCount  (                   ) const;
        css::uno::Reference< css::frame::XFrame > operator[]( sal_uInt32 nIndex ) const;

        /// replacement for deprectaed index access
        css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > getAllElements() const;
        /// for special feature "async quit timer" of desktop only!
        void enableQuitTimer ( const css::uno::Reference< css::frame::XDesktop >& xDesktop );
        void disableQuitTimer();
/*DEPRECATEME
        DECL_LINK( implts_asyncQuit, void* );
*/
        /// special helper for Frame::findFrame()
        css::uno::Reference< css::frame::XFrame > searchOnAllChildrens   ( const ::rtl::OUString& sName ) const;
        css::uno::Reference< css::frame::XFrame > searchOnDirectChildrens( const ::rtl::OUString& sName ) const;

}; // class FrameContainer

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
