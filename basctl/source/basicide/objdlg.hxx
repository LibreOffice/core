/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objdlg.hxx,v $
 * $Revision: 1.6 $
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

class ObjectTreeListBox : public BasicTreeListBox
{
private:

    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

public:
            ObjectTreeListBox( Window* pParent, const ResId& rRes );
            ~ObjectTreeListBox();
};

class ObjectCatalogToolBox_Impl: public ToolBox
{
public:
    ObjectCatalogToolBox_Impl(Window * pParent, ResId const & rResId,
                              ResId const & rImagesHighContrastId);

private:
    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    void setImages();

    ImageList m_aImagesNormal;
    ImageList m_aImagesHighContrast;
    bool m_bHighContrast;
};

class ObjectCatalog : public FloatingWindow
{
private:
    ObjectTreeListBox   aMacroTreeList;
    ObjectCatalogToolBox_Impl aToolBox;
    FixedText           aMacroDescr;
    Link                aCancelHdl;

protected:
    DECL_LINK( ToolBoxHdl, ToolBox* );
    void                CheckButtons();
    DECL_LINK( TreeListHighlightHdl, SvTreeListBox * );
    void                UpdateFields();
    virtual void        Move();
    virtual BOOL        Close();
    virtual void        Resize();

public:
    ObjectCatalog( Window * pParent );
    virtual ~ObjectCatalog();

    void                UpdateEntries();
    void                SetCurrentEntry( BasicEntryDescriptor& rDesc );

    void                SetCancelHdl( const Link& rLink ) { aCancelHdl = rLink; }
};

#endif  //_OBJDLG_HXX

