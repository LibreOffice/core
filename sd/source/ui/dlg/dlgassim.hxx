/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svl/dateitem.hxx>
#include "drawdoc.hxx"
#include <svx/svdotext.hxx>
#include <svtools/treelistbox.hxx>

#include "sdresid.hxx"

class SdPageListControl : public SvTreeListBox
{
private:
    SvLBoxButtonData*   m_pCheckButton;

    SvTreeListEntry* InsertPage( const String& rPageName );
    void InsertTitle( SvTreeListEntry* pEntry, const String& rTitle );

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
