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

#pragma once

#include <unordered_map>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <cppuhelper/implbase.hxx>

namespace framework
{
    class GraphicNameAccess final : public ::cppu::WeakImplHelper< css::container::XNameAccess >
    {
        public:
            GraphicNameAccess();
            virtual ~GraphicNameAccess() override;

            void addElement( const OUString& rName, const css::uno::Reference< css::graphic::XGraphic >& rElement );

            // XNameAccess
            virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
            virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements() override;
            virtual css::uno::Type SAL_CALL getElementType(  ) override;

        private:
            typedef std::unordered_map<OUString, css::uno::Reference< css::graphic::XGraphic >> NameGraphicHashMap;
            NameGraphicHashMap              m_aNameToElementMap;
            css::uno::Sequence< OUString >  m_aSeq;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
