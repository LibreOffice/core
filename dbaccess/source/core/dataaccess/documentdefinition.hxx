/*************************************************************************
 *
 *  $RCSfile: documentdefinition.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:10:49 $
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

#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#define _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _DBA_REGHELPER_HXX_
#include "dba_reghelper.hxx"
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include <comphelper/propertystatecontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDCLIENT_HPP_
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTATECHANGELISTENER_HPP_
#include <com/sun/star/embed/XStateChangeListener.hpp>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    typedef ::cppu::ImplHelper1<        ::com::sun::star::embed::XEmbeddedClient
                                >   ODocumentDefinition_Base;

    class OInterceptor;
    class OEmbeddedClientHelper;
//==========================================================================
//= ODocumentDefinition - a database "document" which is simply a link to a real
//=                   document
//==========================================================================

class ODocumentDefinition
        :public OContentHelper
        ,public ::comphelper::OPropertyStateContainer
        ,public ::comphelper::OPropertyArrayUsageHelper< ODocumentDefinition >
        ,public ODocumentDefinition_Base
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject>         m_xEmbeddedObject;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >   m_xListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XComponentLoader >       m_xFrameLoader;
    OInterceptor*                                                                       m_pInterceptor;
    sal_Bool                                                                            m_bForm; // <TRUE/> if it is a form
    OEmbeddedClientHelper*                                                              m_pClientHelper;

private:
    // Command "insert"
    void insert( const ::rtl::OUString& _sURL, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw( ::com::sun::star::uno::Exception );


    /** loads the EmbeddedObject if not already loaded
        @param  _aClassID
            If set, it will be used to create the embedded object.
    */
    void loadEmbeddedObject(const ::com::sun::star::uno::Sequence< sal_Int8 >& _aClassID = ::com::sun::star::uno::Sequence< sal_Int8 >()
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>()
                            ,sal_Bool _bReadOnly = sal_False);


    void generateNewImage(::com::sun::star::uno::Any& _rImage);
    void fillDocumentInfo(::com::sun::star::uno::Any& _rInfo);
    /** searches for read-only flag in the args of the model and sets it to the given value,
        if the value was not found, it will be appended.
        @param  _bReadOnly
            If <TRUE/> the document will be switched to readonly mode
    */
    void setModelReadOnly(sal_Bool _bReadOnly);
protected:
    virtual ~ODocumentDefinition();

public:

    ODocumentDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
            ,const TContentPtr& _pImpl
            ,sal_Bool _bForm
            ,const ::com::sun::star::uno::Sequence< sal_Int8 >& _aClassID = ::com::sun::star::uno::Sequence< sal_Int8 >()
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>()
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO_STATIC();

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // XComponentSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL visibilityChanged( ::sal_Bool bVisible ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XCommandProcessor
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException) ;

    // XEmbeddedClient
    virtual void SAL_CALL saveObject(  ) throw (::com::sun::star::embed::ObjectSaveVetoException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL onShowWindow( sal_Bool bVisible ) throw (::com::sun::star::uno::RuntimeException);

    // XRename
    virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage() const;

    sal_Bool save(sal_Bool _bApprove);
    void closeObject();
    sal_Bool isModified();
    void fillReportData(sal_Bool _bFill = sal_True);
protected:
    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;
    // helper
    virtual void SAL_CALL disposing();

private:
    void registerProperties();

};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_

