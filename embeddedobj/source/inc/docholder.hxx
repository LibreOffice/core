/*************************************************************************
 *
 *  $RCSfile: docholder.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-18 12:47:08 $
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
 *  Source License Version 1.1 ( the "License" ); You may not use this file
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
 *  Contributor( s ): _______________________________________
 *
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
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

class OCommonEmbeddedObject;
class Interceptor;

class DocumentHolder :
    public ::cppu::WeakImplHelper4<
                        ::com::sun::star::util::XCloseListener,
                          ::com::sun::star::frame::XTerminateListener,
                        ::com::sun::star::util::XModifyListener,
                        ::com::sun::star::document::XEventListener >
{
private:

    OCommonEmbeddedObject* m_pEmbedObj;
    Interceptor*        m_pInterceptor;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDocument;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    ::rtl::OUString m_aContainerName;
    ::rtl::OUString m_aDocumentNamePart;

    sal_Bool m_bReadOnly;

    sal_Bool m_bWaitForClose;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetDocFrame();
    void LoadDocToFrame();

public:

    DocumentHolder( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                    OCommonEmbeddedObject* pEmbObj );
    ~DocumentHolder();

    OCommonEmbeddedObject* GetEmbedObject() { return m_pEmbedObj; }

    void SetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xDoc, sal_Bool bReadOnly );

    void LockOffice();
    void FreeOffice();

    void CloseDocument( sal_Bool bDeliverOwnership, sal_Bool bWaitForClose );
    void CloseFrame();

    void SetTitle(const rtl::OUString& aDocumentName);
    rtl::OUString GetTitle() const { return m_aDocumentNamePart; }

    void SetContainerName(const rtl::OUString& aContainerName);
    rtl::OUString GetContainerName() const { return m_aContainerName; }

    void Show();

    void Hide();

    sal_Bool SetVisArea( const ::com::sun::star::awt::Rectangle& aRect );
    sal_Bool GetVisArea( ::com::sun::star::awt::Rectangle *pRect );
    sal_Bool SetExtent( const ::com::sun::star::awt::Size& aSize );
    sal_Bool GetExtent( ::com::sun::star::awt::Size *pSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetDocument() { return m_xDocument; }

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

};

#endif

