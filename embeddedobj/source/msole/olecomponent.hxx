/*************************************************************************
 *
 *  $RCSfile: olecomponent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-14 15:24:27 $
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

#ifndef _INC_OLECOMPONENT_HXX_
#define _INC_OLECOMPONENT_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_VERBDESCR_HPP_
#include <com/sun/star/embed/VerbDescr.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include <platform.h>

namespace com { namespace sun { namespace star {
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class OleWrapperClientSite;
class OleWrapperAdviseSink;
class OleEmbeddedObject;
class OleComponent : public ::cppu::WeakImplHelper2< ::com::sun::star::util::XCloseable,
                                                     ::com::sun::star::datatransfer::XTransferable >
{
    ::osl::Mutex m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bDisposed;

    CComPtr< IUnknown > m_pObj;
    CComPtr< IOleObject > m_pOleObject;
    CComPtr< IViewObject2 > m_pViewObject2;

    OleEmbeddedObject* m_pUnoOleObject;
    OleWrapperClientSite* m_pOleWrapClientSite;
    OleWrapperAdviseSink* m_pImplAdviseSink;

    ::rtl::OUString m_aTempURL;
    CComPtr< IStorage > m_pIStorage;

    sal_Int32 m_nMSAspect;
    sal_Int32 m_nOLEMiscFlags;
    sal_Int32 m_nAdvConn;

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescr > m_aVerbList;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > m_aDataFlavors;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    sal_uInt16 m_nSupportedFormat;
    sal_uInt32 m_nSupportedMedium;

    sal_Bool m_bOleInitialized;

    sal_Bool InitializeObject_Impl( sal_uInt32 nIconHandle );

    CComPtr< IStorage > CreateIStorageOnXInputStream_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );
    CComPtr< IStorage > CreateNewIStorage_Impl();

    void RetrieveObjectDataFlavors_Impl();

public:
    OleComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& m_xFactory,
                  OleEmbeddedObject* pOleObj );

    virtual ~OleComponent();

    void disconnectEmbeddedObject();

    // ==== Initialization ==================================================
    void LoadEmbeddedObject( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
                             sal_Int64 nAspect );

    void CreateNewEmbeddedObject( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeqCLSID,
                                  sal_Int64 nAspect,
                                  sal_uInt32 nIconHandle );

    void CreateObjectFromData(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransfer,
                        sal_Int64 nAspect );

    void CreateObjectFromFile( const ::rtl::OUString& aFileName, sal_Int64 nAspect, sal_uInt32 nIconHandle );

    void CreateLinkFromFile( const ::rtl::OUString& aFileName, sal_Int64 nAspect, sal_uInt32 nIconHandle );
    // ======================================================================

    void RunObject(); // switch OLE object to running state

    void CloseObject(); // switch OLE object to loaded state

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescr > GetVerbList();

    void ExecuteVerb( sal_Int32 nVerbID );

    void SetHostName( const ::rtl::OUString& aContName, const ::rtl::OUString& aEmbDocName );

    void SetExtent( const ::com::sun::star::awt::Size& aVisAreaSize, sal_Int64 nAspect );

    ::com::sun::star::awt::Size GetExtent( sal_Int64 nAspect );

    sal_Int64 GetViewAspect();

    sal_Int64 GetMiscStatus();

    void StoreObjectToStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream,
                              sal_Bool bStoreVisReplace );

    sal_Bool SaveObject_Impl();
    sal_Bool OnShowWindow_Impl( sal_Bool bShow );
    void OnViewChange_Impl( DWORD dwAspect );

    sal_Bool GetGraphicalCache_Impl( const ::com::sun::star::datatransfer::DataFlavor& aFlavor,
                                     ::com::sun::star::uno::Any& aResult );

    // XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XTransferable
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::uno::RuntimeException);

};

#endif

