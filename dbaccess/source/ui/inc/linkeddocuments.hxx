/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#define _DBAUI_LINKEDDOCUMENTS_HXX_

#include "AppElementType.hxx"

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#include <com/sun/star/frame/XFrame.hpp>
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
#endif

class Window;
//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    //= OLinkedDocumentsAccess
    //==================================================================
    class OLinkedDocumentsAccess
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xDocumentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                    m_xDocumentUI;
        Window*     m_pDialogParent;
        String      m_sCurrentlyEditing;
        ::rtl::OUString
                    m_sDataSourceName;

    public:
        OLinkedDocumentsAccess(
            Window* _pDialogParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& i_rDocumentUI,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
            const ::rtl::OUString& _sDataSourceName
        );
        ~OLinkedDocumentsAccess();

        inline sal_Bool isConnected() const { return m_xConnection.is(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
                open(
                    const ::rtl::OUString& _rLinkName,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition,
                    ElementOpenMode _eOpenMode,
                    const ::comphelper::NamedValueCollection& _rAdditionalArgs
                );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                newDocument(
                          sal_Int32 i_nActionID,
                    const ::comphelper::NamedValueCollection& i_rCreationArgs,
                          ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_rDefinition
                );

        void    newFormWithPilot(
                    const sal_Int32 _nCommandType = -1,
                    const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                );
        void    newReportWithPilot(
                    const sal_Int32 _nCommandType = -1,
                    const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                );
        void    newQueryWithPilot();
        void    newTableWithPilot();

        enum RESULT
        {
            ERROR,
            SUCCESS,
            CANCEL
        };
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            impl_open(
                const ::rtl::OUString& _rLinkName,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDefinition,
                ElementOpenMode _eOpenMode,
                const ::comphelper::NamedValueCollection& _rAdditionalArgs
            );

        void
            impl_newWithPilot(
                const char* _pWizardService,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rObjectName
            );

    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_LINKEDDOCUMENTS_HXX_

