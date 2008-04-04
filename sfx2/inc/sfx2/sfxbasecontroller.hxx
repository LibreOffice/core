/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxbasecontroller.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:18:04 $
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

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#define _SFX_SFXBASECONTROLLER_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHINFORMATIONPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLERBORDER_HPP_
#include <com/sun/star/frame/XControllerBorder.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTITLE_HPP_
#include <com/sun/star/frame/XTitle.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTITLECHANGEBROADCASTER_HPP_
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XCONTEXTMENUINTERCEPTION_HPP_
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XCONTEXTMENUINTERCEPTOR_HPP_
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XMOUSECLICKHANDLER_HPP_
#include <com/sun/star/awt/XMouseClickHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XKEYHANDLER_HPP_
#include <com/sun/star/awt/XKeyHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XUSERINPUTINTERCEPTION_HPP_
#include <com/sun/star/awt/XUserInputInterception.hpp>
#endif

//________________________________________________________________________________________________________
//  include of my own project
//________________________________________________________________________________________________________

#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif

#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

//________________________________________________________________________________________________________
//  defines
//________________________________________________________________________________________________________

// Some defines to write better code :-)
#define REFERENCE               ::com::sun::star::uno::Reference
#define ANY                     ::com::sun::star::uno::Any
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define XDISPATCH               ::com::sun::star::frame::XDispatch
#define XDISPATCHPROVIDER       ::com::sun::star::frame::XDispatchProvider
#define DISPATCHDESCRIPTOR      ::com::sun::star::frame::DispatchDescriptor
#define XMODEL                  ::com::sun::star::frame::XModel
#define XFRAME                  ::com::sun::star::frame::XFrame
#define XCONTROLLER             ::com::sun::star::frame::XController
#define XCONTROLLERBORDER       ::com::sun::star::frame::XControllerBorder
#define XEVENTLISTENER          ::com::sun::star::lang::XEventListener
#define MUTEX                   ::osl::Mutex
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define UNOTYPE                 ::com::sun::star::uno::Type
#define OWEAKOBJECT             ::cppu::OWeakObject
#define XTYPEPROVIDER           ::com::sun::star::lang::XTypeProvider
#define UNOURL                  ::com::sun::star::util::URL
#define OUSTRING                ::rtl::OUString
#define XSTATUSINDICATORSUPPLIER ::com::sun::star::task::XStatusIndicatorSupplier
#define XCONTEXTMENUINTERCEPTION ::com::sun::star::ui::XContextMenuInterception
#define XCONTEXTMENUINTERCEPTOR ::com::sun::star::ui::XContextMenuInterceptor
#define XUSERINPUTINTERCEPTION  ::com::sun::star::awt::XUserInputInterception
#define XDISPATCHINFORMATIONPROVIDER ::com::sun::star::frame::XDispatchInformationProvider
#define XTITLE                  ::com::sun::star::frame::XTitle
#define XTITLECHANGEBROADCASTER ::com::sun::star::frame::XTitleChangeBroadcaster

//________________________________________________________________________________________________________
//  forwards
//________________________________________________________________________________________________________

struct  IMPL_SfxBaseController_DataContainer    ;   // impl. struct to hold member of class SfxBaseController

sal_Int16 MapGroupIDToCommandGroup( sal_Int16 nGroupID );
sal_Bool SupportsCommandGroup( sal_Int16 nCommandGroup );
sal_Int16 MapCommandGroupToGroupID( sal_Int16 nCommandGroup );

//________________________________________________________________________________________________________
//  class declarations
//________________________________________________________________________________________________________

struct IMPL_SfxBaseController_MutexContainer
{
    MUTEX m_aMutex ;
} ;

/**_______________________________________________________________________________________________________
    @short      -

    @descr      -

    @implements -

    @base       -
*/

// Forward to impl-baseclass!
//class IMPL_SfxBaseController ;

