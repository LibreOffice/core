/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmexpl.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:45:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#ifndef _SVX_FMUNOVW_HXX
#include "fmvwimp.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMEXPL_HRC
#include "fmexpl.hrc"
#endif
#ifndef _SVX_FMEXPL_HXX
#include "fmexpl.hxx"
#endif

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif

#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVDOUNO_HXX
#include <svdouno.hxx>
#endif

#ifndef _SVX_FMUNDO_HXX
#include <fmundo.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif

#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif

#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif

#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
TYPEINIT1( FmNavInsertedHint, SfxHint );
DBG_NAME(FmNavInsertedHint);
//------------------------------------------------------------------------
FmNavInsertedHint::FmNavInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos )
    :pEntryData( pInsertedEntryData )
    ,nPos( nRelPos )

{
    DBG_CTOR(FmNavInsertedHint,NULL);
}

//------------------------------------------------------------------------
FmNavInsertedHint::~FmNavInsertedHint()
{
    DBG_DTOR(FmNavInsertedHint,NULL);
}


//========================================================================
// class FmNavInsertedHint
//========================================================================
TYPEINIT1( FmNavModelReplacedHint, SfxHint );
DBG_NAME(FmNavModelReplacedHint);
//------------------------------------------------------------------------
FmNavModelReplacedHint::FmNavModelReplacedHint( FmEntryData* pAffectedEntryData )
    :pEntryData( pAffectedEntryData )
{
    DBG_CTOR(FmNavModelReplacedHint,NULL);
}

//------------------------------------------------------------------------
FmNavModelReplacedHint::~FmNavModelReplacedHint()
{
    DBG_DTOR(FmNavModelReplacedHint,NULL);
}

//========================================================================
// class FmNavRemovedHint
//========================================================================
TYPEINIT1( FmNavRemovedHint, SfxHint );
DBG_NAME(FmNavRemovedHint);
//------------------------------------------------------------------------
FmNavRemovedHint::FmNavRemovedHint( FmEntryData* pRemovedEntryData )
    :pEntryData( pRemovedEntryData )
{
    DBG_CTOR(FmNavRemovedHint,NULL);
}

//------------------------------------------------------------------------
FmNavRemovedHint::~FmNavRemovedHint()
{
    DBG_DTOR(FmNavRemovedHint,NULL);
}


//========================================================================
// class FmNavNameChangedHint
//========================================================================
TYPEINIT1( FmNavNameChangedHint, SfxHint );
DBG_NAME(FmNavNameChangedHint);
//------------------------------------------------------------------------
FmNavNameChangedHint::FmNavNameChangedHint( FmEntryData* pData, const ::rtl::OUString& rNewName )
    :pEntryData( pData )
    ,aNewName( rNewName )
{
    DBG_CTOR(FmNavNameChangedHint,NULL);
}

//------------------------------------------------------------------------
FmNavNameChangedHint::~FmNavNameChangedHint()
{
    DBG_DTOR(FmNavNameChangedHint,NULL);
}

//========================================================================
// class FmNavClearedHint
//========================================================================
TYPEINIT1( FmNavClearedHint, SfxHint );
DBG_NAME(FmNavClearedHint);
//------------------------------------------------------------------------
FmNavClearedHint::FmNavClearedHint()
{
    DBG_CTOR(FmNavClearedHint,NULL);
}

//------------------------------------------------------------------------
FmNavClearedHint::~FmNavClearedHint()
{
    DBG_DTOR(FmNavClearedHint,NULL);
}

//========================================================================
// class FmNavRequestSelectHint
//========================================================================
TYPEINIT1(FmNavRequestSelectHint, SfxHint);

//========================================================================
// class FmNavViewMarksChanged
//========================================================================
TYPEINIT1(FmNavViewMarksChanged, SfxHint);

//========================================================================
// class FmEntryDataList
//========================================================================
DBG_NAME(FmEntryDataList);
//------------------------------------------------------------------------
FmEntryDataList::FmEntryDataList()
{
    DBG_CTOR(FmEntryDataList,NULL);
}

//------------------------------------------------------------------------
FmEntryDataList::~FmEntryDataList()
{
    DBG_DTOR(FmEntryDataList,NULL);
}


