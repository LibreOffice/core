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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

namespace dbaui
{
    // OColumnControlWindow
    class OColumnControlWindow : public OFieldDescControl
    {
        css::lang::Locale      m_aLocale;
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
        css::uno::Reference< css::sdbc::XConnection>          m_xConnection;
        mutable css::uno::Reference< css::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        OTypeInfoMap                m_aDestTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aDestTypeInfoIndex;

        mutable TOTypeInfoSP        m_pTypeInfo; // default type
        OUString                    m_sTypeNames;       // these type names are the ones out of the resource file
        OUString                    m_sAutoIncrementValue;
        bool                        m_bAutoIncrementEnabled;
    protected:
        virtual void        ActivateAggregate( EControlType eType ) override;
        virtual void        DeactivateAggregate( EControlType eType ) override;

        virtual css::lang::Locale  GetLocale() const override;
        virtual css::uno::Reference< css::util::XNumberFormatter > GetFormatter() const override;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos) override;
        virtual bool                isAutoIncrementValueEnabled() const override;
        virtual OUString            getAutoIncrementValue() const override;
        virtual void                CellModified(long nRow, sal_uInt16 nColId ) override;

    public:
        OColumnControlWindow(vcl::Window* pParent
                            ,const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

        void setConnection(const css::uno::Reference< css::sdbc::XConnection>& _xCon);

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() override;
        virtual css::uno::Reference< css::sdbc::XConnection> getConnection() override;
        virtual const OTypeInfoMap* getTypeInfo() const override;
        TOTypeInfoSP const & getDefaultTyp() const;
    };
}   // namespace dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
