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

#ifndef INCLUDED_CUI_SOURCE_INC_PASTEDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_PASTEDLG_HXX

#include <map>
#include <sot/formats.hxx>
#include <tools/globname.hxx>
#include <svtools/transfer.hxx>

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

/********************** SvPasteObjectDialog ******************************
*************************************************************************/

struct TransferableObjectDescriptor;
class TransferableDataHelper;

class SvPasteObjectDialog : public ModalDialog
{
    VclPtr<FixedText> m_pFtObjectSource;
    VclPtr<ListBox> m_pLbInsertList;
    VclPtr<OKButton> m_pOKButton;
    ::std::map< SotClipboardFormatId, OUString > aSupplementMap;
    SvGlobalName    aObjClassName;
    OUString        aObjName;

    ListBox&        ObjectLB()      { return *m_pLbInsertList; }
    FixedText&      ObjectSource()  { return *m_pFtObjectSource; }

    void            SelectObject();
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK_TYPED( DoubleClickHdl, ListBox&, void );

public:
                SvPasteObjectDialog( vcl::Window* pParent );
    virtual     ~SvPasteObjectDialog();
    virtual void dispose() SAL_OVERRIDE;

    void        Insert( SotClipboardFormatId nFormat, const OUString & rFormatName );
    void        SetObjName( const SvGlobalName & rClass, const OUString & rObjName );
    SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 );
};

#endif // INCLUDED_CUI_SOURCE_INC_PASTEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
