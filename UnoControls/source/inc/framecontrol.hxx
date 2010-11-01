/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX
#define _UNOCONTROLS_FRAMECONTROL_CTRL_HXX

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/propshlp.hxx>

//______________________________________________________________________________________________________________
//  includes of my own project
//______________________________________________________________________________________________________________
#include "basecontrol.hxx"
#include "OConnectionPointContainerHelper.hxx"

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO     ::com::sun::star::uno
#define CSS_LANG    ::com::sun::star::lang
#define CSS_BEANS   ::com::sun::star::beans
#define CSS_AWT     ::com::sun::star::awt
#define CSS_FRAME   ::com::sun::star::frame

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

#define SERVICENAME_FRAMECONTROL                        "com.sun.star.frame.FrameControl"
#define IMPLEMENTATIONNAME_FRAMECONTROL                 "stardiv.UnoControls.FrameControl"
#define PROPERTYNAME_LOADERARGUMENTS                    "LoaderArguments"
#define PROPERTYNAME_COMPONENTURL                       "ComponentURL"
#define PROPERTYNAME_FRAME                              "Frame"
#define ERRORTEXT_VOSENSHURE                            "This is an invalid property handle."
#define PROPERTY_COUNT                                  3                                                       // you must count the propertys
#define PROPERTYHANDLE_COMPONENTURL                     0                                                       // Id must be the index into the array
#define PROPERTYHANDLE_FRAME                            1
#define PROPERTYHANDLE_LOADERARGUMENTS                  2

//______________________________________________________________________________________________________________
//  class
//______________________________________________________________________________________________________________

class FrameControl  : public CSS_AWT::XControlModel
                    , public CSS_LANG::XConnectionPointContainer
                    , public BaseControl                                // This order is neccessary for right initialization of m_aMutex!
                    , public ::cppu::OBroadcastHelper
                    , public ::cppu::OPropertySetHelper
{

//______________________________________________________________________________________________________________
//  public methods
//______________________________________________________________________________________________________________

public:

    //__________________________________________________________________________________________________________
    //  construct/destruct
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    FrameControl( const CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory >& xFactory );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~FrameControl();

    //__________________________________________________________________________________________________________
    //  XInterface
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Any SAL_CALL queryInterface(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //__________________________________________________________________________________________________________
    //  XTypeProvider
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Type > SAL_CALL getTypes()
        throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XAggregation
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    CSS_UNO::Any SAL_CALL queryAggregation(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL createPeer(
        const CSS_UNO::Reference< CSS_AWT::XToolkit >&      xToolkit ,
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >&   xParent
    ) throw ( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL setModel(
        const CSS_UNO::Reference< CSS_AWT::XControlModel >& xModel
    ) throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Reference< CSS_AWT::XControlModel > SAL_CALL getModel()
        throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XComponent
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL dispose() throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XView
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL setGraphics(
        const CSS_UNO::Reference< CSS_AWT::XGraphics >& xDevice
    ) throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Reference< CSS_AWT::XGraphics > SAL_CALL getGraphics()
        throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XConnectionPointContainer
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Type > SAL_CALL getConnectionPointTypes()
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Reference< CSS_LANG::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const CSS_UNO::Type& aType
    ) throw ( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(
        const CSS_UNO::Type&                                aType       ,
        const CSS_UNO::Reference< CSS_UNO::XInterface >&    xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise(
        const CSS_UNO::Type&                                aType ,
        const CSS_UNO::Reference< CSS_UNO::XInterface >&    xListener
    ) throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  impl but public methods to register service!
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const CSS_UNO::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::rtl::OUString impl_getStaticImplementationName();

//______________________________________________________________________________________________________________
//  protected methods
//______________________________________________________________________________________________________________

protected:
    using OPropertySetHelper::getFastPropertyValue;
    //__________________________________________________________________________________________________________
    //  OPropertySetHelper
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        CSS_UNO::Any&       rConvertedValue   ,
        CSS_UNO::Any&       rOldValue         ,
        sal_Int32           nHandle           ,
        const CSS_UNO::Any& rValue
    ) throw( CSS_LANG::IllegalArgumentException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle ,
        const CSS_UNO::Any& rValue
    ) throw ( ::com::sun::star::uno::Exception );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL getFastPropertyValue( CSS_UNO::Any&   rValue  ,
                                                sal_Int32       nHandle ) const ;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    //__________________________________________________________________________________________________________
    //  XPropertySet
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    CSS_UNO::Reference< CSS_BEANS::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( CSS_UNO::RuntimeException );

    //__________________________________________________________________________________________________________
    //  BaseControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_AWT::WindowDescriptor* impl_getWindowDescriptor(
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >& xParentPeer
    );

//______________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________

private:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_createFrame(  const CSS_UNO::Reference< CSS_AWT::XWindowPeer >&       xPeer           ,
                            const ::rtl::OUString&                                  sURL            ,
                            const CSS_UNO::Sequence< CSS_BEANS::PropertyValue >&    seqArguments    );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_deleteFrame();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const CSS_UNO::Sequence< CSS_BEANS::Property > impl_getStaticPropertyDescriptor();


//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

private:

    CSS_UNO::Reference< CSS_FRAME::XFrame >         m_xFrame                    ;
    ::rtl::OUString                                 m_sComponentURL             ;
    CSS_UNO::Sequence< CSS_BEANS::PropertyValue >   m_seqLoaderArguments        ;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aInterfaceContainer       ;
    OConnectionPointContainerHelper                 m_aConnectionPointContainer ;

};  // class FrameControl

// The namespace alaises are only used in the header
#undef CSS_UNO
#undef CSS_LANG
#undef CSS_BEANS
#undef CSS_AWT
#undef CSS_FRAME

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_FRAMECONTROL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
