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

#include <vcl/weld.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

namespace pcr
{
    class ListSelectionDialog : public weld::GenericDialogController
    {
    private:
        css::uno::Reference<css::beans::XPropertySet> m_xListBox;
        OUString m_sPropertyName;
        std::unique_ptr<weld::Frame> m_xFrame;
        std::unique_ptr<weld::TreeView> m_xEntries;

    public:
        ListSelectionDialog(
            weld::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxListBox,
            const OUString& _rPropertyName,
            const OUString& _rPropertyUIName
        );
        virtual ~ListSelectionDialog() override;

        virtual short run() override;

    private:
        void    initialize( );
        void    commitSelection();

        void    fillEntryList   ( const css::uno::Sequence< OUString >& _rListEntries );

        void    selectEntries   ( const css::uno::Sequence< sal_Int16 >& /* [in ] */ _rSelection );
        void    collectSelection(       std::vector< sal_Int16 >& /* [out] */ _rSelection );
    };

} // namespacepcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
