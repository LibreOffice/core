/*************************************************************************
 *
 *  $RCSfile: task.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
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

#define ILLEGALARGUMENTEXCEPTION            ::com::sun::star::lang::IllegalArgumentException
#define IPROPERTYARRAYHELPER                ::cppu::IPropertyArrayHelper
#define OBROADCASTHELPER                    ::cppu::OBroadcastHelper
#define OPROPERTYSETHELPER                  ::cppu::OPropertySetHelper
#define PROPERTY                            ::com::sun::star::beans::Property
#define UNOPOINT                            ::com::sun::star::awt::Point
#define UNOSIZE                             ::com::sun::star::awt::Size
#define XPROPERTYSETINFO                    ::com::sun::star::beans::XPropertySetInfo
#define XTASK                               ::com::sun::star::frame::XTask
#define RUNTIMEEXCEPTION                    ::com::sun::star::uno::RuntimeException
#define EXCEPTION                           ::com::sun::star::uno::Exception

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

class Task  :   public XTASK                ,   // => XFrame => XComponent
                public Frame                ,   // Order of baseclasses is neccessary for right initialization!
                public OBROADCASTHELPER     ,
                public OPROPERTYSETHELPER
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

         Task( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

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

        virtual sal_Bool SAL_CALL close() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL tileWindows() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsVertical() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL arrangeWindowsHorizontal() throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL dispose() throw( RUNTIMEEXCEPTION )
        {
            Frame::dispose();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL addEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
        {
            Frame::addEventListener( xListener );
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL removeEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
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

        virtual void SAL_CALL initialize( const REFERENCE< XWINDOW >& xWindow ) throw( RUNTIMEEXCEPTION )
        {
            Frame::initialize( xWindow );
        }

        /*-*******************************************************************************************************/
        virtual REFERENCE< XWINDOW > SAL_CALL getContainerWindow() throw( RUNTIMEEXCEPTION )
        {
            return Frame::getContainerWindow();
        }

        /*-****************************************************************************************************/
        virtual void SAL_CALL setCreator( const REFERENCE< XFRAMESSUPPLIER >& xCreator ) throw( RUNTIMEEXCEPTION )
        {
            Frame::setCreator( xCreator );
        }

        /*-****************************************************************************************************/
        virtual REFERENCE< XFRAMESSUPPLIER > SAL_CALL getCreator() throw( RUNTIMEEXCEPTION )
        {
            return Frame::getCreator();
        }

        /*-****************************************************************************************************/
        virtual OUSTRING SAL_CALL getName() throw( RUNTIMEEXCEPTION )
        {
            return Frame::getName();
        }

        /*-****************************************************************************************************/
        virtual void SAL_CALL setName( const OUSTRING& sName ) throw( RUNTIMEEXCEPTION )
        {
            Frame::setName( sName );
        }

        /*-*******************************************************************************************************/
        virtual sal_Bool SAL_CALL isTop() throw( RUNTIMEEXCEPTION )
        {
            return Frame::isTop();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL activate() throw( RUNTIMEEXCEPTION )
        {
            Frame::activate();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL deactivate() throw( RUNTIMEEXCEPTION )
        {
            Frame::deactivate();
        }

        /*-*******************************************************************************************************/
        virtual sal_Bool SAL_CALL isActive() throw( RUNTIMEEXCEPTION )
        {
            return Frame::isActive();
        }

        /*-*******************************************************************************************************/
        virtual sal_Bool SAL_CALL setComponent( const   REFERENCE< XWINDOW >&       xComponentWindow    ,
                                                const   REFERENCE< XCONTROLLER >&   xController         ) throw( RUNTIMEEXCEPTION )
        {
            return Frame::setComponent( xComponentWindow, xController );
        }

        /*-*******************************************************************************************************/
        virtual REFERENCE< XWINDOW > SAL_CALL getComponentWindow() throw( RUNTIMEEXCEPTION )
        {
            return Frame::getComponentWindow();
        }

        /*-*******************************************************************************************************/
        virtual REFERENCE< XCONTROLLER > SAL_CALL getController() throw( RUNTIMEEXCEPTION )
        {
            return Frame::getController();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL contextChanged() throw( RUNTIMEEXCEPTION )
        {
            Frame::contextChanged();
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL addFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
        {
            Frame::addFrameActionListener( xListener );
        }

        /*-*******************************************************************************************************/
        virtual void SAL_CALL removeFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
        {
            Frame::removeFrameActionListener( xListener );
        }

        /*-****************************************************************************************************//**
            @short      -

            @descr      This method searches for a frame with the specified name.
                        Frames may contain other frames (e.g. a frameset) and may
                        be contained in other frames. This hierarchie ist searched by
                        this method.
                        First some special names are taken into account, i.e. "",
                         "_self", "_top", "_active" etc. The FrameSearchFlags are ignored
                        when comparing these names with aTargetFrameName, further steps are
                        controlled by the FrameSearchFlags. If allowed, the name of the frame
                        itself is compared with the desired one, then ( again if allowed )
                        the method findFrame is called for all children of the frame.
                        At last findFrame may be called for the parent frame ( if allowed ).
                        If no Frame with the given name is found until the top frames container,
                        a new top Frame is created, if this is allowed by a special
                        FrameSearchFlag. The new Frame also gets the desired name.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL findFrame( const   OUSTRING&   sTargetFrameName    ,
                                                                sal_Int32   nSearchFlags        ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL windowClosing( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowActivated( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowDeactivated( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

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

        virtual sal_Bool SAL_CALL convertFastPropertyValue(         ANY&        aConvertedValue ,
                                                                      ANY&      aOldValue       ,
                                                                    sal_Int32   nHandle         ,
                                                            const   ANY&        aValue          ) throw( ILLEGALARGUMENTEXCEPTION );

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

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(         sal_Int32   nHandle ,
                                                                  const ANY&        aValue  ) throw( EXCEPTION );

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

        virtual void SAL_CALL getFastPropertyValue( ANY&        aValue  ,
                                                      sal_Int32 nHandle ) const;

        /*-****************************************************************************************************//**
            @short      return structure and information about transient properties
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      -

            @return     structure with property-informations

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual IPROPERTYARRAYHELPER& SAL_CALL getInfoHelper();

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

        virtual REFERENCE< XPROPERTYSETINFO > SAL_CALL getPropertySetInfo();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        DECL_LINK(      Close_Impl, void* );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_tryToChangeProperty(          sal_Bool    bProperty       ,
                                            const   ANY&        aValue          ,
                                                    ANY&        aOldValue       ,
                                                    ANY&        aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );
        sal_Bool impl_tryToChangeProperty(  const   OUSTRING&   sProperty       ,
                                            const   ANY&        aValue          ,
                                                    ANY&        aOldValue       ,
                                                    ANY&        aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );
        sal_Bool impl_tryToChangeProperty(  const   UNOPOINT&   aProperty       ,
                                            const   ANY&        aValue          ,
                                                    ANY&        aOldValue       ,
                                                    ANY&        aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );
        sal_Bool impl_tryToChangeProperty(  const   UNOSIZE&    aProperty       ,
                                            const   ANY&        aValue          ,
                                                    ANY&        aOldValue       ,
                                                    ANY&        aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        static const SEQUENCE< PROPERTY > impl_getStaticPropertyDescriptor();

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
        sal_Bool        m_bIsPlugIn         ;   /// In objects of these class this member is set to FALSE.
                                                /// But in derived class PlugInFrame it's overwrited with TRUE!

    private:

        // Properties
        sal_Bool            m_bIsAlwaysVisible  ;
        sal_Bool            m_bIsFloating       ;
        UNOPOINT            m_aPosition         ;
        UNOSIZE             m_aSize             ;
        OUSTRING            m_sTitle            ;
        ::vcl::EventPoster  m_aPoster           ;

};      //  class Tasks

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_TASKS_HXX_
