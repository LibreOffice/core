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

#ifndef _OBJDLG_HXX
#define _OBJDLG_HXX

#include <svheader.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include "vcl/image.hxx"

#include <bastype2.hxx>

class ObjectCatalogToolBox_Impl: public ToolBox
{
public:
    ObjectCatalogToolBox_Impl(Window * pParent, ResId const & rResId);

private:
    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    void setImages();

    ImageList m_aImagesNormal;
};

class ObjectCatalog : public BasicDockingWindow
{
private:
    BasicTreeListBox   aMacroTreeList;
    ObjectCatalogToolBox_Impl aToolBox;
    FixedText           aMacroDescr;
    Link                aCancelHdl;

protected:
    DECL_LINK( ToolBoxHdl, ToolBox* );
    void                CheckButtons();
    DECL_LINK( TreeListHighlightHdl, SvTreeListBox * );
    void                UpdateFields();
    virtual void        Move();
    virtual sal_Bool        Close();
    virtual void        Resize();
    virtual void    Paint( const Rectangle& rRect );

public:
    ObjectCatalog( Window * pParent );
    virtual ~ObjectCatalog();

    void                UpdateEntries();
    void                SetCurrentEntry (IDEBaseWindow* pCurWin);
    void                SetCancelHdl( const Link& rLink ) { aCancelHdl = rLink; }
};

#endif  //_OBJDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
