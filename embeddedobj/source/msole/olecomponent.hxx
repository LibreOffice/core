/*************************************************************************
 *
 *  $RCSfile: olecomponent.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:19:08 $
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

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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

#ifndef _COM_SUN_STAR_EMBED_VERBDESCRIPTOR_HPP_
#include <com/sun/star/embed/VerbDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#include <vector>


namespace com { namespace sun { namespace star {
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class OleWrapperClientSite;
class OleWrapperAdviseSink;
class OleEmbeddedObject;
struct OleComponentNative_Impl;

class OleComponent : public ::cppu::WeakImplHelper5< ::com::sun::star::util::XCloseable, ::com::sun::star::lang::XComponent,
                                                     ::com::sun::star::lang::XUnoTunnel, ::com::sun::star::util::XModifiable,
                                                     ::com::sun::star::datatransfer::XTransferable >
{
    ::osl::Mutex m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bDisposed;
    sal_Bool m_bModified;
    OleComponentNative_Impl* m_pNativeImpl;

    OleEmbeddedObject* m_pUnoOleObject;
    OleWrapperClientSite* m_pOleWrapClientSite;
    OleWrapperAdviseSink* m_pImplAdviseSink;

    ::rtl::OUString m_aTempURL;

    sal_Int32 m_nOLEMiscFlags;
    sal_Int32 m_nAdvConn;

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > m_aVerbList;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > m_aDataFlavors;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    sal_Bool m_bOleInitialized;

    sal_Bool InitializeObject_Impl();

    void CreateIStorageOnXInputStream_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );
    void CreateNewIStorage_Impl();

    void RetrieveObjectDataFlavors_Impl();

    void Dispose();

public:
    OleComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& m_xFactory,
                  OleEmbeddedObject* pOleObj );

    virtual ~OleComponent();

    OleComponent* createEmbeddedCopyOfLink();

    void disconnectEmbeddedObject();

    // ==== Initialization ==================================================
    void LoadEmbeddedObject( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );

    void CreateObjectFromClipboard();

    void CreateNewEmbeddedObject( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeqCLSID );

    void CreateObjectFromData(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransfer );

    void CreateObjectFromFile( const ::rtl::OUString& aFileName );

    void CreateLinkFromFile( const ::rtl::OUString& aFileName );

    void InitEmbeddedCopyOfLink( OleComponent* pOleLinkComponent );

    // ======================================================================

    void RunObject(); // switch OLE object to running state

    void CloseObject(); // switch OLE object to loaded state

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > GetVerbList();

    void ExecuteVerb( sal_Int32 nVerbID );

    void SetHostName( const ::rtl::OUString& aContName, const ::rtl::OUString& aEmbDocName );

    void SetExtent( const ::com::sun::star::awt::Size& aVisAreaSize, sal_Int64 nAspect );

    ::com::sun::star::awt::Size GetExtent( sal_Int64 nAspect );
    ::com::sun::star::awt::Size GetReccomendedExtent( sal_Int64 nAspect );

    sal_Int64 GetMiscStatus( sal_Int64 nAspect );

    ::com::sun::star::uno::Sequence< sal_Int8 > GetCLSID();

    void StoreObjectToStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream );

    sal_Bool SaveObject_Impl();
    sal_Bool OnShowWindow_Impl( sal_Bool bShow );
    void OnViewChange_Impl( sal_uInt32 dwAspect );

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

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference < com::sun::star::lang::XEventListener >& aListener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference < com::sun::star::lang::XEventListener >& aListener) throw (::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException) ;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
};

#endif