class SFX2_DLLPUBLIC SfxBaseController  :   public XTYPEPROVIDER
                        ,   public XCONTROLLER
                        ,   public XCONTROLLERBORDER
                        ,   public XDISPATCHPROVIDER
                        ,   public XSTATUSINDICATORSUPPLIER
                        ,   public XCONTEXTMENUINTERCEPTION
                        ,   public XUSERINPUTINTERCEPTION
                        ,   public XDISPATCHINFORMATIONPROVIDER
                        ,   public XTITLE
                        ,   public XTITLECHANGEBROADCASTER
                        ,   public IMPL_SfxBaseController_MutexContainer
                        ,   public OWEAKOBJECT
{
//________________________________________________________________________________________________________
//  public methods
//________________________________________________________________________________________________________

public:

    //____________________________________________________________________________________________________
    //  constructor/destructor
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    SfxBaseController( SfxViewShell* pView ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    ~SfxBaseController() ;

    SAL_DLLPRIVATE void ReleaseShell_Impl();
    SAL_DLLPRIVATE void BorderWidthsChanged_Impl();

    //____________________________________________________________________________________________________
    //  XInterface
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual ANY SAL_CALL queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() ;

    /**___________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() ;

    //____________________________________________________________________________________________________
    //  XTypeProvider
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      get information about supported interfaces
        @descr      -

        @seealso    XTypeProvider

        @param      -

        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual SEQUENCE< UNOTYPE > SAL_CALL getTypes() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider

        @param      -

        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual SEQUENCE< sal_Int8 > SAL_CALL getImplementationId() throw( RUNTIMEEXCEPTION ) ;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL getStatusIndicator(  ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XController
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL attachFrame( const REFERENCE< XFRAME >& xFrame ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL attachModel( const REFERENCE< XMODEL >& xModel ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    ANY SAL_CALL getViewData() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void SAL_CALL restoreViewData( const ANY& aValue ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    REFERENCE< XFRAME > SAL_CALL getFrame() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    REFERENCE< XMODEL > SAL_CALL getModel() throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //  XDispatchProvider
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL &            aURL            ,
                                                            const   OUSTRING &          sTargetFrameName,
                                                                    FrameSearchFlags    eSearchFlags    ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescriptor ) throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //  XControllerBorder
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::frame::BorderWidths SAL_CALL getBorder() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addBorderResizeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XBorderResizeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeBorderResizeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XBorderResizeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL queryBorderedArea( const ::com::sun::star::awt::Rectangle& aPreliminaryRectangle ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION ) ;
    virtual void SAL_CALL registerContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );
    virtual void SAL_CALL releaseContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

    virtual void SAL_CALL addKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XDispatchInformationProvider
    //____________________________________________________________________________________________________
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) throw (::com::sun::star::uno::RuntimeException);

    // css::frame::XTitle
    virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw (::com::sun::star::uno::RuntimeException);

    // css::frame::XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener )     throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE SfxViewShell* GetViewShell_Impl() const;
    SAL_DLLPRIVATE BOOL HandleEvent_Impl( NotifyEvent& rEvent );
    SAL_DLLPRIVATE BOOL HasKeyListeners_Impl();
    SAL_DLLPRIVATE BOOL HasMouseClickListeners_Impl();
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle > impl_getTitleHelper ();
//#endif

//________________________________________________________________________________________________________
//  private variables
//________________________________________________________________________________________________________

    /** With this method you can set the flag that controlls whether the
        frame is released together with a controller when the later one is
        disposed.
        @param bFlag
            When passing <true/>, the default value of this flag, then
            disposing the controller results in releasing the frame.
            Passing <false/> leaves the frame unaffected.
    */
    void FrameIsReleasedWithController (sal_Bool bFlag);

private:

    IMPL_SfxBaseController_DataContainer*   m_pData ;

} ; // class SfxBaseController

#endif  // _SFX_SFXBASECONTROLLER_HXX
