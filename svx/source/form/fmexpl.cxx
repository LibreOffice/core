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
#include <svx/svdobjkind.hxx>
#include <svx/fmtools.hxx>
#include <fmexch.hxx>

#include <svx/svxids.hrc>

#include <fmprop.hxx>
#include <bitmaps.hlst>
#include <svx/dialmgr.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sfx2/objitem.hxx>

#include <svx/fmshell.hxx>
#include <comphelper/types.hxx>
#include <utility>

using namespace ::svxform;
using namespace ::com::sun::star::uno;
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

FmNavNameChangedHint::FmNavNameChangedHint( FmEntryData* pData, OUString _aNewName )
    :pEntryData( pData )
    ,aNewName(std::move( _aNewName ))
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


void FmEntryDataList::removeNoDelete( FmEntryData* pItem )
{
    auto it = std::find_if(maEntryDataList.begin(), maEntryDataList.end(),
        [&pItem](const std::unique_ptr<FmEntryData>& rEntryData) { return rEntryData.get() == pItem; });
    if (it != maEntryDataList.end())
    {
        // coverity[leaked_storage] - deliberately not deleted, ownership transferred
        it->release();
        maEntryDataList.erase( it );
        return;
    }
    assert(false);
}


void FmEntryDataList::insert( std::unique_ptr<FmEntryData> pItem, size_t Index )
{
    if ( Index < maEntryDataList.size() )
    {
        maEntryDataList.insert( maEntryDataList.begin() + Index, std::move(pItem) );
    }
    else
        maEntryDataList.push_back( std::move(pItem) );
}


void FmEntryDataList::clear()
{
    maEntryDataList.clear();
}


FmEntryData::FmEntryData( FmEntryData* pParentData, const Reference< XInterface >& _rxIFace )
    :pParent( pParentData )
{
    pChildList.reset( new FmEntryDataList() );

    newObject( _rxIFace );
}


FmEntryData::~FmEntryData()
{
    pChildList->clear();
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
    pChildList.reset( new FmEntryDataList() );
    aText = rEntryData.GetText();
    m_aNormalImage = rEntryData.GetNormalImage();
    pParent = rEntryData.GetParent();

    FmEntryData* pChildData;
    size_t nEntryCount = rEntryData.GetChildList()->size();
    for( size_t i = 0; i < nEntryCount; i++ )
    {
        pChildData = rEntryData.GetChildList()->at( i );
        std::unique_ptr<FmEntryData> pNewChildData = pChildData->Clone();
        pChildList->insert( std::move(pNewChildData), size_t(-1) );
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
    m_aNormalImage = RID_SVXBMP_FORM;

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


std::unique_ptr<FmEntryData> FmFormData::Clone()
{
    return std::unique_ptr<FmEntryData>(new FmFormData( *this ));
}


bool FmFormData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;
    FmFormData* pFormData = dynamic_cast<FmFormData*>(pEntryData);
    if( !pFormData )
        return false;
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


std::unique_ptr<FmEntryData> FmControlData::Clone()
{
    return std::unique_ptr<FmEntryData>(new FmControlData( *this ));
}


OUString FmControlData::GetImage() const
{
    // Default-Image
    OUString aImage(RID_SVXBMP_CONTROL);

    Reference< XServiceInfo > xInfo( m_xFormComponent, UNO_QUERY );
    if (!m_xFormComponent.is())
        return aImage;


    // Spezielle Control-Images
    SdrObjKind nObjectType = getControlTypeByObject(xInfo);
    switch (nObjectType)
    {
    case SdrObjKind::FormButton:
        aImage = RID_SVXBMP_BUTTON;
        break;

    case SdrObjKind::FormFixedText:
        aImage = RID_SVXBMP_FIXEDTEXT;
        break;

    case SdrObjKind::FormEdit:
        aImage = RID_SVXBMP_EDITBOX;
        break;

    case SdrObjKind::FormRadioButton:
        aImage = RID_SVXBMP_RADIOBUTTON;
        break;

    case SdrObjKind::FormCheckbox:
        aImage = RID_SVXBMP_CHECKBOX;
        break;

    case SdrObjKind::FormListbox:
        aImage = RID_SVXBMP_LISTBOX;
        break;

    case SdrObjKind::FormCombobox:
        aImage = RID_SVXBMP_COMBOBOX;
        break;

    case SdrObjKind::FormNavigationBar:
        aImage = RID_SVXBMP_NAVIGATIONBAR;
        break;

    case SdrObjKind::FormGroupBox:
        aImage = RID_SVXBMP_GROUPBOX;
        break;

    case SdrObjKind::FormImageButton:
        aImage = RID_SVXBMP_IMAGEBUTTON;
        break;

    case SdrObjKind::FormFileControl:
        aImage = RID_SVXBMP_FILECONTROL;
        break;

    case SdrObjKind::FormHidden:
        aImage = RID_SVXBMP_HIDDEN;
        break;

    case SdrObjKind::FormDateField:
        aImage = RID_SVXBMP_DATEFIELD;
        break;

    case SdrObjKind::FormTimeField:
        aImage = RID_SVXBMP_TIMEFIELD;
        break;

    case SdrObjKind::FormNumericField:
        aImage = RID_SVXBMP_NUMERICFIELD;
        break;

    case SdrObjKind::FormCurrencyField:
        aImage = RID_SVXBMP_CURRENCYFIELD;
        break;

    case SdrObjKind::FormPatternField:
        aImage = RID_SVXBMP_PATTERNFIELD;
        break;

    case SdrObjKind::FormImageControl:
        aImage = RID_SVXBMP_IMAGECONTROL;
        break;

    case SdrObjKind::FormFormattedField:
        aImage = RID_SVXBMP_FORMATTEDFIELD;
        break;

    case SdrObjKind::FormGrid:
        aImage = RID_SVXBMP_GRID;
        break;

    case SdrObjKind::FormScrollbar:
        aImage = RID_SVXBMP_SCROLLBAR;
        break;

    case SdrObjKind::FormSpinButton:
        aImage = RID_SVXBMP_SPINBUTTON;
        break;

    default:;
    }

    return aImage;
}

bool FmControlData::IsEqualWithoutChildren( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return true;

    FmControlData* pControlData = dynamic_cast<FmControlData*>(pEntryData);
    if( !pControlData )
        return false;

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
      : SfxDockingWindow(_pBindings, _pMgr, _pParent, u"FormNavigator"_ustr, u"svx/ui/formnavigator.ui"_ustr)
      , SfxControllerItem( SID_FM_FMEXPLORER_CONTROL, *_pBindings )
      , m_xNavigatorTree(new NavigatorTree(m_xBuilder->weld_tree_view(u"treeview"_ustr)))
    {
        SetHelpId( HID_FORM_NAVIGATOR_WIN );

        SetText( SvxResId(RID_STR_FMEXPLORER) );
        SfxDockingWindow::SetFloatingSize( Size(200,200) );
    }

    NavigatorFrame::~NavigatorFrame()
    {
        disposeOnce();
    }

    void NavigatorFrame::dispose()
    {
        m_xNavigatorTree.reset();
        ::SfxControllerItem::dispose();
        SfxDockingWindow::dispose();
    }

    void NavigatorFrame::UpdateContent( FmFormShell* pFormShell )
    {
        m_xNavigatorTree->UpdateContent(pFormShell);
    }

    void NavigatorFrame::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
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
        if (m_xNavigatorTree )
            m_xNavigatorTree->GrabFocus();
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
