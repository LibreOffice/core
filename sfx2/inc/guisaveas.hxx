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

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <sfx2/signaturestate.hxx>


namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

namespace vcl { class Window; }
class ModelData_Impl;

class SfxStoringHelper
{
    friend class ModelData_Impl;

private:
    css::uno::Reference< css::container::XNameAccess >     m_xFilterCFG;
    css::uno::Reference< css::container::XContainerQuery > m_xFilterQuery;
    css::uno::Reference< css::frame::XModuleManager2 >     m_xModuleManager;

    css::uno::Reference< css::container::XNameAccess > const & GetFilterConfiguration();
    css::uno::Reference< css::container::XContainerQuery > const & GetFilterQuery();
    css::uno::Reference< css::frame::XModuleManager2 > const & GetModuleManager();

public:
    SfxStoringHelper();

    bool GUIStoreModel(
                    const css::uno::Reference< css::frame::XModel >& xModel,
                    const OUString& aSlotName,
                    css::uno::Sequence< css::beans::PropertyValue >& aArgsSequence,
                    bool bPreselectPassword,
                    const OUString& aUserSelectedName,
                    SignatureState nDocumentSignatureState = SignatureState::NOSIGNATURES );

    static bool CheckFilterOptionsAppearence(
                    const css::uno::Reference< css::container::XNameAccess >& xFilterCFG,
                    const OUString& aFilterName );


    static void SetDocInfoState(
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::document::XDocumentProperties>& i_xOldDocInfo,
        bool bNoModify );

    static bool WarnUnacceptableFormat(
                                    const css::uno::Reference< css::frame::XModel >& xModel,
                                    const OUString& aOldUIName,
                                    const OUString& aDefUIName,
                                    const OUString& aDefExtension,
                                    bool bCanProceedFurther,
                                    bool rDefaultIsAlien );

    static vcl::Window* GetModelWindow( const css::uno::Reference< css::frame::XModel >& xModel );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
