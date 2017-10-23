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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DLGSAVE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DLGSAVE_HXX

#include <apitools.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/msgbox.hxx>
#include <memory>
#include <o3tl/typed_flags_set.hxx>

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XConnection;
    }
}}}

enum class SADFlags {
    NONE                  = 0x0000,
    AdditionalDescription = 0x0001,
    TitlePasteAs          = 0x0100,
    TitleRename           = 0x0200,
};
namespace o3tl {
    template<> struct typed_flags<SADFlags> : is_typed_flags<SADFlags, 0x0301> {};
}


class Button;
class Edit;
namespace dbaui
{
    class OSaveAsDlgImpl;
    class IObjectNameCheck;
    class OSaveAsDlg : public ModalDialog
    {
    private:
        std::unique_ptr<OSaveAsDlgImpl> m_pImpl;
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    public:
        OSaveAsDlg( vcl::Window * pParent, sal_Int32 _rType,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags = SADFlags::NONE);

        OSaveAsDlg( vcl::Window* _pParent,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const OUString& _rDefault,
                    const OUString& _sLabel,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags = SADFlags::NONE);
        virtual ~OSaveAsDlg() override;
        virtual void dispose() override;

        const OUString& getName() const;
        OUString getCatalog() const;
        OUString getSchema() const;
    private:
        DECL_LINK(ButtonClickHdl, Button *, void);
        DECL_LINK(EditModifyHdl,  Edit&, void);

        void implInitOnlyTitle(const OUString& _rLabel);
        void implInit();
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DLGSAVE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
