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

#include <svl/dateitem.hxx>
#include "drawdoc.hxx"
#include <svx/svdotext.hxx>
#include <svtools/treelistbox.hxx>

#include "sdresid.hxx"

class SdPageListControl : public SvTreeListBox
{
private:
    SvLBoxButtonData*   m_pCheckButton;

    SvTreeListEntry* InsertPage( const OUString& rPageName );
    void InsertTitle( SvTreeListEntry* pEntry, const OUString& rTitle );

public:
    SdPageListControl( Window* pParent, const ResId& rResId );
    ~SdPageListControl();

    void Fill( SdDrawDocument* pDoc );
    void Clear();

    sal_uInt16 GetSelectedPage();
    sal_Bool IsPageChecked( sal_uInt16 nPage );

    DECL_LINK( CheckButtonClickHdl, void * );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
