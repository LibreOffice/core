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
#include "precompiled_svx.hxx"

#ifndef _SVX_FMRESIDS_HRC
#include "svx/fmresids.hrc"
#endif
#ifndef _SVX_FMEXPL_HRC
#include "fmexpl.hrc"
#endif
#include "fmexpl.hxx"

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#include <svx/fmglob.hxx>
#include "fmservs.hxx"
#include <svx/fmmodel.hxx>
#include "fmexch.hxx"
#include "fmundo.hxx"
#include "fmpgeimp.hxx"

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#include <svx/dialmgr.hxx>
#include <svx/svditer.hxx>
#include <svx/svdouno.hxx>
#include <fmundo.hxx>
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
#include <tools/shl.hxx>

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <vcl/sound.hxx>
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
#include <rtl/logfile.hxx>

using namespace ::svxform;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

//========================================================================

SV_IMPL_PTRARR_SORT( FmEntryDataArray, FmEntryDataPtr )
SV_IMPL_PTRARR_SORT( SvLBoxEntrySortedArray, SvLBoxEntryPtr )

//========================================================================
// class FmNavInsertedHint
//========================================================================
//------------------------------------------------------------------------
FmNavInsertedHint::FmNavInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos )
    :pEntryData( pInsertedEntryData )
    ,nPos( nRelPos )

{
}

//------------------------------------------------------------------------
FmNavInsertedHint::~FmNavInsertedHint()
{
}


//========================================================================
// class FmNavInsertedHint
//========================================================================
//------------------------------------------------------------------------
FmNavModelReplacedHint::FmNavModelReplacedHint( FmEntryData* pAffectedEntryData )
    :pEntryData( pAffectedEntryData )
{
}

//------------------------------------------------------------------------
FmNavModelReplacedHint::~FmNavModelReplacedHint()
{
}

//========================================================================
// class FmNavRemovedHint
//========================================================================
//------------------------------------------------------------------------
FmNavRemovedHint::FmNavRemovedHint( FmEntryData* pRemovedEntryData )
    :pEntryData( pRemovedEntryData )
{
}

//------------------------------------------------------------------------
FmNavRemovedHint::~FmNavRemovedHint()
{
}


//========================================================================
// class FmNavNameChangedHint
//========================================================================
//------------------------------------------------------------------------
FmNavNameChangedHint::FmNavNameChangedHint( FmEntryData* pData, const ::rtl::OUString& rNewName )
    :pEntryData( pData )
    ,aNewName( rNewName )
{
}

//------------------------------------------------------------------------
FmNavNameChangedHint::~FmNavNameChangedHint()
{
}

//========================================================================
// class FmNavClearedHint
//========================================================================
//------------------------------------------------------------------------
FmNavClearedHint::FmNavClearedHint()
{
}

//------------------------------------------------------------------------
FmNavClearedHint::~FmNavClearedHint()
{
}

//========================================================================
// class FmNavRequestSelectHint
//========================================================================

//========================================================================
// class FmNavViewMarksChanged
//========================================================================

//========================================================================
// class FmEntryDataList
//========================================================================
//------------------------------------------------------------------------
FmEntryDataList::FmEntryDataList()
{
}

//------------------------------------------------------------------------
FmEntryDataList::~FmEntryDataList()
{
}


//========================================================================
// class FmEntryData
//========================================================================
//------------------------------------------------------------------------
FmEntryData::FmEntryData( FmEntryData* pParentData, const Reference< XInterface >& _rxIFace )
    :pParent( pParentData )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmEntryData::FmEntryData" );
    pChildList = new FmEntryDataList();

    newObject( _rxIFace );
}

//------------------------------------------------------------------------
FmEntryData::~FmEntryData()
{
    Clear();
    delete pChildList;
}

//------------------------------------------------------------------------
void FmEntryData::newObject( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmEntryData::newObject" );
    // do not just copy, normalize it
    m_xNormalizedIFace = Reference< XInterface >( _rxIFace, UNO_QUERY );
    m_xProperties = m_xProperties.query( m_xNormalizedIFace );
    m_xChild = m_xChild.query( m_xNormalizedIFace );
}

//------------------------------------------------------------------------
FmEntryData::FmEntryData( const FmEntryData& rEntryData )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmEntryData::FmEntryData" );
    pChildList = new FmEntryDataList();
    aText = rEntryData.GetText();
    m_aNormalImage = rEntryData.GetNormalImage();
    m_aHCImage = rEntryData.GetHCImage();
    pParent = rEntryData.GetParent();

    FmEntryData* pChildData;
    sal_uInt32 nEntryCount = rEntryData.GetChildList()->Count();
    for( sal_uInt32 i=0; i<nEntryCount; i++ )
    {
        pChildData = rEntryData.GetChildList()->GetObject(i);
        FmEntryData* pNewChildData = pChildData->Clone();
        pChildList->Insert( pNewChildData, LIST_APPEND );
    }

    m_xNormalizedIFace = rEntryData.m_xNormalizedIFace;
    m_xProperties = rEntryData.m_xProperties;
    m_xChild = rEntryData.m_xChild;
}

