/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guisaveas.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:56:54 $
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

#ifndef _SFX_GUISAVEAS_HXX_
#define _SFX_GUISAVEAS_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#include <comphelper/sequenceashashmap.hxx>


namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentInfo;
    }
} } }

class Window;
class ModelData_Impl;

class SfxStoringHelper
{
    friend class ModelData_Impl;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xFilterCFG;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery > m_xFilterQuery;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > m_xModuleManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xNamedModManager;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > GetServiceFactory();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetFilterConfiguration();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery > GetFilterQuery();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > GetModuleManager();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetNamedModuleManager();


public:
    SfxStoringHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    sal_Bool GUIStoreModel(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel,
                    const ::rtl::OUString& aSlotName,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgsSequence,
                    sal_Bool bPreselectPassword );

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SearchForFilter(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery >& xFilterQuery,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aSearchRequest,
                    sal_Int32 nMustFlags,
                    sal_Int32 nDontFlags );

    static sal_Bool CheckFilterOptionsAppearence(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xFilterCFG,
                    const ::rtl::OUString& aFilterName );


    static void SetDocInfoState(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo>& i_xOldDocInfo,
        sal_Bool bNoModify );

    static sal_Bool WarnUnacceptableFormat(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel,
                                    ::rtl::OUString aOldUIName,
                                    ::rtl::OUString aDefUIName,
                                    sal_Bool bCanProceedFurther );

    static void ExecuteFilterDialog( SfxStoringHelper& _rStorageHelper
                                    ,const ::rtl::OUString& sFilterName
                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel
                                    ,/*OUT*/::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgsSequence
                                );

    static Window* GetModelWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );

};

#endif

