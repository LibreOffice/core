/*************************************************************************
 *
 *  $RCSfile: docholder.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mav $ $Date: 2003-04-02 15:44:18 $
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

#ifndef _DOCHOLDER_HXX_
#define _DOCHOLDER_HXX_

#include "common.h"

#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class EmbedDocument_Impl;

class DocumentHolder :
    public ::cppu::WeakImplHelper3<
                        ::com::sun::star::util::XCloseListener,
                          ::com::sun::star::frame::XTerminateListener,
                        ::com::sun::star::util::XModifyListener >
{
private:

    EmbedDocument_Impl* m_pOLEInterface;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    ::rtl::OUString m_aDocumentNamePart,m_aFilterName;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > DocumentFrame();

public:

    DocumentHolder( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,EmbedDocument_Impl *pOLEInterface);
    ~DocumentHolder();

    void SetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xDoc);

    void CloseDocument();
    void CloseFrame();
    void FreeOffice();

    void resizeWin( const SIZEL& rNewSize );

    void setTitle(const rtl::OUString& aDocumentName);

    void show();

    void hide();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetDocument() { return m_xDocument; }

// XEventListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aSource );

// XCloseListener
    virtual void SAL_CALL queryClosing( const com::sun::star::lang::EventObject& aSource, sal_Bool bGetsOwnership )
        throw( ::com::sun::star::util::CloseVetoException );

    virtual void SAL_CALL notifyClosing( const com::sun::star::lang::EventObject& aSource );

// XTerminateListener
    virtual void SAL_CALL queryTermination( const com::sun::star::lang::EventObject& aSource )
        throw( ::com::sun::star::frame::TerminationVetoException );

    virtual void SAL_CALL notifyTermination( const com::sun::star::lang::EventObject& aSource );

// XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