//------------------------------------------------------------------------
void FmEntryData::Clear()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmEntryData::Clear" );
    for (;;)
    {
        FmEntryData* pEntryData = GetChildList()->Remove(sal_uLong(0));
        if (pEntryData == NULL)
            break;
        delete pEntryData;
    }
}

//------------------------------------------------------------------------
sal_Bool FmEntryData::IsEqualWithoutChilds( FmEntryData* pEntryData )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmEntryData::IsEqualWithoutChilds" );
    if(this == pEntryData)
        return sal_True;

    if( !pEntryData )
        return sal_False;

    if( !aText.equals(pEntryData->GetText()))
        return sal_False;

    if( !pEntryData->GetParent() && pParent )
        return sal_False;

    if( pEntryData->GetParent() && !pParent )
        return sal_False;

    if( !pEntryData->GetParent() && !pParent )
        return sal_True;

    if( !pParent->IsEqualWithoutChilds(pEntryData->GetParent()) )
        return sal_False;

    return sal_True;
}


//========================================================================
// class FmFormData
//========================================================================
//------------------------------------------------------------------------
FmFormData::FmFormData( const Reference< XForm >& _rxForm, const ImageList& _rNormalImages, const ImageList& _rHCImages, FmFormData* _pParent )
    :FmEntryData( _pParent, _rxForm )
    ,m_xForm( _rxForm )
{
    //////////////////////////////////////////////////////////////////////
    // Images setzen

    m_aNormalImage = _rNormalImages.GetImage( RID_SVXIMG_FORM );
    m_aHCImage = _rHCImages.GetImage( RID_SVXIMG_FORM );

    //////////////////////////////////////////////////////////////////////
    // Titel setzen
    if (m_xForm.is())
    {
        Reference< XPropertySet >  xSet(m_xForm, UNO_QUERY);
        if (xSet.is())
        {
            ::rtl::OUString aEntryName(::comphelper::getString(xSet->getPropertyValue( FM_PROP_NAME )));
            SetText(aEntryName);
        }
    }
    else
        SetText( ::rtl::OUString() );
}

//------------------------------------------------------------------------
FmFormData::~FmFormData()
{
}

//------------------------------------------------------------------------
FmFormData::FmFormData( const FmFormData& rFormData )
    :FmEntryData( rFormData )
{
    m_xForm = rFormData.GetFormIface();
}

//------------------------------------------------------------------------
FmEntryData* FmFormData::Clone()
{
    return new FmFormData( *this );
}

//------------------------------------------------------------------------
sal_Bool FmFormData::IsEqualWithoutChilds( FmEntryData* pEntryData )
{
    if(this == pEntryData)
        return sal_True;

    FmFormData* pFormData = dynamic_cast< FmFormData* >(pEntryData);

    if( !pFormData )
        return sal_False;

    if( (XForm*)m_xForm.get() != (XForm*)pFormData->GetFormIface().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pFormData );
}


//========================================================================
// class FmControlData
//========================================================================
//------------------------------------------------------------------------
FmControlData::FmControlData( const Reference< XFormComponent >& _rxComponent, const ImageList& _rNormalImages, const ImageList& _rHCImages, FmFormData* _pParent )
    :FmEntryData( _pParent, _rxComponent )
    ,m_xFormComponent( _rxComponent )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmControlData::FmControlData" );
    //////////////////////////////////////////////////////////////////////
    // Images setzen
    m_aNormalImage = GetImage( _rNormalImages );
    m_aHCImage = GetImage( _rHCImages );

    //////////////////////////////////////////////////////////////////////
    // Titel setzen
    Reference< XPropertySet >  xSet(m_xFormComponent, UNO_QUERY);
    if( xSet.is() )
    {
#ifdef DBG_UTIL
        ::rtl::OUString aEntryName = ::comphelper::getString(xSet->getPropertyValue( FM_PROP_NAME ));
#endif
        SetText( ::comphelper::getString(xSet->getPropertyValue( FM_PROP_NAME )));
    }
}

//------------------------------------------------------------------------
FmControlData::~FmControlData()
{
}

//------------------------------------------------------------------------
FmControlData::FmControlData( const FmControlData& rControlData )
    :FmEntryData( rControlData )
{
    m_xFormComponent = rControlData.GetFormComponent();
}

//------------------------------------------------------------------------
FmEntryData* FmControlData::Clone()
{
    return new FmControlData( *this );
}

//------------------------------------------------------------------------
Image FmControlData::GetImage(const ImageList& ilNavigatorImages) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmControlData::FmControlData" );
    //////////////////////////////////////////////////////////////////////
    // Default-Image
    Image aImage = ilNavigatorImages.GetImage( RID_SVXIMG_CONTROL );

    Reference< XServiceInfo > xInfo( m_xFormComponent, UNO_QUERY );
    if (!m_xFormComponent.is())
        return aImage;

    //////////////////////////////////////////////////////////////////////
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

