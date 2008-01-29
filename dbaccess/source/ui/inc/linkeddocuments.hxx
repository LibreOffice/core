/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linkeddocuments.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:51:56 $
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

#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#define _DBAUI_LINKEDDOCUMENTS_HXX_

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
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
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
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xParentFrame;
        Window*     m_pDialogParent;
        String      m_sCurrentlyEditing;
        ::rtl::OUString
                    m_sDataSourceName;

    public:
        OLinkedDocumentsAccess(
            Window* _pDialogParent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
            ,const ::rtl::OUString& _sDataSourceName
            );
        ~OLinkedDocumentsAccess();

        enum EOpenMode
        {
            OPEN_NORMAL,
            OPEN_DESIGN,
            OPEN_FORMAIL
        };

        inline sal_Bool isConnected() const { return m_xConnection.is(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       open(const ::rtl::OUString& _rLinkName
                                                                                    ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                                                                                    , EOpenMode _eOpenMode = OPEN_NORMAL);

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newDocument(sal_Int32 _nNewFormId
                                ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                                ,const sal_Int32 _nCommandType
                                ,const ::rtl::OUString& _sObjectName);

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newWithPilot(
                            const char* _pWizardService
                            ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                            ,const sal_Int32 _nCommandType = -1
                            ,const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                        );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newFormWithPilot(
                            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                            ,const sal_Int32 _nCommandType = -1
                            ,const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                        );
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newReportWithPilot(
                            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                            ,const sal_Int32 _nCommandType = -1
                            ,const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                        );
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newQueryWithPilot(
                            const sal_Int32 _nCommandType = -1
                            ,const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                        );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newTableWithPilot();

        enum RESULT
        {
            ERROR,
            SUCCESS,
            CANCEL
        };
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>   implOpen(const ::rtl::OUString& _rLinkName
                                ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                                , EOpenMode _eOpenMode);
    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_LINKEDDOCUMENTS_HXX_

