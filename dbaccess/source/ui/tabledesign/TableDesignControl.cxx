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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#include "TableDesignControl.hxx"
#endif
#ifndef _DBU_TBL_HRC_
#include "dbu_tbl.hrc"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

using namespace ::dbaui;
using namespace ::svt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
//--- Defines
#define HANDLE_ID 0

DBG_NAME(OTableRowView)
//------------------------------------------------------------------------
OTableRowView::OTableRowView(Window* pParent)
    :EditBrowseBox(pParent, ModuleRes(RID_DB_TAB_EDITOR),EBBF_NONE,
                    BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_AUTOSIZE_LASTCOL |
                    BROWSER_KEEPSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nCurUndoActId(0)
    ,m_bCurrentModified(sal_False)
    ,m_bUpdatable(sal_False)
    ,m_bClipboardFilled(sal_False)
{
    DBG_CTOR(OTableRowView,NULL);

}

//------------------------------------------------------------------------
OTableRowView::~OTableRowView()
{

    DBG_DTOR(OTableRowView,NULL);
}

//------------------------------------------------------------------------
void OTableRowView::Init()
{
    EditBrowseBox::Init();

//  SetMapMode( MapMode(MAP_TWIP) );
//  GetDataWindow().SetMapMode( GetMapMode() );

    Font aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    // Font fuer die Ueberschriften auf Light setzen
    aFont = GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    SetFont(aFont);

    // HandleColumn, fuer maximal fuenf Ziffern einrichten
    InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth('0') * 4)/*, sal_True */);

    BrowserMode nMode = BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                        BROWSER_HLINESFULL | BROWSER_VLINESFULL | BROWSER_AUTOSIZE_LASTCOL;
    if (IsUpdatable())
        nMode |= BROWSER_HIDECURSOR;

    SetMode(nMode);
}

//------------------------------------------------------------------------
void OTableRowView::KeyInput( const KeyEvent& rEvt )
{
    if (IsDeleteAllowed(0))
    {
        if (rEvt.GetKeyCode().GetCode() == KEY_DELETE &&    // Delete rows
            !rEvt.GetKeyCode().IsShift() &&
            !rEvt.GetKeyCode().IsMod1())
        {
            DeleteRows();
            return;
        }
        if( rEvt.GetKeyCode().GetCode() == KEY_F2 )
        {
            ::com::sun::star::util::URL aUrl;
            aUrl.Complete =::rtl::OUString::createFromAscii(".uno:DSBEditDoc");
            GetView()->getController().dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

//------------------------------------------------------------------------
void OTableRowView::SetUpdatable( sal_Bool bUpdate )
{
    m_bUpdatable = bUpdate;

}

//------------------------------------------------------------------------
void OTableRowView::Command(const CommandEvent& rEvt)
{

    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if (!rEvt.IsMouseEvent())
            {
                EditBrowseBox::Command(rEvt);
                return;
            }

            sal_uInt16 nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
            long   nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());

            if ( nColId == HANDLE_ID )
            {
                PopupMenu aContextMenu(ModuleRes(RID_TABLEDESIGNROWPOPUPMENU));
                long nSelectRowCount = GetSelectRowCount();
                aContextMenu.EnableItem( SID_CUT, nSelectRowCount != 0);
                aContextMenu.EnableItem( SID_COPY, nSelectRowCount  != 0);
                aContextMenu.EnableItem( SID_PASTE, m_bClipboardFilled );
                aContextMenu.EnableItem( SID_DELETE, IsUpdatable() && nSelectRowCount != 0 );
                switch (aContextMenu.Execute(this, rEvt.GetMousePosPixel()))
                {
                    case SID_CUT:
                        cut();
                        break;
                    case SID_COPY:
                        copy();
                        break;
                    case SID_PASTE:
                        Paste( nRow );
                        SetNoSelection();
                        GoToRow( nRow );
                        SeekRow( nRow );
                        break;

                    case SID_DELETE:
                        DeleteRows();
                        break;
                    case SID_TABLEDESIGN_INSERTROWS:
                        InsertNewRows( nRow );
                        SetNoSelection();
                        GoToRow( nRow );
                        SeekRow( nRow );
                        break;
                    default:
                        break;
                }
            }

        }
        default:
            EditBrowseBox::Command(rEvt);
    }

}

//------------------------------------------------------------------------------
void OTableRowView::cut()
{
    CopyRows();
    DeleteRows();
}

//------------------------------------------------------------------------------
void OTableRowView::copy()
{
    CopyRows();
}

//------------------------------------------------------------------------------
void OTableRowView::paste()
{
    OSL_ENSURE(0,"OTableRowView::Paste : (pseudo-) abstract method called !");
}

//------------------------------------------------------------------------------
void OTableRowView::Paste( long nRow )
{
    InsertRows( nRow );
}

//------------------------------------------------------------------------------
EditBrowseBox::RowStatus OTableRowView::GetRowStatus(long nRow) const
{
    if (nRow >= 0 && m_nDataPos == nRow)
        return CURRENT;
    else
        return CLEAN;
}



