/*************************************************************************
 *
 *  $RCSfile: TableWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:54:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <vcl/wall.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_QUERY_HRC
#include "Query.hrc"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif


using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

const long TABWIN_SIZING_AREA = 4;
const long LISTBOX_SCROLLING_AREA = 6;
const ULONG SCROLLING_TIMESPAN = 500;


TYPEINIT0(OTableWindow);
//========================================================================
// class OTableWindow
//========================================================================
DBG_NAME(OTableWindow);
//------------------------------------------------------------------------------
OTableWindow::OTableWindow( Window* pParent, OTableWindowData* pTabWinData )
          :Window( pParent, WB_3DLOOK|WB_MOVEABLE )
          ,m_aTitle( this )
          ,m_nSizingFlags( SIZING_NONE )
          ,m_bActive( FALSE )
          ,m_pListBox(NULL)
{
    DBG_CTOR(OTableWindow,NULL);
    // ich uebernehme nicht die Verantwortung fuer die Daten, ich merke mir nur den Zeiger darauf
    m_pData = pTabWinData;

    // Position und Groesse bestimmen
    if( GetData()->HasPosition() )
        SetPosPixel( GetData()->GetPosition() );

    if( GetData()->HasSize() )
        SetSizePixel( GetData()->GetSize() );

    // Hintergrund setzen
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(Color(aSystemStyle.GetFaceColor())));
    // und Textfarbe (obwohl ich eigentlich keinen Text habe, aber wer weiss, was
    // abgeleitete Klassen machen)
    SetTextColor(aSystemStyle.GetButtonTextColor());

    EnableClipSiblings();
}

//------------------------------------------------------------------------------
OTableWindow::~OTableWindow()
{
    if (m_pListBox)
    {
        EmptyListBox();
        delete m_pListBox;
    }
    DBG_DTOR(OTableWindow,NULL);
}
// -----------------------------------------------------------------------------
const OQueryTableView* OTableWindow::getTableView() const
{
    OSL_ENSURE(static_cast<OQueryTableView*>(GetParent()),"No OQueryTableView!");
    return static_cast<OQueryTableView*>(GetParent());
}
// -----------------------------------------------------------------------------
OQueryTableView* OTableWindow::getTableView()
{
    OSL_ENSURE(static_cast<OQueryTableView*>(GetParent()),"No OQueryTableView!");
    return static_cast<OQueryTableView*>(GetParent());
}
// -----------------------------------------------------------------------------
OQueryDesignView* OTableWindow::getDesignView()
{
    OSL_ENSURE(static_cast<OQueryDesignView*>(GetParent()->GetParent()->GetParent()),"No OQueryDesignView!");
    return static_cast<OQueryDesignView*>(GetParent()->GetParent()->GetParent());
}
//------------------------------------------------------------------------------
void OTableWindow::SetPosPixel( const Point& rNewPos )
{
    Point aNewPosData = rNewPos + getTableView()->GetScrollOffset();
    GetData()->SetPosition( aNewPosData );
    Window::SetPosPixel( rNewPos );
}

//------------------------------------------------------------------------------
void OTableWindow::SetSizePixel( const Size& rNewSize )
{
    Size aOutSize(rNewSize);
    if( aOutSize.Width() < TABWIN_WIDTH_MIN )
        aOutSize.Width() = TABWIN_WIDTH_MIN;
    if( aOutSize.Height() < TABWIN_HEIGHT_MIN )
        aOutSize.Height() = TABWIN_HEIGHT_MIN;

    GetData()->SetSize( aOutSize );
    Window::SetSizePixel( aOutSize );
}
//------------------------------------------------------------------------------
void OTableWindow::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}

//------------------------------------------------------------------------------
OTableWindowListBox* OTableWindow::CreateListBox()
{
    return new OTableWindowListBox(this, GetComposedName(), GetTableName());
}

//------------------------------------------------------------------------------
BOOL OTableWindow::FillListBox()
{
    // mark all primary keys with special image
    ImageList aImageList(ModuleRes(IMG_JOINS));
    Image aPrimKeyImage = aImageList.GetImage(IMG_PRIMARY_KEY);

    if (GetData()->IsShowAll())
        m_pListBox->InsertEntry( ::rtl::OUString::createFromAscii("*") );

    // first we need the keys from the table
    Reference<XKeysSupplier> xKeys(m_xTable,UNO_QUERY);
    Reference<XNameAccess> xPKeyColumns;
    if(xKeys.is())
    {
        Reference< XIndexAccess> xKeyIndex = xKeys->getKeys();
        Reference<XColumnsSupplier> xColumnsSupplier;
        // search the one and only primary key
        for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
        {
            Reference<XPropertySet> xProp;
            ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
            sal_Int32 nKeyType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::PRIMARY == nKeyType)
            {
                xColumnsSupplier = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
                break;
            }
        }
        if(xColumnsSupplier.is())
            xPKeyColumns = xColumnsSupplier->getColumns();
    }
    Sequence< ::rtl::OUString> aColumns = m_xColumns->getElementNames();
    const ::rtl::OUString* pBegin = aColumns.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aColumns.getLength();

    for (; pBegin != pEnd; ++pBegin)
    {
        // is this column in the primary key
        if (xPKeyColumns.is() && xPKeyColumns->hasByName(*pBegin))
            m_pListBox->InsertEntry(*pBegin, aPrimKeyImage, aPrimKeyImage);
        else
            m_pListBox->InsertEntry(*pBegin);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
void OTableWindow::EmptyListBox()
{
    // da ich defaultmaessig keine USerData an die Items haenge, kann ich hier einfach loeschen
    SvLBoxEntry* pEntry = m_pListBox->First();

    while(pEntry)
    {
        SvLBoxEntry* pNextEntry = m_pListBox->Next(pEntry);
        m_pListBox->GetModel()->Remove(pEntry);
        pEntry = pNextEntry;
    }
}

//------------------------------------------------------------------------------
BOOL OTableWindow::Init()
{
    // get the from the connection
    OQueryDesignView* pParent = getDesignView();
    Reference<XConnection > xConnection = pParent->getController()->getConnection();
    Reference<XTablesSupplier> xSups(xConnection,UNO_QUERY);
    OSL_ENSURE(xSups.is(),"The connection isn't a tablessupplier!");
    Reference<XNameAccess> xTables = xSups->getTables();

    ::rtl::OUString aName = GetTableName();
    if(!xTables->hasByName(aName))
        return FALSE;

    ::cppu::extractInterface(m_xTable,xTables->getByName(aName));
    Reference<XColumnsSupplier> xColumnsSups(m_xTable,UNO_QUERY);
    m_xColumns = xColumnsSups->getColumns();


    // ListBox anlegen, wenn notwendig
    if (!m_pListBox)
    {
        m_pListBox = CreateListBox();
        DBG_ASSERT(m_pListBox != NULL, "OTableWindow::Init() : CreateListBox hat NULL geliefert !");
        m_pListBox->SetSelectionMode(MULTIPLE_SELECTION);
    }

    // Titel setzen
    m_aTitle.SetText( m_pData->GetWinName() );
    m_aTitle.Show();

    m_pListBox->Show();

    // die Felder in die ListBox eintragen
    EmptyListBox();
    if (!FillListBox())
        return FALSE;
    m_pListBox->SelectAll(FALSE);
    return TRUE;
}

//------------------------------------------------------------------------------
void OTableWindow::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS)
    {
        // nehmen wir den worst-case an : die Farben haben sich geaendert, also
        // mich anpassen
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(Color(aSystemStyle.GetFaceColor())));
        SetTextColor(aSystemStyle.GetButtonTextColor());
    }
}
//------------------------------------------------------------------------------
void OTableWindow::Paint( const Rectangle& rRect )
{
    Rectangle aRect( Point(0,0), GetOutputSizePixel() );
    Window::Paint( rRect );
    Draw3DBorder( aRect );
}

//------------------------------------------------------------------------------
void OTableWindow::SetTitle( const ::rtl::OUString& rTit )
{
    m_aTitle.SetText( rTit );
}

//------------------------------------------------------------------------------
void OTableWindow::Draw3DBorder(const Rectangle& rRect)
{
    // die Style-Settings des Systems fuer meine Farben
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();

    // Schwarze Linie unten und rechts
    SetLineColor(aSystemStyle.GetDarkShadowColor());
    DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
    DrawLine( rRect.BottomRight(), rRect.TopRight() );

    // Dunkelgraue Linie ueber der schwarzen
    SetLineColor(aSystemStyle.GetShadowColor());
    DrawLine( rRect.BottomLeft()+Point(1,-1), rRect.BottomRight()-Point(1,1) );
    DrawLine( rRect.BottomRight()-Point(1,1), rRect.TopRight()+Point(-1,1) );

    // Hellgraue Linie links und oben
    SetLineColor(aSystemStyle.GetLightColor());
    DrawLine( rRect.BottomLeft()+Point(1,-2), rRect.TopLeft()+Point(1,1) );
    DrawLine( rRect.TopLeft()+Point(1,1), rRect.TopRight()+Point(-2,1) );
}

//------------------------------------------------------------------------------
void OTableWindow::MouseMove( const MouseEvent& rEvt )
{
    Window::MouseMove(rEvt);

    OQueryTableView* pCont = getTableView();
    if (pCont->getDesignView()->getController()->isReadOnly())
        return;

    Point   aPos = rEvt.GetPosPixel();
    Size    aOutSize = GetOutputSizePixel();
    Pointer aPointer;

    //////////////////////////////////////////////////////////////////////
    // Flags anpassen, wenn Mauszeiger in sizingArea
    m_nSizingFlags = SIZING_NONE;

    if( aPos.X() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_LEFT;

    if( aPos.Y() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_TOP;

    if( aPos.X() > aOutSize.Width()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_RIGHT;

    if( aPos.Y() > aOutSize.Height()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_BOTTOM;

    //////////////////////////////////////////////////////////////////////
    // Mauszeiger anpassen, wenn Mauszeiger in sizingArea
    switch( m_nSizingFlags )
    {
    case SIZING_TOP:
    case SIZING_BOTTOM:
        aPointer = Pointer( POINTER_SSIZE );
        break;

    case SIZING_LEFT:
    case SIZING_RIGHT:
        aPointer = Pointer( POINTER_ESIZE );
        break;

    case SIZING_LEFT+SIZING_TOP:
    case SIZING_RIGHT+SIZING_BOTTOM:
        aPointer = Pointer( POINTER_SESIZE );
        break;

    case SIZING_RIGHT+SIZING_TOP:
    case SIZING_LEFT+SIZING_BOTTOM:
        aPointer = Pointer( POINTER_NESIZE );
        break;
    }

    SetPointer( aPointer );
}

//------------------------------------------------------------------------------
void OTableWindow::MouseButtonDown( const MouseEvent& rEvt )
{
    //////////////////////////////////////////////////////////////////////
    // Wenn sizing, dann bekommt Parent die Nachricht,
    // dass jetzt die Fenstergroesse seines Childs veraendert wird
    if( m_nSizingFlags )
        getTableView()->BeginChildSizing( this, GetPointer() );

    Window::MouseButtonDown( rEvt );
}



//------------------------------------------------------------------------------
void OTableWindow::Resize()
{
    //////////////////////////////////////////////////////////////////////
    // Modify Flag des Documents nicht beim ersten Resize setzen
    if( IsVisible() )
    {
        OQueryTableView* pTabWinCont = getTableView();
//      pTabWinCont->GetViewShell()->GetShell->SetModified( TRUE );
    }

    //////////////////////////////////////////////////////////////////////
    // Das Fenster darf nicht verschwinden, deshalb min. Groesse setzen
    Size    aOutSize = GetOutputSizePixel();
    aOutSize = Size(CalcZoom(aOutSize.Width()),CalcZoom(aOutSize.Height()));
    Point   aPos = GetPosPixel();

    long nCharHeight = CalcZoom(GetTextHeight())+ CalcZoom(4);
//  if( aOutSize.Width() < TABWIN_WIDTH_MIN )
//  {
//      aOutSize.Width() = TABWIN_WIDTH_MIN;
//      bChanged = sal_True;
//  }
//  if( aOutSize.Height() < TABWIN_HEIGHT_MIN )
//  {
//      aOutSize.Height() = TABWIN_HEIGHT_MIN;
//      nCharHeight = GetTextHeight() + 4;
//      bChanged = sal_True;
//  }
//  if(bChanged)
//  {
//      SetPosSizePixel( aPos, aOutSize );
//      m_pData->SetSize( aOutSize );
//  }

    //////////////////////////////////////////////////////////////////////
    // Titel und ListBox anpassen
    //  long nCharHeight = GetTextHeight() + 4;
    long n5Pos = CalcZoom(5);
    long n10Pos = CalcZoom(10);
    m_aTitle.SetPosSizePixel( Point(n5Pos,n5Pos), Size( aOutSize.Width()-n10Pos, nCharHeight) );
    m_pListBox->SetPosSizePixel( Point(n5Pos,nCharHeight+n5Pos), Size(aOutSize.Width()-n10Pos,aOutSize.Height()-nCharHeight-n10Pos) );

    Window::Invalidate();
}

//------------------------------------------------------------------------------
void OTableWindow::SetBoldTitle( BOOL bBold )
{
    Font aFont = m_aTitle.GetFont();
    aFont.SetWeight( bBold?WEIGHT_BOLD:WEIGHT_NORMAL );
    m_aTitle.SetFont( aFont );
    m_aTitle.Invalidate();
}

//------------------------------------------------------------------------------
void OTableWindow::GetFocus()
{
    SetBoldTitle( TRUE );
    m_bActive = TRUE;
}

//------------------------------------------------------------------------------
void OTableWindow::LoseFocus()
{
    SetBoldTitle( FALSE );
    m_bActive = FALSE;
}

//------------------------------------------------------------------------------
void OTableWindow::Remove()
{
    //////////////////////////////////////////////////////////////////
    // Fenster loeschen
    OQueryTableView* pTabWinCont = getTableView();
    pTabWinCont->RemoveTabWin( this );
    pTabWinCont->Invalidate();
}

//------------------------------------------------------------------------------
void OTableWindow::KeyInput( const KeyEvent& rEvt )
{
    USHORT nCode = rEvt.GetKeyCode().GetCode();
    BOOL   bShift = rEvt.GetKeyCode().IsShift();
    BOOL   bCtrl = rEvt.GetKeyCode().IsMod1();

    if( !bCtrl && !bShift && (nCode==KEY_DELETE) )
    {
        Remove();
    }
    else
        Window::KeyInput( rEvt );
}

//------------------------------------------------------------------------------
void OTableWindow::InvalidateLines()
{
    getTableView()->InvalidateConnections();
}

//------------------------------------------------------------------------------
BOOL OTableWindow::ExistsAConn() const
{
    return getTableView()->ExistsAConn(this);
}
//------------------------------------------------------------------------------
void OTableWindow::EnumValidFields(::std::vector< ::rtl::OUString>& arrstrFields)
{
    arrstrFields.clear();
    // diese Default-Implementierung zaehlt einfach alles auf, was es in der ListBox gibt ... fuer anderes Verhalten ueberschreiben
    if (!m_pListBox)
        return;

    SvLBoxEntry* pEntryLoop = m_pListBox->First();
    while (pEntryLoop)
    {
        arrstrFields.push_back(m_pListBox->GetEntryText(pEntryLoop));
        pEntryLoop = m_pListBox->Next(pEntryLoop);
    }
}
// -----------------------------------------------------------------------------
void OTableWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_ZOOM )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );

        m_aTitle.SetZoom(GetZoom());
        m_pListBox->SetZoom(GetZoom());
        Resize();
        Invalidate();
    }
}
// -----------------------------------------------------------------------------



