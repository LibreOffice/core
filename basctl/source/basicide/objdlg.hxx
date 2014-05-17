/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual void    KeyInput( const KeyEvent& rEvt );

    bool            OpenCurrent();
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
    virtual sal_Bool        Close();
    virtual void        Resize();

public:
    ObjectCatalog( Window * pParent );
    virtual ~ObjectCatalog();

    void                UpdateEntries();
    void                SetCurrentEntry( BasicEntryDescriptor& rDesc );

    void                SetCancelHdl( const Link& rLink ) { aCancelHdl = rLink; }
};

#endif  //_OBJDLG_HXX

