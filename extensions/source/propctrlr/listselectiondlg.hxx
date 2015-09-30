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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

namespace pcr
{
    class ListSelectionDialog : public ModalDialog
    {
    private:
        VclPtr<ListBox>        m_pEntries;

        css::uno::Reference< css::beans::XPropertySet >
                               m_xListBox;
        OUString               m_sPropertyName;

    public:
        ListSelectionDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxListBox,
            const OUString& _rPropertyName,
            const OUString& _rPropertyUIName
        );
        virtual ~ListSelectionDialog();
        virtual void dispose() SAL_OVERRIDE;

        // Dialog overridables
        virtual short   Execute() SAL_OVERRIDE;

    private:
        void    initialize( );
        void    commitSelection();

        void    fillEntryList   ( const css::uno::Sequence< OUString >& _rListEntries );

        void    selectEntries   ( const css::uno::Sequence< sal_Int16 >& /* [in ] */ _rSelection );
        void    collectSelection(       css::uno::Sequence< sal_Int16 >& /* [out] */ _rSelection );
    };


} // namespacepcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