//========================================================================
// class FmEntryData
//========================================================================
TYPEINIT0( FmEntryData );
DBG_NAME(FmEntryData);
//------------------------------------------------------------------------
FmEntryData::FmEntryData( FmEntryData* pParentData, const Reference< XInterface >& _rxIFace )
    :pParent( pParentData )
{
    DBG_CTOR(FmEntryData,NULL);
    pChildList = new FmEntryDataList();

    newObject( _rxIFace );
}

//------------------------------------------------------------------------
FmEntryData::~FmEntryData()
{
    Clear();
    delete pChildList;
    DBG_DTOR(FmEntryData,NULL);
}

//------------------------------------------------------------------------
void FmEntryData::newObject( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace )
{
    // do not just copy, normalize it
    m_xNormalizedIFace = Reference< XInterface >( _rxIFace, UNO_QUERY );
    m_xProperties = m_xProperties.query( m_xNormalizedIFace );
    m_xChild = m_xChild.query( m_xNormalizedIFace );
}

//------------------------------------------------------------------------
FmEntryData::FmEntryData( const FmEntryData& rEntryData )
{
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
    for (;;)
    {
        FmEntryData* pEntryData = GetChildList()->Remove(ULONG(0));
        if (pEntryData == NULL)
            break;
        delete pEntryData;
    }
}

//------------------------------------------------------------------------
sal_Bool FmEntryData::IsEqualWithoutChilds( FmEntryData* pEntryData )
{
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
TYPEINIT1( FmFormData, FmEntryData );
DBG_NAME(FmFormData);
//------------------------------------------------------------------------
FmFormData::FmFormData( const Reference< XForm >& _rxForm, const ImageList& _rNormalImages, const ImageList& _rHCImages, FmFormData* _pParent )
    :FmEntryData( _pParent, _rxForm )
    ,m_xForm( _rxForm )
{
    DBG_CTOR(FmEntryData,NULL);
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
    DBG_DTOR(FmEntryData,NULL);
}

//------------------------------------------------------------------------
FmFormData::FmFormData( const FmFormData& rFormData )
    :FmEntryData( rFormData )
{
    DBG_CTOR(FmEntryData,NULL);
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
    if( !pEntryData->ISA(FmFormData) )
        return sal_False;
    FmFormData* pFormData = (FmFormData*)pEntryData;
    if( (XForm*)m_xForm.get() != (XForm*)pFormData->GetFormIface().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pFormData );
}


//========================================================================
// class FmControlData
//========================================================================
TYPEINIT1( FmControlData, FmEntryData );
DBG_NAME(FmControlData);
//------------------------------------------------------------------------
FmControlData::FmControlData( const Reference< XFormComponent >& _rxComponent, const ImageList& _rNormalImages, const ImageList& _rHCImages, FmFormData* _pParent )
    :FmEntryData( _pParent, _rxComponent )
    ,m_xFormComponent( _rxComponent )
{
    DBG_CTOR(FmControlData,NULL);
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
    DBG_DTOR(FmControlData,NULL);
}

//------------------------------------------------------------------------
FmControlData::FmControlData( const FmControlData& rControlData )
    :FmEntryData( rControlData )
{
    DBG_CTOR(FmControlData,NULL);
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
    if(this == pEntryData)
        return sal_True;

    if( !pEntryData->ISA(FmControlData) )
        return sal_False;
    FmControlData* pControlData = (FmControlData*)pEntryData;

    if( (XFormComponent*)m_xFormComponent.get() != (XFormComponent*)pControlData->GetFormComponent().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pControlData );
}

//------------------------------------------------------------------------
void FmControlData::ModelReplaced( const Reference< XFormComponent >& _rxNew, const ImageList& _rNormalImages, const ImageList& _rHCImages )
{
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
    DBG_NAME(NavigatorFrame)
    //------------------------------------------------------------------------
    NavigatorFrame::NavigatorFrame( SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                                  Window* _pParent )
      :SfxDockingWindow( _pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
      ,SfxControllerItem( SID_FM_FMEXPLORER_CONTROL, *_pBindings )
    {
        DBG_CTOR(NavigatorFrame,NULL);
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
        DBG_DTOR(NavigatorFrame,NULL);
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
            FmFormShell* pShell = PTR_CAST( FmFormShell,((SfxObjectItem*)pState)->GetShell() );
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
