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

