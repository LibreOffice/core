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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX

#include <vcl/weld.hxx>

namespace svx
{
    class WebConnectionInfoDialog : public weld::GenericDialogController
    {
    private:
        sal_Int32 m_nPos;

        std::unique_ptr<weld::Button> m_xRemoveBtn;
        std::unique_ptr<weld::Button> m_xRemoveAllBtn;
        std::unique_ptr<weld::Button> m_xChangeBtn;
        std::unique_ptr<weld::TreeView> m_xPasswordsLB;

        DECL_LINK( HeaderBarClickedHdl, int, void );
        DECL_LINK( RemovePasswordHdl, weld::Button&, void );
        DECL_LINK( RemoveAllPasswordsHdl, weld::Button&, void );
        DECL_LINK( ChangePasswordHdl, weld::Button&, void );
        DECL_LINK( EntrySelectedHdl, weld::TreeView&, void );

        void FillPasswordList();

    public:
        explicit WebConnectionInfoDialog(weld::Window* pParent);
        virtual ~WebConnectionInfoDialog() override;
    };


}


#endif // INCLUDED_CUI_SOURCE_OPTIONS_WEBCONNINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
