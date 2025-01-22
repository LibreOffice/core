/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SFX2_INC_GUISAVEAS_HXX
#define INCLUDED_SFX2_INC_GUISAVEAS_HXX

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

#include <comphelper/sequenceashashmap.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/signaturestate.hxx>

#include <svtools/dialogclosedlistener.hxx>

#include <tools/urlobj.hxx>


namespace com::sun::star::document { class XDocumentProperties; }

namespace weld { class Window; }
class ModelData_Impl;

class SfxStoringHelper
{
    friend class ModelData_Impl;

private:
    css::uno::Reference< css::container::XNameAccess >     m_xFilterCFG;
    css::uno::Reference< css::container::XContainerQuery > m_xFilterQuery;
    css::uno::Reference< css::frame::XModuleManager2 >     m_xModuleManager;

    std::shared_ptr<ModelData_Impl> m_xModelData;
    css::uno::Sequence< css::beans::PropertyValue > m_aArgsSequence;

    css::uno::Reference< css::container::XNameAccess > const & GetFilterConfiguration();
    css::uno::Reference< css::container::XContainerQuery > const & GetFilterQuery();
    css::uno::Reference< css::frame::XModuleManager2 > const & GetModuleManager();

    bool m_bRemote;
    bool m_bPreselectPassword;
    bool m_bDialogUsed;
    bool m_bSetStandardName;
    sal_Int16 m_nStoreMode;

    static bool FinishGUIStoreModel(::comphelper::SequenceAsHashMap::const_iterator& aFileNameIter,
                             ModelData_Impl& aModelData, bool bRemote, sal_Int16 nStoreMode,
                             css::uno::Sequence< css::beans::PropertyValue >& aFilterProps,
                             bool bSetStandardName, bool bPreselectPassword, bool bDialogUsed,
                             std::u16string_view aFilterFromMediaDescr, std::u16string_view aOldFilterName,
                             css::uno::Sequence< css::beans::PropertyValue >& aArgsSequence,
                             OUString aFilterName);

    void CallFinishGUIStoreModel();

public:
    SfxStoringHelper();

    bool GUIStoreModel(
                    const css::uno::Reference< css::frame::XModel >& xModel,
                    std::u16string_view aSlotName,
                    css::uno::Sequence< css::beans::PropertyValue >& aArgsSequence,
                    bool bPreselectPassword,
                    SignatureState nDocumentSignatureState,
                    bool bIsAsync );

    static bool CheckFilterOptionsAppearance(
                    const css::uno::Reference< css::container::XNameAccess >& xFilterCFG,
                    const OUString& aFilterName );


    static void SetDocInfoState(
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::document::XDocumentProperties>& i_xOldDocInfo );

    static bool WarnUnacceptableFormat(
                                    const css::uno::Reference< css::frame::XModel >& xModel,
                                    std::u16string_view aOldUIName,
                                    std::u16string_view aExtension,
                                    const OUString& aDefExtension,
                                    bool rDefaultIsAlien );

    static css::uno::Reference<css::awt::XWindow> GetModelXWindow(const css::uno::Reference<css::frame::XModel>& rModel);
    static weld::Window* GetModelWindow( const css::uno::Reference< css::frame::XModel >& xModel );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
