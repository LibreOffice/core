/*************************************************************************
 *
 *  $RCSfile: linkeddocuments.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:00:33 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        Window*     m_pDialogParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xDocumentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    m_xConnection;
        String      m_sCurrentlyEditing;

    public:
        OLinkedDocumentsAccess(
            Window* _pDialogParent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
            );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       open(const ::rtl::OUString& _rLinkName
                                                                                    ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                                                                                    , sal_Bool _bReadOnly = sal_True);

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       newForm(sal_Int32 _nNewFormId
                                ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition);

        sal_Bool        newFormWithPilot(
                            const String& _rDataSourceName,
                            const sal_Int32 _nCommandType,
                            const String& _rObjectName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                        );
        sal_Bool        newReportWithPilot(
                            const String& _rDataSourceName,
                            const sal_Int32 _nCommandType,
                            const String& _rObjectName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                        );
        sal_Bool        newQueryWithPilot(
                            const String& _rDataSourceName,
                            const sal_Int32 _nCommandType,
                            const String& _rObjectName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                        );
    protected:
        enum RESULT
        {
            ERROR,
            SUCCESS,
            CANCEL
        };
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>   implOpen(const ::rtl::OUString& _rLinkName
                                ,::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition
                                , sal_Bool _bReadOnly = sal_True);
        void    implDrop(const ::rtl::OUString& _rLinkName);
    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_LINKEDDOCUMENTS_HXX_

