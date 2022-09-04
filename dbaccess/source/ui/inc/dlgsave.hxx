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

#include "SqlNameEdit.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/weld.hxx>
#include <memory>

namespace com::sun::star {
    namespace sdbc {
        class XConnection;
    }
}

enum class SADFlags {
    NONE                  = 0x0000,
    AdditionalDescription = 0x0001,
    TitlePasteAs          = 0x0100,
    TitleRename           = 0x0200,
};
namespace o3tl {
    template<> struct typed_flags<SADFlags> : is_typed_flags<SADFlags, 0x0301> {};
}

namespace dbaui
{
    class IObjectNameCheck;
    class OSaveAsDlg : public weld::GenericDialogController
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        OUString                   m_aQryLabel;
        OUString                   m_sTblLabel;
        OUString                   m_aName;
        const IObjectNameCheck&    m_rObjectNameCheck;
        css::uno::Reference< css::sdbc::XDatabaseMetaData>            m_xMetaData;
        sal_Int32                  m_nType;
        SADFlags                   m_nFlags;

        OSQLNameChecker            m_aChecker;

        std::unique_ptr<weld::Label> m_xDescription;
        std::unique_ptr<weld::Label> m_xCatalogLbl;
        std::unique_ptr<weld::ComboBox> m_xCatalog;
        std::unique_ptr<weld::Label> m_xSchemaLbl;
        std::unique_ptr<weld::ComboBox> m_xSchema;
        std::unique_ptr<weld::Label> m_xLabel;
        std::unique_ptr<weld::Entry> m_xTitle;
        std::unique_ptr<weld::Button> m_xPB_OK;

        DECL_LINK(TextFilterHdl, OUString&, bool);

    public:
        OSaveAsDlg( weld::Window * pParent, sal_Int32 _rType,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags);

        OSaveAsDlg( weld::Window* _pParent,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const OUString& _rDefault,
                    const OUString& _sLabel,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags);
        virtual ~OSaveAsDlg() override;

        const OUString& getName() const;
        OUString getCatalog() const;
        OUString getSchema() const;
    private:
        DECL_LINK(ButtonClickHdl, weld::Button&, void);
        DECL_LINK(EditModifyHdl,  weld::Entry&, void);

        void implInitOnlyTitle(const OUString& _rLabel);
        void implInit();
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
