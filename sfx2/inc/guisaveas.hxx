/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SFX_GUISAVEAS_HXX_
#define _SFX_GUISAVEAS_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <sfx2/signaturestate.hxx>


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
                    sal_Bool bPreselectPassword,
                    ::rtl::OUString aUserSelectedName,
                    sal_uInt16 nDocumentSignatureState = SIGNATURESTATE_NOSIGNATURES );

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

