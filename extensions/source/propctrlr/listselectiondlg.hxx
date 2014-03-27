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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>


namespace pcr
{



    //= ListSelectionDialog

    class ListSelectionDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        ListBox         m_aEntries;
        OKButton        m_aOK;
        CancelButton    m_aCancel;
        HelpButton      m_aHelp;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xListBox;
        OUString m_sPropertyName;

    public:
        ListSelectionDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxListBox,
            const OUString& _rPropertyName,
            const OUString& _rPropertyUIName
        );

        // Dialog overridables
        virtual short   Execute() SAL_OVERRIDE;

    private:
        void    initialize( );
        void    commitSelection();

        void    fillEntryList   ( const ::com::sun::star::uno::Sequence< OUString >& _rListEntries );

        void    selectEntries   ( const ::com::sun::star::uno::Sequence< sal_Int16 >& /* [in ] */ _rSelection );
        void    collectSelection(       ::com::sun::star::uno::Sequence< sal_Int16 >& /* [out] */ _rSelection );
    };


} // namespacepcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
