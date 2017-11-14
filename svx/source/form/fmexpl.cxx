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


#include <svx/strings.hrc>
#include <fmexpl.hxx>

#include <helpids.h>
#include <svx/fmglob.hxx>
#include <fmservs.hxx>
#include <svx/fmmodel.hxx>
#include <fmexch.hxx>
#include <fmundo.hxx>
#include <fmpgeimp.hxx>

#include <svx/svxids.hrc>

#include <fmprop.hxx>
#include <bitmaps.hlst>
#include <svx/dialmgr.hxx>
#include <svx/svditer.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdobj.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <vcl/menu.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/request.hxx>

#include <vcl/wrkwin.hxx>
#include <svx/fmshell.hxx>
#include <fmshimp.hxx>
#include <svx/fmpage.hxx>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::svxform;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

FmNavInsertedHint::FmNavInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos )
    :pEntryData( pInsertedEntryData )
    ,nPos( nRelPos )

{
}


FmNavInsertedHint::~FmNavInsertedHint()
{
}


// class FmNavInsertedHint

FmNavModelReplacedHint::FmNavModelReplacedHint( FmEntryData* pAffectedEntryData )
    :pEntryData( pAffectedEntryData )
{
}


FmNavModelReplacedHint::~FmNavModelReplacedHint()
{
}

FmNavRemovedHint::FmNavRemovedHint( FmEntryData* pRemovedEntryData )
    :pEntryData( pRemovedEntryData )
{
}


FmNavRemovedHint::~FmNavRemovedHint()
{
}

FmNavNameChangedHint::FmNavNameChangedHint( FmEntryData* pData, const OUString& rNewName )
    :pEntryData( pData )
    ,aNewName( rNewName )
{
}


FmNavNameChangedHint::~FmNavNameChangedHint()
{
}

FmNavClearedHint::FmNavClearedHint()
{
}


FmNavClearedHint::~FmNavClearedHint()
{
}


FmEntryDataList::FmEntryDataList()
{
}


FmEntryDataList::~FmEntryDataList()
{
}


FmEntryData* FmEntryDataList::remove( FmEntryData* pItem )
{
    FmEntryDataBaseList::const_iterator aEnd = maEntryDataList.end();
    for ( FmEntryDataBaseList::iterator it = maEntryDataList.begin();
          it != aEnd;
          ++it
        )
    {
        if ( *it == pItem )
        {
            maEntryDataList.erase( it );
            break;
        }
    }
    return pItem;
}


void FmEntryDataList::insert( FmEntryData* pItem, size_t Index )
{
    if ( Index < maEntryDataList.size() )
    {
        FmEntryDataBaseList::iterator it = maEntryDataList.begin();
        ::std::advance( it, Index );
        maEntryDataList.insert( it, pItem );
    }
    else
        maEntryDataList.push_back( pItem );
}


void FmEntryDataList::clear()
{
    for (FmEntryData* p : maEntryDataList)
        delete p;
    maEntryDataList.clear();
}


FmEntryData::FmEntryData( FmEntryData* pParentData, const Reference< XInterface >& _rxIFace )
    :pParent( pParentData )
{
    pChildList = new FmEntryDataList();

    newObject( _rxIFace );
}


FmEntryData::~FmEntryData()
{
    GetChildList()->clear();
    delete pChildList;
}


void FmEntryData::newObject( const css::uno::Reference< css::uno::XInterface >& _rxIFace )
{
    // do not just copy, normalize it
    m_xNormalizedIFace.set( _rxIFace, UNO_QUERY );
    m_xProperties.set(m_xNormalizedIFace, css::uno::UNO_QUERY);
    m_xChild.set(m_xNormalizedIFace, css::uno::UNO_QUERY);
}


FmEntryData::FmEntryData( const FmEntryData& rEntryData )
{
    pChildList = new FmEntryDataList();
    aText = rEntryData.GetText();
    m_aNormalImage = rEntryData.GetNormalImage();
    pParent = rEntryData.GetParent();

    FmEntryData* pChildData;
    size_t nEntryCount = rEntryData.GetChildList()->size();
    for( size_t i = 0; i < nEntryCount; i++ )
    {
        pChildData = rEntryData.GetChildList()->at( i );
        FmEntryData* pNewChildData = pChildData->Clone();
        pChildList->insert( pNewChildData, size_t(-1) );
    }

    m_xNormalizedIFace = rEntryData.m_xNormalizedIFace;
    m_xProperties = rEntryData.m_xProperties;
    m_xChild = rEntryData.m_xChild;
}



bool FmEntryData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;

    if( !pEntryData )
        return false;

    if( aText != pEntryData->GetText() )
        return false;

    if( !pEntryData->GetParent() && pParent )
        return false;

    if( pEntryData->GetParent() && !pParent )
        return false;

    if( !pEntryData->GetParent() && !pParent )
        return true;

    if( !pParent->IsEqualWithoutChildren(pEntryData->GetParent()) )
        return false;

    return true;
}