//------------------------------------------------------------------------
sal_Bool FmControlData::IsEqualWithoutChilds( FmEntryData* pEntryData )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmControlData::IsEqualWithoutChilds" );
    if(this == pEntryData)
        return sal_True;

    FmControlData* pControlData = dynamic_cast< FmControlData* >(pEntryData);

    if( !pControlData )
        return sal_False;

    if( (XFormComponent*)m_xFormComponent.get() != (XFormComponent*)pControlData->GetFormComponent().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pControlData );
}

//------------------------------------------------------------------------
void FmControlData::ModelReplaced( const Reference< XFormComponent >& _rxNew, const ImageList& _rNormalImages, const ImageList& _rHCImages )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmControlData::ModelReplaced" );
    m_xFormComponent = _rxNew;
    newObject( m_xFormComponent );

    // Images neu setzen
    m_aNormalImage = GetImage( _rNormalImages );
    m_aHCImage = GetImage( _rHCImages );
}

//............................................................................
namespace svxform
{
//............................................................................

    //========================================================================
    // class NavigatorFrame
    //========================================================================
    //------------------------------------------------------------------------
    NavigatorFrame::NavigatorFrame( SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                                  Window* _pParent )
      :SfxDockingWindow( _pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
      ,SfxControllerItem( SID_FM_FMEXPLORER_CONTROL, *_pBindings )
    {
        SetHelpId( HID_FORM_NAVIGATOR_WIN );

        m_pNavigatorTree = new NavigatorTree(comphelper::getProcessServiceFactory(), this );
        m_pNavigatorTree->Show();
        SetText( SVX_RES(RID_STR_FMEXPLORER) );
        SfxDockingWindow::SetFloatingSize( Size(200,200) );
    }

    //------------------------------------------------------------------------
    NavigatorFrame::~NavigatorFrame()
    {
        delete m_pNavigatorTree;
    }

    //-----------------------------------------------------------------------
    void NavigatorFrame::UpdateContent( FmFormShell* pFormShell )
    {
        m_pNavigatorTree->UpdateContent( pFormShell );
    }

    //-----------------------------------------------------------------------
    void NavigatorFrame::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
    {
        if( !pState  || SID_FM_FMEXPLORER_CONTROL != nSID )
            return;

        if( eState >= SFX_ITEM_AVAILABLE )
        {
            FmFormShell* pShell = dynamic_cast< FmFormShell* >( ((SfxObjectItem*)pState)->GetShell() );
            UpdateContent( pShell );
        }
        else
            UpdateContent( NULL );
    }

    //-----------------------------------------------------------------------
    void NavigatorFrame::GetFocus()
    {
        if ( m_pNavigatorTree )
            m_pNavigatorTree->GrabFocus();
        else
            SfxDockingWindow::GetFocus();
    }

    //-----------------------------------------------------------------------
    sal_Bool NavigatorFrame::Close()
    {
        UpdateContent( NULL );
        return SfxDockingWindow::Close();
    }

    //-----------------------------------------------------------------------
    void NavigatorFrame::FillInfo( SfxChildWinInfo& rInfo ) const
    {
        SfxDockingWindow::FillInfo( rInfo );
        rInfo.bVisible = sal_False;
    }

    //-----------------------------------------------------------------------
    Size NavigatorFrame::CalcDockingSize( SfxChildAlignment eAlign )
    {
        if ( ( eAlign == SFX_ALIGN_TOP ) || ( eAlign == SFX_ALIGN_BOTTOM ) )
            return Size();

        return SfxDockingWindow::CalcDockingSize( eAlign );
    }

    //-----------------------------------------------------------------------
    SfxChildAlignment NavigatorFrame::CheckAlignment( SfxChildAlignment _eActAlign, SfxChildAlignment _eAlign )
    {
        if ( ( _eAlign == SFX_ALIGN_LEFT ) || ( _eAlign == SFX_ALIGN_RIGHT ) || ( _eAlign == SFX_ALIGN_NOALIGNMENT ) )
            return _eAlign;
        return _eActAlign;
    }

    //------------------------------------------------------------------------
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


    //========================================================================
    // class NavigatorFrameManager
    //========================================================================

    //-----------------------------------------------------------------------
    SFX_IMPL_DOCKINGWINDOW( NavigatorFrameManager, SID_FM_SHOW_FMEXPLORER )

    //-----------------------------------------------------------------------
    NavigatorFrameManager::NavigatorFrameManager( Window* _pParent, sal_uInt16 _nId,
                                        SfxBindings* _pBindings, SfxChildWinInfo* _pInfo )
                     :SfxChildWindow( _pParent, _nId )
    {
        pWindow = new NavigatorFrame( _pBindings, this, _pParent );
        eChildAlignment = SFX_ALIGN_NOALIGNMENT;
        ((SfxDockingWindow*)pWindow)->Initialize( _pInfo );
    }

//............................................................................
}   // namespace svxform
//............................................................................
