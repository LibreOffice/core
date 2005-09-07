/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:10:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OBJDLG_HXX
#define _OBJDLG_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include "vcl/image.hxx"
#endif

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

