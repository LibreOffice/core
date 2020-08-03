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

#include <map>
#include <sot/formats.hxx>
#include <tools/globname.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>

struct TransferableObjectDescriptor;
class TransferableDataHelper;

class SvPasteObjectDialog : public weld::GenericDialogController
{
    std::map< SotClipboardFormatId, OUString > aSupplementMap;
    // Additional UNO command to be displayed along the supported paste formats
    std::pair<OUString, OUString> aExtraCommand;
    SvGlobalName    aObjClassName;
    OUString        aObjName;

    std::unique_ptr<weld::Label> m_xFtObjectSource;
    std::unique_ptr<weld::TreeView> m_xLbInsertList;
    std::unique_ptr<weld::Button> m_xOKButton;

    weld::TreeView& ObjectLB() { return *m_xLbInsertList; }

    void            SelectObject();
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);

public:
    SvPasteObjectDialog(weld::Window* pParent);

    void        Insert( SotClipboardFormatId nFormat, const OUString & rFormatName );
    void        InsertUno( const OUString& sUnoCmd, const OUString& sLabel);
    void        SetObjName( const SvGlobalName & rClass, const OUString & rObjName );
    /**
     * @brief PreGetFormat Prepares the dialog for running to get format of paste as a SotClipboardFormatId value by calling GetFormatOnly()
     * @param aHelper
     */
    void        PreGetFormat( const TransferableDataHelper& aHelper);
    /**
     * @brief GetFormatOnly Returns a SotClipboardFormatId value. Should be called after actually running the dialog.
     * @return
     */
    SotClipboardFormatId GetFormatOnly();
    /**
     * @brief GetFormat Prepares and runs the dialog, and returns a SotClipboardFormatId depending on the RET_OK result
     * @param aHelper TransferableDataHelper containing the data to be pasted
     * @return a SotClipboardFormatId value depending on the result of running the dialog
     */
    SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
