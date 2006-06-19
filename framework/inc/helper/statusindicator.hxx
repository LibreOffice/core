/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusindicator.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:54:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_

//_______________________________________________
// include files of own module

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#include <helper/statusindicatorfactory.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_______________________________________________
// include UNO interfaces

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

//_______________________________________________
// include all others

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

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
