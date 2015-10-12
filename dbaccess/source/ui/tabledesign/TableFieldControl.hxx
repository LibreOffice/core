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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDCONTROL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDCONTROL_HXX

#include "FieldDescControl.hxx"

namespace dbaui
{
    class OTableEditorCtrl;
    class OTableDesignHelpBar;
    // OTableFieldControl
    class OTableFieldControl : public OFieldDescControl
    {
        OTableEditorCtrl*   GetCtrl() const;
    protected:
        virtual void        ActivateAggregate( EControlType eType ) override;
        virtual void        DeactivateAggregate( EControlType eType ) override;
        // are to be implemented by the derived classes
        virtual void        CellModified(long nRow, sal_uInt16 nColId ) override;
        virtual bool        IsReadOnly() override;
        virtual void        SetModified(bool bModified) override;
        virtual css::uno::Reference< css::util::XNumberFormatter >    GetFormatter() const override;

        virtual css::lang::Locale  GetLocale() const override;

        virtual TOTypeInfoSP                    getTypeInfo(sal_Int32 _nPos) override;
        virtual const OTypeInfoMap*             getTypeInfo() const override;
        virtual bool                            isAutoIncrementValueEnabled() const override;
        virtual OUString                        getAutoIncrementValue() const override;

    public:
        OTableFieldControl( vcl::Window* pParent, OTableDesignHelpBar* pHelpBar);

        OUString BoolStringPersistent(const OUString& rUIString) const { return OFieldDescControl::BoolStringPersistent(rUIString); }
        OUString BoolStringUI(const OUString& rPersistentString) const { return OFieldDescControl::BoolStringUI(rPersistentString); }

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() override;
        virtual css::uno::Reference< css::sdbc::XConnection> getConnection() override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
