/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docholder.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 18:20:43 $
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

#ifndef _DOCHOLDER_HXX_
#define _DOCHOLDER_HXX_

#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XBORDERRESIZELISTENER_HPP_
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XBORDERWIDTHS_HPP_
#include <com/sun/star/frame/BorderWidths.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XHATCHWINDOWCONTROLLER_HPP_
#include <com/sun/star/embed/XHatchWindowController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

class OCommonEmbeddedObject;
class Interceptor;

class DocumentHolder :
    public ::cppu::WeakImplHelper6<
                        ::com::sun::star::util::XCloseListener,
                          ::com::sun::star::frame::XTerminateListener,
                        ::com::sun::star::util::XModifyListener,
                        ::com::sun::star::document::XEventListener,
                        ::com::sun::star::frame::XBorderResizeListener,
                        ::com::sun::star::embed::XHatchWindowController >
{
private:

    OCommonEmbeddedObject* m_pEmbedObj;

    Interceptor*        m_pInterceptor;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor > m_xOutplaceInterceptor;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > m_xComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xOwnWindow; // set for inplace objects
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xHatchWindow; // set for inplace objects

    ::com::sun::star::awt::Rectangle m_aObjRect;
    ::com::sun::star::frame::BorderWidths m_aBorderWidths;

    ::rtl::OUString m_aContainerName;
    ::rtl::OUString m_aDocumentNamePart;

    sal_Bool m_bReadOnly;

    sal_Bool m_bWaitForClose;
    sal_Bool m_bAllowClosing;
    sal_Bool m_bDesktopTerminated;

    sal_Int32 m_nNoBorderResizeReact;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > m_xCachedDocAreaAcc;


    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetDocFrame();
    sal_Bool LoadDocToFrame( sal_Bool );

    ::com::sun::star::awt::Rectangle CalculateBorderedArea( const ::com::sun::star::awt::Rectangle& aRect );
    ::com::sun::star::awt::Rectangle AddBorderToArea( const ::com::sun::star::awt::Rectangle& aRect );

    void ResizeWindows_Impl( const ::com::sun::star::awt::Rectangle& aHatchRect );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > RetrieveOwnMenu_Impl();
    sal_Bool MergeMenues_Impl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xOwnLM,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContLM,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
                const ::rtl::OUString& aContModuleName );

public:

    static void FindConnectPoints(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMenu,
        sal_Int32 nConnectPoints[2] )
            throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > MergeMenuesForInplace(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xContMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
        const ::rtl::OUString& aContModuleName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xOwnMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xOwnDisp )
            throw ( ::com::sun::star::uno::Exception );


    DocumentHolder( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                    OCommonEmbeddedObject* pEmbObj );
    ~DocumentHolder();

    OCommonEmbeddedObject* GetEmbedObject() { return m_pEmbedObj; }

    void SetComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >& xDoc, sal_Bool bReadOnly );
    void ResizeHatchWindow();
    void LockOffice();
    void FreeOffice();

    void CloseDocument( sal_Bool bDeliverOwnership, sal_Bool bWaitForClose );
    void CloseFrame();

    void SetTitle(const rtl::OUString& aDocumentName);

    rtl::OUString GetTitle() const
    {
        return m_aContainerName + ::rtl::OUString::createFromAscii( " - " ) + m_aDocumentNamePart;
    }

    void SetContainerName(const rtl::OUString& aContainerName);
    rtl::OUString GetContainerName() const { return m_aContainerName; }

    void PlaceFrame( const ::com::sun::star::awt::Rectangle& aNewRect );

    sal_Bool SetFrameLMVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                    sal_Bool bVisible );

    sal_Bool ShowInplace( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
                      const ::com::sun::star::awt::Rectangle& aRectangleToShow,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP );

    sal_Bool ShowUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP,
        const ::rtl::OUString& aContModuleName );
    sal_Bool HideUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM );

    void Show();

    void Hide();

    // sal_Bool SetVisArea( sal_Int64 nAspect, const ::com::sun::star::awt::Rectangle& aRect );
    // sal_Bool GetVisArea( sal_Int64 nAspect, ::com::sun::star::awt::Rectangle *pRect );
    sal_Bool SetExtent( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize );
    sal_Bool GetExtent( sal_Int64 nAspect, ::com::sun::star::awt::Size *pSize );

    sal_Int32 GetMapUnit( sal_Int64 nAspect );

    void SetOutplaceDispatchInterceptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >&
                                                                                            xOutplaceInterceptor )
    {
        m_xOutplaceInterceptor = xOutplaceInterceptor;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > GetComponent() { return m_xComponent; }

// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XCloseListener
    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

// XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

// XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw ( ::com::sun::star::uno::RuntimeException );

// XBorderResizeListener
    virtual void SAL_CALL borderWidthsChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& aObject, const ::com::sun::star::frame::BorderWidths& aNewSize ) throw (::com::sun::star::uno::RuntimeException);

// XHatchWindowController
    virtual void SAL_CALL requestPositioning( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calcAdjustedRectangle( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL activated(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deactivated(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