FmFormData::FmFormData(const Reference< XForm >& _rxForm, FmFormData* _pParent)
    : FmEntryData(_pParent, _rxForm)
    , m_xForm(_rxForm)
{
    // set images
    m_aNormalImage = Image(RID_SVXBMP_FORM);

    // set title
    if (m_xForm.is())
    {
        Reference< XPropertySet >  xSet(m_xForm, UNO_QUERY);
        if (xSet.is())
        {
            OUString aEntryName(::comphelper::getString(xSet->getPropertyValue( FM_PROP_NAME )));
            SetText(aEntryName);
        }
    }
    else
        SetText( OUString() );
}

FmFormData::~FmFormData()
{
}

FmFormData::FmFormData( const FmFormData& rFormData )
    :FmEntryData( rFormData )
{
    m_xForm = rFormData.GetFormIface();
}


FmEntryData* FmFormData::Clone()
{
    return new FmFormData( *this );
}


bool FmFormData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;
    if( dynamic_cast<const FmFormData*>( pEntryData) ==  nullptr )
        return false;
    FmFormData* pFormData = static_cast<FmFormData*>(pEntryData);
    if( m_xForm.get() != pFormData->GetFormIface().get() )
        return false;

    return FmEntryData::IsEqualWithoutChildren( pFormData );
}

FmControlData::FmControlData(const Reference< XFormComponent >& _rxComponent, FmFormData* _pParent)
:   FmEntryData( _pParent, _rxComponent ),
    m_xFormComponent( _rxComponent )
{

    // set images
    m_aNormalImage = GetImage();


    // set title
    Reference< XPropertySet >  xSet(m_xFormComponent, UNO_QUERY);
    if( xSet.is() )
    {
        SetText( ::comphelper::getString(xSet->getPropertyValue( FM_PROP_NAME )));
    }
}


FmControlData::~FmControlData()
{
}


FmControlData::FmControlData( const FmControlData& rControlData )
    :FmEntryData( rControlData )
{
    m_xFormComponent = rControlData.GetFormComponent();
}


FmEntryData* FmControlData::Clone()
{
    return new FmControlData( *this );
}


Image FmControlData::GetImage() const
{
    // Default-Image
    Image aImage(BitmapEx(RID_SVXBMP_CONTROL));

    Reference< XServiceInfo > xInfo( m_xFormComponent, UNO_QUERY );
    if (!m_xFormComponent.is())
        return aImage;


    // Spezielle Control-Images
    sal_Int16 nObjectType = getControlTypeByObject(xInfo);
    switch (nObjectType)
    {
    case OBJ_FM_BUTTON:
        aImage = Image(BitmapEx(RID_SVXBMP_BUTTON));
        break;

    case OBJ_FM_FIXEDTEXT:
        aImage = Image(BitmapEx(RID_SVXBMP_FIXEDTEXT));
        break;

    case OBJ_FM_EDIT:
        aImage = Image(BitmapEx(RID_SVXBMP_EDITBOX));
        break;

    case OBJ_FM_RADIOBUTTON:
        aImage = Image(BitmapEx(RID_SVXBMP_RADIOBUTTON));
        break;

    case OBJ_FM_CHECKBOX:
        aImage = Image(BitmapEx(RID_SVXBMP_CHECKBOX));
        break;

    case OBJ_FM_LISTBOX:
        aImage = Image(BitmapEx(RID_SVXBMP_LISTBOX));
        break;

    case OBJ_FM_COMBOBOX:
        aImage = Image(BitmapEx(RID_SVXBMP_COMBOBOX));
        break;

    case OBJ_FM_NAVIGATIONBAR:
        aImage = Image(BitmapEx(RID_SVXBMP_NAVIGATIONBAR));
        break;

    case OBJ_FM_GROUPBOX:
        aImage = Image(BitmapEx(RID_SVXBMP_GROUPBOX));
        break;

    case OBJ_FM_IMAGEBUTTON:
        aImage = Image(BitmapEx(RID_SVXBMP_IMAGEBUTTON));
        break;

    case OBJ_FM_FILECONTROL:
        aImage = Image(BitmapEx(RID_SVXBMP_FILECONTROL));
        break;

    case OBJ_FM_HIDDEN:
        aImage = Image(BitmapEx(RID_SVXBMP_HIDDEN));
        break;

    case OBJ_FM_DATEFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_DATEFIELD));
        break;

    case OBJ_FM_TIMEFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_TIMEFIELD));
        break;

    case OBJ_FM_NUMERICFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_NUMERICFIELD));
        break;

    case OBJ_FM_CURRENCYFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_CURRENCYFIELD));
        break;

    case OBJ_FM_PATTERNFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_PATTERNFIELD));
        break;

    case OBJ_FM_IMAGECONTROL:
        aImage = Image(BitmapEx(RID_SVXBMP_IMAGECONTROL));
        break;

    case OBJ_FM_FORMATTEDFIELD:
        aImage = Image(BitmapEx(RID_SVXBMP_FORMATTEDFIELD));
        break;

    case OBJ_FM_GRID:
        aImage = Image(BitmapEx(RID_SVXBMP_GRID));
        break;

    case OBJ_FM_SCROLLBAR:
        aImage = Image(BitmapEx(RID_SVXBMP_SCROLLBAR));
        break;

    case OBJ_FM_SPINBUTTON:
        aImage = Image(BitmapEx(RID_SVXBMP_SPINBUTTON));
        break;
    }

    return aImage;
}

