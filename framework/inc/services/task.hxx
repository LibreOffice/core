/*************************************************************************
 *
 *  $RCSfile: task.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:10 $
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

#ifndef __FRAMEWORK_SERVICES_TASK_HXX_
#define __FRAMEWORK_SERVICES_TASK_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#include <tools/link.hxx>
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

/*-************************************************************************************************************//**
    @short      implements an special frame - a task frame

    @descr      -

    @implements XTask

    @base       Frame
                OPropertySet
*//*-*************************************************************************************************************/

class Task  :   public css::frame::XTask            ,   // => XFrame => XComponent
                public Frame                        ,   // Order of baseclasses is neccessary for right initialization!
                public ::cppu::OBroadcastHelper     ,
                public ::cppu::OPropertySetHelper
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor to create instance

            @descr      This constructor initialize a new instance of this class,
                        and will be set valid values on his member and baseclasses.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         Task( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor

            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~Task();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XTask
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL close() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL tileWindows() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsVertical() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsHorizontal() throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XComponent
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      We must overwrite this method, because baseclass Frame implements XFrame and XComponent.
                        XTask is derived from these classes to! The compiler don't know, which base is the right one.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException )
        {
            Frame::dispose();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::addEventListener( xListener );
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::removeEventListener( xListener );
        }

        //---------------------------------------------------------------------------------------------------------
        //   XFrame
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      We must overwrite this method, because baseclass Frame implements XFrame and XComponent.
                        XTask is derived from these classes to! The compiler don't know, which base is right.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException )
        {
            Frame::initialize( xWindow );
        }

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getContainerWindow() throw( css::uno::RuntimeException )
        {
            return Frame::getContainerWindow();
        }

        virtual void SAL_CALL setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
        {
            Frame::setCreator( xCreator );
        }

        virtual css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL getCreator() throw( css::uno::RuntimeException )
        {
            return Frame::getCreator();
        }

        virtual ::rtl::OUString SAL_CALL getName() throw( css::uno::RuntimeException )
        {
            return Frame::getName();
        }

        virtual void SAL_CALL setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException )
        {
            Frame::setName( sName );
        }

        virtual sal_Bool SAL_CALL isTop() throw( css::uno::RuntimeException )
        {
            return Frame::isTop();
        }

        virtual void SAL_CALL activate() throw( css::uno::RuntimeException )
        {
            Frame::activate();
        }

        virtual void SAL_CALL deactivate() throw( css::uno::RuntimeException )
        {
            Frame::deactivate();
        }

        virtual sal_Bool SAL_CALL isActive() throw( css::uno::RuntimeException )
        {
            return Frame::isActive();
        }

        virtual sal_Bool SAL_CALL setComponent( const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow    ,
                                                const   css::uno::Reference< css::frame::XController >& xController         ) throw( css::uno::RuntimeException )
        {
            return Frame::setComponent( xComponentWindow, xController );
        }

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() throw( css::uno::RuntimeException )
        {
            return Frame::getComponentWindow();
        }

        virtual css::uno::Reference< css::frame::XController > SAL_CALL getController() throw( css::uno::RuntimeException )
        {
            return Frame::getController();
        }

        virtual void SAL_CALL contextChanged() throw( css::uno::RuntimeException )
        {
            Frame::contextChanged();
        }

        virtual void SAL_CALL addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::addFrameActionListener( xListener );
        }

        virtual void SAL_CALL removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
        {
            Frame::removeFrameActionListener( xListener );
        }

        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL findFrame(   const   ::rtl::OUString&    sTargetFrameName    ,
                                                                                           sal_Int32            nSearchFlags        ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XTopWindowListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //   XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      This object is forced to release all references to the interfaces given
                        by the parameter Source.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        //---------------------------------------------------------------------------
        //  OPropertySetHelper
        //---------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      try to convert a property value
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        You must try to convert the value of given propertyhandle and
                        return results of this operation. This will be use to ask vetoable
                        listener. If no listener have a veto, we will change value realy!
                        ( in method setFastPropertyValue_NoBroadcast(...) )

            @seealso    OPropertySetHelper
            @seealso    setFastPropertyValue_NoBroadcast()

            @param      "aConvertedValue"   new converted value of property
            @param      "aOldValue"         old value of property
            @param      "nHandle"           handle of property
            @param      "aValue"            new value of property

            @return     sal_True if value will be changed, sal_FALSE otherway

            @onerror    IllegalArgumentException, if you call this with an invalid argument
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL convertFastPropertyValue(         css::uno::Any&      aConvertedValue ,
                                                                      css::uno::Any&        aOldValue       ,
                                                                    sal_Int32           nHandle         ,
                                                            const   css::uno::Any&      aValue          ) throw( css::lang::IllegalArgumentException );

        /*-****************************************************************************************************//**
            @short      set value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        Handle and value are valid everyway! You must set the new value only.
                        After this, baseclass send messages to all listener automaticly.

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    new value of property

            @return     -

            @onerror    An exception is thrown.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(         sal_Int32       nHandle ,
                                                                  const css::uno::Any&  aValue  ) throw( css::uno::Exception );

        /*-****************************************************************************************************//**
            @short      get value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    current value of property

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL getFastPropertyValue( css::uno::Any&  aValue  ,
                                                      sal_Int32     nHandle ) const;

        /*-****************************************************************************************************//**
            @short      return structure and information about transient properties
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      -

            @return     structure with property-informations

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        /*-****************************************************************************************************//**
            @short      return propertysetinfo
            @descr      You can call this method to get information about transient properties
                        of this object.

            @seealso    OPropertySetHelper
            @seealso    XPropertySet
            @seealso    XMultiPropertySet

            @param      -

            @return     reference to object with information [XPropertySetInfo]

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        DECL_LINK( Close_Impl, void* );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_tryToChangeProperty(          sal_Bool            bProperty       ,
                                            const   css::uno::Any&      aValue          ,
                                                    css::uno::Any&      aOldValue       ,
                                                    css::uno::Any&      aConvertedValue ) throw( css::lang::IllegalArgumentException );
        sal_Bool impl_tryToChangeProperty(  const   ::rtl::OUString&    sProperty       ,
                                            const   css::uno::Any&      aValue          ,
                                                    css::uno::Any&      aOldValue       ,
                                                    css::uno::Any&      aConvertedValue ) throw( css::lang::IllegalArgumentException );
        sal_Bool impl_tryToChangeProperty(  const   css::awt::Point&    aProperty       ,
                                            const   css::uno::Any&      aValue          ,
                                                    css::uno::Any&      aOldValue       ,
                                                    css::uno::Any&      aConvertedValue ) throw( css::lang::IllegalArgumentException );
        sal_Bool impl_tryToChangeProperty(  const   css::awt::Size&     aProperty       ,
                                            const   css::uno::Any&      aValue          ,
                                                    css::uno::Any&      aOldValue       ,
                                                    css::uno::Any&      aConvertedValue ) throw( css::lang::IllegalArgumentException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        // Not used in the moment!

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    protected:

        // But some values are neede by derived classes!
        sal_Bool            m_bIsPlugIn         ;   /// In objects of these class this member is set to FALSE.
                                                    /// But in derived class PlugInFrame it's overwrited with TRUE!

    private:

        // Properties
        sal_Bool            m_bIsAlwaysVisible  ;
        sal_Bool            m_bIsFloating       ;
        css::awt::Point     m_aPosition         ;
        css::awt::Size      m_aSize             ;
        ::rtl::OUString     m_sTitle            ;
        ::vcl::EventPoster  m_aPoster           ;

};      //  class Tasks

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_TASKS_HXX_
