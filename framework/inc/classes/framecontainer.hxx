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

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#define __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <general.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/evntpost.hxx>

namespace framework{

typedef ::std::vector< css::uno::Reference< css::frame::XFrame > >  TFrameContainer     ;
typedef TFrameContainer::iterator                                   TFrameIterator      ;
typedef TFrameContainer::const_iterator                             TConstFrameIterator ;

/*-************************************************************************************************************//**
    @short          implement a container to hold children of frame, task or desktop
    @descr          Every object of frame, task or desktop hold reference to his children. These container is used as helper
                    to do this. Some helper-classe like OFrames or OTasksAccess use it to. They hold a pointer to an instance
                    of this class, which is a member of a frame, task or desktop! You can append and remove frames.
                    It's possible to set one of these frames as active or deactive. You could have full index-access to
                    container-items.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during boostrap!

    @devstatus      ready to use
    @threadsafe     yes
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
        css::uno::Reference< css::frame::XFrame > searchOnAllChildrens   ( const OUString& sName ) const;
        css::uno::Reference< css::frame::XFrame > searchOnDirectChildrens( const OUString& sName ) const;

}; // class FrameContainer

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
