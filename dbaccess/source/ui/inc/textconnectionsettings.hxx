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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TEXTCONNECTIONSETTINGS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TEXTCONNECTIONSETTINGS_HXX

#include "propertystorage.hxx"

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

#include <memory>

class SfxItemSet;
namespace dbaui
{

    class OTextConnectionHelper;

    // TextConnectionSettingsDialog
    class TextConnectionSettingsDialog : public ModalDialog
    {
    public:
        TextConnectionSettingsDialog( vcl::Window* _pParent, SfxItemSet& _rItems );
        virtual ~TextConnectionSettingsDialog();
        virtual void dispose() override;

        /** initializes a set of PropertyStorage instances, which are bound to
            the text-connection relevant items in our item sets
        */
        static void bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues );

        virtual short   Execute() override;

    private:
        VclPtr<OTextConnectionHelper>               m_pTextConnectionHelper;
        VclPtr<OKButton>                            m_pOK;
        SfxItemSet&                                 m_rItems;

    private:
        DECL_LINK_TYPED( OnOK, Button*, void );
    };

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TEXTCONNECTIONSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
