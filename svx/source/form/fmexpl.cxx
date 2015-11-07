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


#include "svx/fmresids.hrc"
#include "fmexpl.hxx"

#include "fmhelp.hrc"
#include <svx/fmglob.hxx>
#include "fmservs.hxx"
#include <svx/fmmodel.hxx>
#include "fmexch.hxx"
#include "fmundo.hxx"
#include "fmpgeimp.hxx"

#include <svx/svxids.hrc>

#include "fmprop.hrc"
#include <svx/dialmgr.hxx>
#include "svx/svditer.hxx"
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
#include "fmshimp.hxx"
#include <svx/fmpage.hxx>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

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
    for ( FmEntryDataBaseList::iterator it = maEntryDataList.begin();
          it != maEntryDataList.end();
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
    for ( size_t i = 0, n = maEntryDataList.size(); i < n; ++i )
        delete maEntryDataList[ i ];
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
    Clear();
    delete pChildList;
}


void FmEntryData::newObject( const css::uno::Reference< css::uno::XInterface >& _rxIFace )
{
    // do not just copy, normalize it
    m_xNormalizedIFace = Reference< XInterface >( _rxIFace, UNO_QUERY );
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


void FmEntryData::Clear()
{
    GetChildList()->clear();
}


bool FmEntryData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;

    if( !pEntryData )
        return false;

    if( !aText.equals(pEntryData->GetText()))
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


FmFormData::FmFormData(
    const Reference< XForm >& _rxForm,
    const ImageList& _rNormalImages,
    FmFormData* _pParent
)
:   FmEntryData( _pParent, _rxForm ),
    m_xForm( _rxForm )
{

    // Images setzen

    m_aNormalImage = _rNormalImages.GetImage( RID_SVXIMG_FORM );


    // Titel setzen
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


FmControlData::FmControlData(
    const Reference< XFormComponent >& _rxComponent,
    const ImageList& _rNormalImages,
    FmFormData* _pParent
)
:   FmEntryData( _pParent, _rxComponent ),
    m_xFormComponent( _rxComponent )
{

    // Images setzen
    m_aNormalImage = GetImage( _rNormalImages );


    // Titel setzen
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


Image FmControlData::GetImage(const ImageList& ilNavigatorImages) const
{

    // Default-Image
    Image aImage = ilNavigatorImages.GetImage( RID_SVXIMG_CONTROL );

    Reference< XServiceInfo > xInfo( m_xFormComponent, UNO_QUERY );
    if (!m_xFormComponent.is())
        return aImage;


    // Spezielle Control-Images
    sal_Int16 nObjectType = getControlTypeByObject(xInfo);
    switch (nObjectType)
    {
    case OBJ_FM_BUTTON:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_BUTTON );
        break;

    case OBJ_FM_FIXEDTEXT:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_FIXEDTEXT );
        break;

    case OBJ_FM_EDIT:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_EDIT );
        break;

    case OBJ_FM_RADIOBUTTON:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_RADIOBUTTON );
        break;

    case OBJ_FM_CHECKBOX:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_CHECKBOX );
        break;

    case OBJ_FM_LISTBOX:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_LISTBOX );
        break;

    case OBJ_FM_COMBOBOX:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_COMBOBOX );
        break;

    case OBJ_FM_NAVIGATIONBAR:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_NAVIGATIONBAR );
        break;

    case OBJ_FM_GROUPBOX:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_GROUPBOX );
        break;

    case OBJ_FM_IMAGEBUTTON:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_IMAGEBUTTON );
        break;

    case OBJ_FM_FILECONTROL:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_FILECONTROL );
        break;

    case OBJ_FM_HIDDEN:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_HIDDEN );
        break;

    case OBJ_FM_DATEFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_DATEFIELD );
        break;

    case OBJ_FM_TIMEFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_TIMEFIELD );
        break;

    case OBJ_FM_NUMERICFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_NUMERICFIELD );
        break;

    case OBJ_FM_CURRENCYFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_CURRENCYFIELD );
        break;

    case OBJ_FM_PATTERNFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_PATTERNFIELD );
        break;

    case OBJ_FM_IMAGECONTROL:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_IMAGECONTROL );
        break;

    case OBJ_FM_FORMATTEDFIELD:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_FORMATTEDFIELD );
        break;

    case OBJ_FM_GRID:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_GRID );
        break;

    case OBJ_FM_SCROLLBAR:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_SCROLLBAR );
        break;

    case OBJ_FM_SPINBUTTON:
        aImage = ilNavigatorImages.GetImage( RID_SVXIMG_SPINBUTTON);
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


void FmControlData::ModelReplaced(
    const Reference< XFormComponent >& _rxNew,
    const ImageList& _rNormalImages
)
{
    m_xFormComponent = _rxNew;
    newObject( m_xFormComponent );

    // Images neu setzen
    m_aNormalImage = GetImage( _rNormalImages );
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
        SetText( SVX_RES(RID_STR_FMEXPLORER) );
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
            UpdateContent( NULL );
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
        UpdateContent( NULL );
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

        Size aLogOutputSize = PixelToLogic( GetOutputSizePixel(), MAP_APPFONT );
        Size aLogExplSize = aLogOutputSize;
        aLogExplSize.Width() -= 6;
        aLogExplSize.Height() -= 6;

        Point aExplPos = LogicToPixel( Point(3,3), MAP_APPFONT );
        Size aExplSize = LogicToPixel( aLogExplSize, MAP_APPFONT );

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