bool FmControlData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;

    if( dynamic_cast<const FmControlData*>( pEntryData) ==  nullptr )
        return false;
    FmControlData* pControlData = static_cast<FmControlData*>(pEntryData);

    if( m_xFormComponent.get() != pControlData->GetFormComponent().get() )
        return false;

    return FmEntryData::IsEqualWithoutChildren( pControlData );
}

void FmControlData::ModelReplaced(const Reference< XFormComponent >& _rxNew)
{
    m_xFormComponent = _rxNew;
    newObject( m_xFormComponent );
    // set images anew
    m_aNormalImage = GetImage();
}

namespace svxform
{

    NavigatorFrame::NavigatorFrame( SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                                  vcl::Window* _pParent )
      :SfxDockingWindow( _pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
      ,SfxControllerItem( SID_FM_FMEXPLORER_CONTROL, *_pBindings )
    {
        SetHelpId( HID_FORM_NAVIGATOR_WIN );

        m_pNavigatorTree = VclPtr<NavigatorTree>::Create( this );
        m_pNavigatorTree->Show();
        SetText( SvxResId(RID_STR_FMEXPLORER) );
        SfxDockingWindow::SetFloatingSize( Size(200,200) );
    }


    NavigatorFrame::~NavigatorFrame()
    {
        disposeOnce();
    }

    void NavigatorFrame::dispose()
    {
        m_pNavigatorTree.disposeAndClear();
        ::SfxControllerItem::dispose();
        SfxDockingWindow::dispose();
    }


    void NavigatorFrame::UpdateContent( FmFormShell* pFormShell )
    {
        m_pNavigatorTree->UpdateContent( pFormShell );
    }


    void NavigatorFrame::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
    {
        if( !pState  || SID_FM_FMEXPLORER_CONTROL != nSID )
            return;

        if( eState >= SfxItemState::DEFAULT )
        {
            FmFormShell* pShell = dynamic_cast<FmFormShell*>( static_cast<const SfxObjectItem*>(pState)->GetShell()  );
            UpdateContent( pShell );
        }
        else
            UpdateContent( nullptr );
    }


    void NavigatorFrame::GetFocus()
    {
        if ( m_pNavigatorTree )
            m_pNavigatorTree->GrabFocus();
        else
            SfxDockingWindow::GetFocus();
    }


    bool NavigatorFrame::Close()
    {
        UpdateContent( nullptr );
        return SfxDockingWindow::Close();
    }


    void NavigatorFrame::FillInfo( SfxChildWinInfo& rInfo ) const
    {
        SfxDockingWindow::FillInfo( rInfo );
        rInfo.bVisible = false;
    }


    Size NavigatorFrame::CalcDockingSize( SfxChildAlignment eAlign )
    {
        if ( ( eAlign == SfxChildAlignment::TOP ) || ( eAlign == SfxChildAlignment::BOTTOM ) )
            return Size();

        return SfxDockingWindow::CalcDockingSize( eAlign );
    }


    SfxChildAlignment NavigatorFrame::CheckAlignment( SfxChildAlignment _eActAlign, SfxChildAlignment _eAlign )
    {
        if ( ( _eAlign == SfxChildAlignment::LEFT ) || ( _eAlign == SfxChildAlignment::RIGHT ) || ( _eAlign == SfxChildAlignment::NOALIGNMENT ) )
            return _eAlign;
        return _eActAlign;
    }


    void NavigatorFrame::Resize()
    {
        SfxDockingWindow::Resize();

        Size aLogOutputSize = PixelToLogic(GetOutputSizePixel(), MapMode(MapUnit::MapAppFont));
        Size aLogExplSize = aLogOutputSize;
        aLogExplSize.Width() -= 6;
        aLogExplSize.Height() -= 6;

        Point aExplPos = LogicToPixel(Point(3, 3), MapMode(MapUnit::MapAppFont));
        Size aExplSize = LogicToPixel(aLogExplSize, MapMode(MapUnit::MapAppFont));

        m_pNavigatorTree->SetPosSizePixel( aExplPos, aExplSize );
    }


    // class NavigatorFrameManager


    SFX_IMPL_DOCKINGWINDOW( NavigatorFrameManager, SID_FM_SHOW_FMEXPLORER )


    NavigatorFrameManager::NavigatorFrameManager( vcl::Window* _pParent, sal_uInt16 _nId,
                                        SfxBindings* _pBindings, SfxChildWinInfo* _pInfo )
                     :SfxChildWindow( _pParent, _nId )
    {
        SetWindow( VclPtr<NavigatorFrame>::Create( _pBindings, this, _pParent ) );
        static_cast<SfxDockingWindow*>(GetWindow())->Initialize( _pInfo );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
