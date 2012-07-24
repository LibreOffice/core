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

#ifndef _OBJDLG_HXX
#define _OBJDLG_HXX

#include <svheader.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include "vcl/image.hxx"

#include <bastype2.hxx>

class StarBASIC;

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
