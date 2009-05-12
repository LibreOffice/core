/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: statusindicator.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_

//_______________________________________________
// include files of own module

#include <helper/statusindicatorfactory.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>

//_______________________________________________
// include UNO interfaces
#include <com/sun/star/task/XStatusIndicator.hpp>

//_______________________________________________
// include all others
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// definitions

//_______________________________________________
/**
    @short          implement a status indicator object

    @descr          With this indicator you can show a message and a progress ...
                    but you share the output device with other indicator objects,
                    if this instances was created by the same factory.
                    Then the last created object has full access to device.
                    All others change her internal data structure only.

                    All objects of this StatusIndicator class calls a c++ interface
                    on the StatusIndicatorFactory (where they was created).
                    The factory holds all data structures and paints the progress.

    @devstatus      ready to use
    @threadsafe     yes
*/
class StatusIndicator : public  css::lang::XTypeProvider
                      , public  css::task::XStatusIndicator
                      , private ThreadHelpBase                  // Order of baseclasses is neccessary for right initializaton!
                      , public  ::cppu::OWeakObject             // => XInterface
{
    //-------------------------------------------
    // member
    private:

        /** @short  weak reference to our factory
            @descr  All our interface calls will be forwarded
                    to a suitable c++ interface on this factory.
                    But we dont hold our factory alive. They
                    correspond with e.g. with a Frame service and
                    will be owned by him. If the frame will be closed
                    he close our factory too ...
         */
        css::uno::WeakReference< css::task::XStatusIndicatorFactory > m_xFactory;

    //-------------------------------------------
    // c++ interface
    public:

        //----------------------------------------
        /** @short  initialize new instance of this class.

            @param  pFactory
                    pointer to our factory
         */
        StatusIndicator(StatusIndicatorFactory* pFactory);

        //----------------------------------------
        /** @short  does nothing real ....
         */
        virtual ~StatusIndicator();

    //-------------------------------------------
    // uno interface
    public:

        //---------------------------------------
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //---------------------------------------
        // XStatusIndicator
        virtual void SAL_CALL start(const ::rtl::OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setText(const ::rtl::OUString& sText)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setValue(sal_Int32 nValue)
            throw(css::uno::RuntimeException);

}; // class StatusIndicator

} // namespace framework

#endif // __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
