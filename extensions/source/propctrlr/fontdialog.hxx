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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FONTDIALOG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FONTDIALOG_HXX

#include <sfx2/tabdlg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>


namespace pcr
{



    //= ControlCharacterDialog

    class ControlCharacterDialog : public SfxTabDialog
    {
    public:
        ControlCharacterDialog(vcl::Window* _pParent, const SfxItemSet& _rCoreSet);
        virtual ~ControlCharacterDialog();

        /// creates an item set to be used with this dialog
        static SfxItemSet*  createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

        /// destroys an item previously created with <method>createItemSet</method>
        static void         destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

        /// fills the given item set with values obtained from the given property set
        static void         translatePropertiesToItems(
            const css::uno::Reference< css::beans::XPropertySet >& _rxModel,
            SfxItemSet* _pSet);

        /** fills the given property set with values obtained from the given item set
        */
        static void translateItemsToProperties(
            const SfxItemSet& _rSet,
            const css::uno::Reference< css::beans::XPropertySet >& _rxModel);

        /** fills the given property set with values obtained from the given item set
        */
        static void translateItemsToProperties(
            const SfxItemSet& _rSet,
            css::uno::Sequence< css::beans::NamedValue >& _out_properties );

    protected:
        virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage) SAL_OVERRIDE;
    private:
        sal_uInt16 m_nCharsId;
    };

}   // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FONTDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
