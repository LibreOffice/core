/*************************************************************************
 *
 *  $RCSfile: fmexpl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_TABORDER_HXX
#include "taborder.hxx"
#endif

#ifndef _SVX_DBERRBOX_HXX
#include "dbmsgbox.hxx"
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

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
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
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif // _SVX_FMPROP_HXX

#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

//========================================================================

#define DROP_ACTION_TIMER_INITIAL_TICKS     10
    // solange dauert es, bis das Scrollen anspringt
#define DROP_ACTION_TIMER_SCROLL_TICKS      3
    // in diesen Intervallen wird jeweils eine Zeile gescrollt
#define DROP_ACTION_TIMER_TICK_BASE         10
    // das ist die Basis, mit der beide Angaben multipliziert werden (in ms)

#define EXPLORER_SYNC_DELAY                 200
    // dieser Betrag an Millisekunden wird gewartet, ehe der Explorer nach einem Select oder Deselect die ::com::sun::star::sdbcx::View synchronisiert

SV_IMPL_PTRARR_SORT( FmEntryDataArray, FmEntryDataPtr )
SV_IMPL_PTRARR_SORT( SvLBoxEntrySortedArray, SvLBoxEntryPtr )

//========================================================================
// class FmExplInsertedHint
//========================================================================
TYPEINIT1( FmExplInsertedHint, SfxHint );
DBG_NAME(FmExplInsertedHint);
//------------------------------------------------------------------------
FmExplInsertedHint::FmExplInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos )
    :pEntryData( pInsertedEntryData )
    ,nPos( nRelPos )

{
    DBG_CTOR(FmExplInsertedHint,NULL);
}

//------------------------------------------------------------------------
FmExplInsertedHint::~FmExplInsertedHint()
{
    DBG_DTOR(FmExplInsertedHint,NULL);
}


//========================================================================
// class FmExplInsertedHint
//========================================================================
TYPEINIT1( FmExplModelReplacedHint, SfxHint );
DBG_NAME(FmExplModelReplacedHint);
//------------------------------------------------------------------------
FmExplModelReplacedHint::FmExplModelReplacedHint( FmEntryData* pAffectedEntryData )
    :pEntryData( pAffectedEntryData )
{
    DBG_CTOR(FmExplModelReplacedHint,NULL);
}

//------------------------------------------------------------------------
FmExplModelReplacedHint::~FmExplModelReplacedHint()
{
    DBG_DTOR(FmExplModelReplacedHint,NULL);
}

//========================================================================
// class FmExplRemovedHint
//========================================================================
TYPEINIT1( FmExplRemovedHint, SfxHint );
DBG_NAME(FmExplRemovedHint);
//------------------------------------------------------------------------
FmExplRemovedHint::FmExplRemovedHint( FmEntryData* pRemovedEntryData )
    :pEntryData( pRemovedEntryData )
{
    DBG_CTOR(FmExplRemovedHint,NULL);
}

//------------------------------------------------------------------------
FmExplRemovedHint::~FmExplRemovedHint()
{
    DBG_DTOR(FmExplRemovedHint,NULL);
}


//========================================================================
// class FmExplNameChangedHint
//========================================================================
TYPEINIT1( FmExplNameChangedHint, SfxHint );
DBG_NAME(FmExplNameChangedHint);
//------------------------------------------------------------------------
FmExplNameChangedHint::FmExplNameChangedHint( FmEntryData* pData, const ::rtl::OUString& rNewName )
    :pEntryData( pData )
    ,aNewName( rNewName )
{
    DBG_CTOR(FmExplNameChangedHint,NULL);
}

//------------------------------------------------------------------------
FmExplNameChangedHint::~FmExplNameChangedHint()
{
    DBG_DTOR(FmExplNameChangedHint,NULL);
}

//========================================================================
// class FmExplClearedHint
//========================================================================
TYPEINIT1( FmExplClearedHint, SfxHint );
DBG_NAME(FmExplClearedHint);
//------------------------------------------------------------------------
FmExplClearedHint::FmExplClearedHint()
{
    DBG_CTOR(FmExplClearedHint,NULL);
}

//------------------------------------------------------------------------
FmExplClearedHint::~FmExplClearedHint()
{
    DBG_DTOR(FmExplClearedHint,NULL);
}

//========================================================================
// class FmExplRequestSelectHint
//========================================================================
TYPEINIT1(FmExplRequestSelectHint, SfxHint);

//========================================================================
// class FmExplViewMarksChanged
//========================================================================
TYPEINIT1(FmExplViewMarksChanged, SfxHint);

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
FmEntryData::FmEntryData( FmEntryData* pParentData )
    :pParent( pParentData )
{
    DBG_CTOR(FmEntryData,NULL);
    pChildList = new FmEntryDataList();
}

//------------------------------------------------------------------------
FmEntryData::~FmEntryData()
{
    Clear();
    delete pChildList;
    DBG_DTOR(FmEntryData,NULL);
}

//------------------------------------------------------------------------
FmEntryData::FmEntryData( const FmEntryData& rEntryData )
{
    pChildList = new FmEntryDataList();
    aText = rEntryData.GetText();
    aCollapsedImage = rEntryData.GetCollapsedImage();
    aExpandedImage = rEntryData.GetExpandedImage();
    pParent = rEntryData.GetParent();

    FmEntryData* pChildData;
    sal_uInt32 nEntryCount = rEntryData.GetChildList()->Count();
    for( sal_uInt32 i=0; i<nEntryCount; i++ )
    {
        pChildData = rEntryData.GetChildList()->GetObject(i);
        FmEntryData* pNewChildData = pChildData->Clone();
        pChildList->Insert( pNewChildData, LIST_APPEND );
    }
}

//------------------------------------------------------------------------
void FmEntryData::Clear()
{
    FmEntryData* pEntryData;
    while( pEntryData = GetChildList()->Remove(sal_uInt32(0)) )
        delete pEntryData;
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
FmFormData::FmFormData( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm, const ImageList& ilNavigatorImages, FmFormData* pParent )
    :FmEntryData( pParent )
    ,m_xForm( xForm )
{
    DBG_CTOR(FmEntryData,NULL);
    //////////////////////////////////////////////////////////////////////
    // Images setzen

    aCollapsedImage = ilNavigatorImages.GetImage( RID_SVXIMG_FORM );
    aExpandedImage  = ilNavigatorImages.GetImage( RID_SVXIMG_FORM );

    //////////////////////////////////////////////////////////////////////
    // Titel setzen
    if (m_xForm.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xForm, ::com::sun::star::uno::UNO_QUERY);
        if (xSet.is())
        {
            ::rtl::OUString aEntryName(::utl::getString(xSet->getPropertyValue( FM_PROP_NAME )));
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
    if( (::com::sun::star::form::XForm*)m_xForm.get() != (::com::sun::star::form::XForm*)pFormData->GetFormIface().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pFormData );
}


//========================================================================
// class FmControlData
//========================================================================
TYPEINIT1( FmControlData, FmEntryData );
DBG_NAME(FmControlData);
//------------------------------------------------------------------------
FmControlData::FmControlData( ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent, const ImageList& ilNavigatorImages, FmFormData* pParent )
    :FmEntryData( pParent )
    ,m_xFormComponent( xFormComponent )
{
    DBG_CTOR(FmControlData,NULL);
    //////////////////////////////////////////////////////////////////////
    // Images setzen
    aCollapsedImage = GetImage(ilNavigatorImages);
    aExpandedImage = GetImage(ilNavigatorImages);

    //////////////////////////////////////////////////////////////////////
    // Titel setzen
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xFormComponent, ::com::sun::star::uno::UNO_QUERY);
    if( xSet.is() )
    {
#if DBG_UTIL
        ::rtl::OUString aEntryName = ::utl::getString(xSet->getPropertyValue( FM_PROP_NAME ));
#endif
        SetText( ::utl::getString(xSet->getPropertyValue( FM_PROP_NAME )));
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

    if (!m_xFormComponent.is()) return aImage;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >  xInfo(m_xFormComponent, ::com::sun::star::uno::UNO_QUERY);
    if (!xInfo.is())
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

    if( (::com::sun::star::form::XFormComponent*)m_xFormComponent.get() != (::com::sun::star::form::XFormComponent*)pControlData->GetFormComponent().get() )
        return sal_False;

    return FmEntryData::IsEqualWithoutChilds( pControlData );
}

//------------------------------------------------------------------------
void FmControlData::ModelReplaced(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xNew, const ImageList& ilNavigatorImages)
{
    m_xFormComponent = xNew;
    // Images neu setzen
    aCollapsedImage = aExpandedImage = GetImage(ilNavigatorImages);
}

//========================================================================
// class FmXExplPropertyChangeList
//========================================================================

//------------------------------------------------------------------------
FmXExplPropertyChangeList::FmXExplPropertyChangeList(FmExplorerModel* _pModel)
                          :m_pExplModel(_pModel)
                          ,m_bCanUndo(sal_True)
                          ,m_nLocks(0)
{
}

// ::com::sun::star::beans::XPropertyChangeListener
//------------------------------------------------------------------------
void SAL_CALL FmXExplPropertyChangeList::disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------------
void SAL_CALL FmXExplPropertyChangeList::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    if( !m_pExplModel ) return;
    if( evt.PropertyName != FM_PROP_NAME ) return;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent(evt.Source, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(evt.Source, ::com::sun::star::uno::UNO_QUERY);

    FmEntryData* pEntryData;
    if( xForm.is() )
        pEntryData = m_pExplModel->FindData( xForm, m_pExplModel->GetRootList() );
    else if( xFormComponent.is() )
        pEntryData = m_pExplModel->FindData( xFormComponent, m_pExplModel->GetRootList() );

    if( pEntryData )
    {
        ::rtl::OUString aNewName =  ::utl::getString(evt.NewValue);
        pEntryData->SetText( aNewName );
        FmExplNameChangedHint aNameChangedHint( pEntryData, aNewName );
        m_pExplModel->Broadcast( aNameChangedHint );
    }
}

// ::com::sun::star::container::XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXExplPropertyChangeList::elementInserted(const ::com::sun::star::container::ContainerEvent& evt)
{
    if (IsLocked() || !m_pExplModel)
        return;

    // keine Undoaction einfuegen
    m_bCanUndo = sal_False;

    Insert(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)evt.Element.getValue(), ::utl::getINT32(evt.Accessor));

    m_bCanUndo = sal_True;
}

//------------------------------------------------------------------------------
void FmXExplPropertyChangeList::Insert(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xIface, sal_Int32 nIndex)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(xIface, ::com::sun::star::uno::UNO_QUERY);
    if (xForm.is())
    {
        m_pExplModel->InsertForm(xForm, sal_uInt32(nIndex));
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xForm, ::com::sun::star::uno::UNO_QUERY);
        for (sal_Int32 i = 0; i < xContainer->getCount(); i++)
            Insert(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xContainer->getByIndex(i).getValue(), i);
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComp(xIface, ::com::sun::star::uno::UNO_QUERY);
        if (xFormComp.is())
            m_pExplModel->InsertFormComponent(xFormComp, sal_uInt32(nIndex));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXExplPropertyChangeList::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt)
{
    if (IsLocked() || !m_pExplModel)
        return;

    m_bCanUndo = sal_False;

    // EntryData loeschen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xReplaced(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)evt.ReplacedElement.getValue(), ::com::sun::star::uno::UNO_QUERY);
    FmEntryData* pEntryData = m_pExplModel->FindData(xReplaced, m_pExplModel->GetRootList(), sal_True);
    if (pEntryData)
    {
        if (pEntryData->ISA(FmControlData))
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xComp(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)evt.Element.getValue(), ::com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xComp.is(), "FmXExplPropertyChangeList::elementReplaced : invalid argument !");
                // an einer FmControlData sollte eine ::com::sun::star::form::XFormComponent haengen
            m_pExplModel->ReplaceFormComponent(xReplaced, xComp);
        }
        else if (pEntryData->ISA(FmFormData))
        {
            DBG_ERROR("replacing forms not implemented yet !");
        }
    }

    m_bCanUndo = sal_True;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXExplPropertyChangeList::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt)
{
    if (IsLocked() || !m_pExplModel)
        return;

    m_bCanUndo = sal_False;

    //////////////////////////////////////////////////////////
    // EntryData loeschen
    FmEntryData* pEntryData = m_pExplModel->FindData(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)evt.Element.getValue(), m_pExplModel->GetRootList(), sal_True);
    if (pEntryData)
        m_pExplModel->Remove(pEntryData);

    m_bCanUndo = sal_True;
}


//========================================================================
// class FmExplorerModel
//========================================================================

//------------------------------------------------------------------------
FmExplorerModel::FmExplorerModel(const ImageList& ilNavigatorImages)
                :m_pFormShell(NULL)
                ,m_pFormPage(NULL)
                ,m_pFormModel(NULL)
                ,m_ilNavigatorImages(ilNavigatorImages)
{
    m_pPropChangeList = new FmXExplPropertyChangeList(this);
    m_pPropChangeList->acquire();
    m_pRootList = new FmEntryDataList();
}

//------------------------------------------------------------------------
FmExplorerModel::~FmExplorerModel()
{
    //////////////////////////////////////////////////////////////////////
    // Als Listener abmelden
    if( m_pFormShell)
    {
        FmFormModel* pFormModel = m_pFormShell->GetFormModel();
        if( pFormModel && IsListening(*pFormModel))
            EndListening( *pFormModel );

        if (IsListening(*m_pFormShell))
            EndListening(*m_pFormShell);
    }

    Clear();
    delete m_pRootList;
    m_pPropChangeList->ReleaseModel();
    m_pPropChangeList->release();
}


//------------------------------------------------------------------------
void FmExplorerModel::SetModified( sal_Bool bMod )
{
    if( !m_pFormShell ) return;
    SfxObjectShell* pObjShell = m_pFormShell->GetFormModel()->GetObjectShell();
    if( !pObjShell ) return;
    pObjShell->SetModified( bMod );
}

//------------------------------------------------------------------------
void FmExplorerModel::Clear()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  xForms( GetForms());
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(xForms, ::com::sun::star::uno::UNO_QUERY);
    if (xContainer.is())
        xContainer->removeContainerListener((::com::sun::star::container::XContainerListener*)m_pPropChangeList);

    //////////////////////////////////////////////////////////////////////
    // RootList loeschen
    FmEntryData* pChildData;
    FmEntryDataList* pRootList = GetRootList();

    for( sal_uInt32 i=pRootList->Count(); i>0; i-- )
    {
        pChildData = pRootList->GetObject(i-1);
        pRootList->Remove( pChildData );
        delete pChildData;
    }

    //////////////////////////////////////////////////////////////////////
    // UI benachrichtigen
    FmExplClearedHint aClearedHint;
    Broadcast( aClearedHint );
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  FmExplorerModel::GetForms() const
{
    if( !m_pFormShell || !m_pFormShell->GetCurPage())
        return NULL;
    else
        return m_pFormShell->GetCurPage()->GetForms();
}

//------------------------------------------------------------------------
void FmExplorerModel::Insert(FmEntryData* pEntry, sal_uInt32 nRelPos, sal_Bool bAlterModel)
{
    if (IsListening(*m_pFormModel))
        EndListening(*m_pFormModel);

    m_pPropChangeList->Lock();
    FmFormData* pFolder     = (FmFormData*) pEntry->GetParent();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >     xElement    (pEntry->GetElement(), ::com::sun::star::uno::UNO_QUERY);
    if (bAlterModel)
    {
        XubString aStr;
        if (pEntry->ISA(FmFormData))
            aStr = SVX_RES(RID_STR_FORM);
        else
            aStr = SVX_RES(RID_STR_CONTROL);

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer;
        if (pFolder)
            xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pFolder->GetFormIface(), ::com::sun::star::uno::UNO_QUERY);
        else
            xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (GetForms(), ::com::sun::star::uno::UNO_QUERY);

        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
        aUndoStr.SearchAndReplace('#', aStr);
        m_pFormModel->BegUndo(aUndoStr);

        if (nRelPos >= xContainer->getCount())
            nRelPos = (sal_Int32)xContainer->getCount();

        // UndoAction
        if (m_pPropChangeList->CanUndo())
            m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                     FmUndoContainerAction::Inserted,
                                                     xContainer,
                                                     xElement,
                                                     nRelPos));

        // das Element muss den Typ haben, den der ::com::sun::star::sdbcx::Container erwartet
        if (xContainer->getElementType() ==
            ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>*)0))

        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xElementAsForm(xElement, ::com::sun::star::uno::UNO_QUERY);
            xContainer->insertByIndex(nRelPos, ::com::sun::star::uno::makeAny(xElementAsForm));
        }
        else if (xContainer->getElementType() ==
            ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>*)0))

        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xElementAsComponent(xElement, ::com::sun::star::uno::UNO_QUERY);
            xContainer->insertByIndex(nRelPos, ::com::sun::star::uno::makeAny(xElementAsComponent));
        }
        else
        {
            DBG_ERROR("FmExplorerModel::Insert : the parent container needs an elementtype I don't know !");
        }

        m_pFormModel->EndUndo();
    }

    //////////////////////////////////////////////////////////////////////
    // Als PropertyChangeListener anmelden
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xElement, ::com::sun::star::uno::UNO_QUERY);
    if( xSet.is() )
        xSet->addPropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

    //////////////////////////////////////////////////////////////////////
    // Daten aus Model entfernen
    if (pEntry->ISA(FmFormData))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(xElement, ::com::sun::star::uno::UNO_QUERY);
        if (xContainer.is())
            xContainer->addContainerListener((::com::sun::star::container::XContainerListener*)m_pPropChangeList);
    }

    if (pFolder)
        pFolder->GetChildList()->Insert( pEntry, nRelPos );
    else
        GetRootList()->Insert( pEntry, nRelPos );

    //////////////////////////////////////////////////////////////////////
    // UI benachrichtigen
    FmExplInsertedHint aInsertedHint( pEntry, nRelPos );
    Broadcast( aInsertedHint );

    m_pPropChangeList->UnLock();
    if (IsListening(*m_pFormModel))
        StartListening(*m_pFormModel);
}

//------------------------------------------------------------------------
void FmExplorerModel::Remove(FmEntryData* pEntry, sal_Bool bAlterModel)
{
    //////////////////////////////////////////////////////////////////////
    // ::com::sun::star::form::Form und Parent holen
    if (!pEntry || !m_pFormModel)
        return;

    if (IsListening(*m_pFormModel))
        EndListening(*m_pFormModel);

    m_pPropChangeList->Lock();
    FmFormData*     pFolder     = (FmFormData*) pEntry->GetParent();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >      xElement    (pEntry->GetElement(), ::com::sun::star::uno::UNO_QUERY);
    if (bAlterModel)
    {
        XubString        aStr;
        if (pEntry->ISA(FmFormData))
            aStr = SVX_RES(RID_STR_FORM);
        else
            aStr = SVX_RES(RID_STR_CONTROL);

        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE));
        aUndoStr.SearchAndReplace('#', aStr);
        m_pFormModel->BegUndo(aUndoStr);
    }

    // jetzt die eigentliche Entfernung der Daten aus dem Model
    if (pEntry->ISA(FmFormData))
        RemoveForm((FmFormData*)pEntry);
    else
        RemoveFormComponent((FmControlData*)pEntry);


    if (bAlterModel)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xElement->getParent(), ::com::sun::star::uno::UNO_QUERY);
        // aus dem ::com::sun::star::sdbcx::Container entfernen
        sal_Int32 nContainerIndex = getElementPos(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xContainer, ::com::sun::star::uno::UNO_QUERY), xElement);
        // UndoAction
        if (nContainerIndex >= 0)
        {
            if (m_pPropChangeList->CanUndo())
                m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                      FmUndoContainerAction::Removed,
                                                      xContainer,
                                                      xElement, nContainerIndex ));
            xContainer->removeByIndex(nContainerIndex );
        }
        m_pFormModel->EndUndo();
    }

    // beim Vater austragen
    if (pFolder)
        pFolder->GetChildList()->Remove(pEntry);
    else
    {
        GetRootList()->Remove(pEntry);
        //////////////////////////////////////////////////////////////////////
        // Wenn keine ::com::sun::star::form::Form mehr in der Root, an der Shell CurForm zuruecksetzen
        if (!GetRootList()->Count())
            m_pFormShell->GetImpl()->setCurForm( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > () );
    }

    //////////////////////////////////////////////////////////////////////
    // UI benachrichtigen
    FmExplRemovedHint aRemovedHint( pEntry );
    Broadcast( aRemovedHint );

    // Eintrag loeschen
    delete pEntry;

    m_pPropChangeList->UnLock();
    StartListening(*m_pFormModel);
}

//------------------------------------------------------------------------
void FmExplorerModel::RemoveForm(FmFormData* pFormData)
{
    //////////////////////////////////////////////////////////////////////
    // ::com::sun::star::form::Form und Parent holen
    if (!pFormData || !m_pFormModel)
        return;

    FmEntryDataList*    pChildList = pFormData->GetChildList();
    sal_uInt32 nCount = pChildList->Count();
    for (sal_uInt32 i = nCount; i > 0; i--)
    {
        FmEntryData* pEntryData = pChildList->GetObject(i - 1);

        //////////////////////////////////////////////////////////////////////
        // Child ist ::com::sun::star::form::Form -> rekursiver Aufruf
        if( pEntryData->ISA(FmFormData) )
            RemoveForm( (FmFormData*)pEntryData);
        else if( pEntryData->ISA(FmControlData) )
            RemoveFormComponent((FmControlData*) pEntryData);
    }

    //////////////////////////////////////////////////////////////////////
    // Als PropertyChangeListener abmelden
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(pFormData->GetFormIface(), ::com::sun::star::uno::UNO_QUERY);
    if( xSet.is() )
        xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(xSet, ::com::sun::star::uno::UNO_QUERY);
    if (xContainer.is())
        xContainer->removeContainerListener((::com::sun::star::container::XContainerListener*)m_pPropChangeList);
}

//------------------------------------------------------------------------
void FmExplorerModel::RemoveFormComponent(FmControlData* pControlData)
{
    //////////////////////////////////////////////////////////////////////
    // Control und Parent holen
    if (!pControlData)
        return;

    //////////////////////////////////////////////////////////////////////
    // Als PropertyChangeListener abmelden
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(pControlData->GetElement(), ::com::sun::star::uno::UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList);
}

//------------------------------------------------------------------------
void FmExplorerModel::ClearBranch( FmFormData* pParentData )
{
    //////////////////////////////////////////////////////////////////////
    // Alle Eintraege dieses Zweiges loeschen
    FmEntryDataList* pChildList = pParentData->GetChildList();
    FmEntryData* pChildData;

    for( sal_uInt32 i=pChildList->Count(); i>0; i-- )
    {
        pChildData = pChildList->GetObject(i-1);
        if( pChildData->ISA(FmFormData) )
            ClearBranch( (FmFormData*)pChildData );

        pChildList->Remove( pChildData );
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::FillBranch( FmFormData* pFormData )
{
    //////////////////////////////////////////////////////////////
    // ::com::sun::star::form::Forms aus der Root einfuegen
    if( pFormData == NULL )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >   xForms(GetForms(), ::com::sun::star::uno::UNO_QUERY);
        if (!xForms.is()) return;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >     xSubForm;
        FmFormData* pSubFormData;
        for (sal_uInt32 i=0; i<xForms->getCount(); ++i)
        {
            DBG_ASSERT(xForms->getByIndex(i).getValueType() ==
                ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>*)0),

                "FmExplorerModel::FillBranch : the root container should supply only elements of type XForm");
            xSubForm = *(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > *)xForms->getByIndex(i).getValue();
            pSubFormData = new FmFormData( xSubForm, m_ilNavigatorImages, pFormData );
            Insert( pSubFormData, LIST_APPEND );

            //////////////////////////////////////////////////////////////
            // Neuer Branch, wenn SubForm wiederum Subforms enthaelt
            FillBranch( pSubFormData );
        }
    }

    //////////////////////////////////////////////////////////////
    // Componenten einfuegen
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xComponents( GetFormComponents(pFormData));
        if( !xComponents.is() ) return;

        ::rtl::OUString aControlName;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet;
        FmControlData* pNewControlData;
        FmFormData* pSubFormData;

        for (sal_Int32 j=0; j<xComponents->getCount(); ++j)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xCurrentComponent(*(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > *)xComponents->getByIndex(j).getValue());
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xSubForm(xCurrentComponent, ::com::sun::star::uno::UNO_QUERY);

            if (xSubForm.is())
            {   // die aktuelle Component ist eine ::com::sun::star::form::Form
                pSubFormData = new FmFormData(xSubForm, m_ilNavigatorImages, pFormData);
                Insert(pSubFormData, LIST_APPEND);

                //////////////////////////////////////////////////////////////
                // Neuer Branch, wenn SubForm wiederum Subforms enthaelt
                FillBranch(pSubFormData);
            }
            else
            {
                pNewControlData = new FmControlData(xCurrentComponent, m_ilNavigatorImages, pFormData);
                Insert(pNewControlData, LIST_APPEND);
            }
        }
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::InsertForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xForm, sal_uInt32 nRelPos)
{
    FmFormData* pFormData = (FmFormData*)FindData( xForm, GetRootList() );
    if (pFormData)
        return;

    //////////////////////////////////////////////////////////
    // ParentData setzen
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xIFace( xForm->getParent());
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xParentForm(xIFace, ::com::sun::star::uno::UNO_QUERY);
    FmFormData* pParentData = NULL;
    if (xParentForm.is())
        pParentData = (FmFormData*)FindData( xParentForm, GetRootList() );

    pFormData = new FmFormData( xForm, m_ilNavigatorImages, pParentData );
    Insert( pFormData, nRelPos );
}

//------------------------------------------------------------------------
void FmExplorerModel::InsertFormComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xComp, sal_uInt32 nRelPos)
{
    //////////////////////////////////////////////////////////
    // ParentData setzen
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xIFace( xComp->getParent());
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(xIFace, ::com::sun::star::uno::UNO_QUERY);
    if (!xForm.is())
        return;

    FmFormData* pParentData = (FmFormData*)FindData( xForm, GetRootList() );
    if( !pParentData )
    {
        pParentData = new FmFormData( xForm, m_ilNavigatorImages, NULL );
        Insert( pParentData, LIST_APPEND );
    }

    if (!FindData(xComp, pParentData->GetChildList(),sal_False))
    {
        //////////////////////////////////////////////////////////
        // Neue EntryData setzen
        FmEntryData* pNewEntryData = new FmControlData( xComp, m_ilNavigatorImages, pParentData );

        //////////////////////////////////////////////////////////
        // Neue EntryData einfuegen
        Insert( pNewEntryData, nRelPos );
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::ReplaceFormComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xOld, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xNew)
{
    FmEntryData* pData = FindData(xOld, GetRootList(), sal_True);
    DBG_ASSERT(pData && pData->ISA(FmControlData), "FmExplorerModel::ReplaceFormComponent : invalid argument !");
    ((FmControlData*)pData)->ModelReplaced(xNew, m_ilNavigatorImages);

    FmExplModelReplacedHint aReplacedHint( pData );
    Broadcast( aReplacedHint );
}

//------------------------------------------------------------------------
FmEntryData* FmExplorerModel::FindData(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xElement, FmEntryDataList* pDataList, sal_Bool bRecurs)
{
    for (sal_uInt16 i=0; i < pDataList->Count(); i++)
    {
        FmEntryData* pEntryData = pDataList->GetObject(i);
        if (pEntryData->GetElement() == xElement)
            // zu beachten : das == fuer ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  macht einen 'tiefen' Vergleich, liefert also sal_True, wenn die beiden Refs das
            // selbe Objekt bezeichnen, egal ob die auf die selben Interfaces zeigen
            return pEntryData;
        else if (bRecurs)
        {
            pEntryData = FindData( xElement, pEntryData->GetChildList() );
            if (pEntryData)
                return pEntryData;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------
FmEntryData* FmExplorerModel::FindData( const ::rtl::OUString& rText, FmFormData* pParentData, sal_Bool bRecurs )
{
    FmEntryDataList* pDataList;
    if( !pParentData )
        pDataList = GetRootList();
    else
        pDataList = pParentData->GetChildList();

    ::rtl::OUString aEntryText;
    FmEntryData* pEntryData;
    FmEntryData* pChildData;

    for( sal_uInt16 i=0; i<pDataList->Count(); i++ )
    {
        pEntryData = pDataList->GetObject(i);
        aEntryText = pEntryData->GetText();

        if( aEntryText.equalsIgnoreCase(rText))
            return pEntryData;

        if( bRecurs && pEntryData->ISA(FmFormData) )
        {
            pChildData = FindData( rText, (FmFormData*)pEntryData );
            if( pChildData )
                return pChildData;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------
void FmExplorerModel::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA(SdrHint) )
    {
        SdrHint* pSdrHint = (SdrHint*)&rHint;
        switch( pSdrHint->GetKind() )
        {
            case HINT_OBJINSERTED:
                InsertSdrObj(pSdrHint->GetObject());
                break;
            case HINT_OBJREMOVED:
                RemoveSdrObj(pSdrHint->GetObject());
                break;
        }
    }
    // hat sich die shell verabschiedet?
    else if ( rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
        Update((FmFormShell*)NULL);

    // hat sich die Markierung der Controls veraendert ?
    else if (rHint.ISA(FmExplViewMarksChanged))
    {
        FmExplViewMarksChanged* pvmcHint = (FmExplViewMarksChanged*)&rHint;
        BroadcastMarkedObjects( pvmcHint->GetAffectedView()->GetMarkList() );
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::InsertSdrObj(const SdrObject* pObj)
{
    if (pObj->GetObjInventor() == FmFormInventor)
    {                                           //////////////////////////////////////////////////////////////////////
        // Ist dieses Objekt ein ::com::sun::star::form::XFormComponent?
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent(((SdrUnoObj*)pObj)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xFormComponent.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xFormComponent->getParent(), ::com::sun::star::uno::UNO_QUERY);
            if (xContainer.is())
            {
                sal_Int32 nPos = getElementPos(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xContainer, ::com::sun::star::uno::UNO_QUERY), xFormComponent);
                InsertFormComponent(xFormComponent, nPos);
            }
        }
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while (aIter.IsMore())
            InsertSdrObj(aIter.Next());
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::RemoveSdrObj(const SdrObject* pObj)
{
    if (pObj->GetObjInventor() == FmFormInventor)
    {                                           //////////////////////////////////////////////////////////////////////
        // Ist dieses Objekt ein ::com::sun::star::form::XFormComponent?
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent(((SdrUnoObj*)pObj)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xFormComponent.is())
        {
            FmEntryData* pEntryData = FindData(xFormComponent, GetRootList(), sal_True);
            if (pEntryData)
                Remove(pEntryData);
        }
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while (aIter.IsMore())
            RemoveSdrObj(aIter.Next());
    }
}

sal_Bool FmExplorerModel::InsertFormComponent(FmExplRequestSelectHint& rHint, SdrObject* pObject)
{
    if ( pObject->ISA(SdrObjGroup) )
    {   // rekursiv absteigen
        const SdrObjList *pChilds = ((SdrObjGroup*)pObject)->GetSubList();
        for ( sal_uInt16 i=0; i<pChilds->GetObjCount(); ++i )
        {
            SdrObject* pCurrent = pChilds->GetObj(i);
            if (!InsertFormComponent(rHint, pCurrent))
                return sal_False;
        }
    } else
        if (pObject->IsUnoObj())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xControlModel( ((SdrUnoObj*)pObject)->GetUnoControlModel());
            // Ist dieses Objekt ein ::com::sun::star::form::XFormComponent?
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormViewControl(xControlModel, ::com::sun::star::uno::UNO_QUERY);
            if (xFormViewControl.is())
            {   // es ist ein ::com::sun::star::form::Form-Control -> selektieren lassen
                FmEntryData* pControlData = FindData( xFormViewControl, GetRootList() );
                if (pControlData)
                    rHint.AddItem( pControlData );
            } else
            {   // es ist kein ::com::sun::star::form::Form-Control -> im Baum ueberhaupt nix selektieren lassen
                return sal_False;
            }
        } else
            return sal_False;

    return sal_True;
}

void FmExplorerModel::BroadcastMarkedObjects(const SdrMarkList& mlMarked)
{
    // gehen wir durch alle markierten Objekte und suchen wir die raus, mit denen ich was anfangen kann
    FmExplRequestSelectHint rshRequestSelection;
    sal_Bool bIsMixedSelection = sal_False;

    for (int i=0; (i<mlMarked.GetMarkCount()) && !bIsMixedSelection; i++)
    {
        SdrObject* pobjCurrent = mlMarked.GetMark(i)->GetObj();
        bIsMixedSelection |= !InsertFormComponent(rshRequestSelection, pobjCurrent);
            // bei einem Nicht-::com::sun::star::form::Form-Control liefert InsertFormComponent sal_False !
    }

    rshRequestSelection.SetMixedSelection(bIsMixedSelection);
    if (bIsMixedSelection)
        rshRequestSelection.ClearItems();

    Broadcast(rshRequestSelection);
        // eine leere Liste interpretiert der FmExplorer so, dass er seine Selektion komplett rausnimmt
}

//------------------------------------------------------------------------
void FmExplorerModel::Update( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & xForms )
{
    //////////////////////////////////////////////////////////////////////
    // Model von der Root aufwaerts neu fuellen
    Clear();
    if (xForms.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xFormContainer(xForms, ::com::sun::star::uno::UNO_QUERY);
        if (xFormContainer.is())
            xFormContainer->addContainerListener((::com::sun::star::container::XContainerListener*)m_pPropChangeList);

        FillBranch(NULL);

        // jetzt in meinem Tree genau die das in meiner ::com::sun::star::sdbcx::View markierte Control selektieren
        // (bzw alle solchen), falls es eines gibt ...
        if(!m_pFormShell) return;       // keine Shell -> wech

        FmFormView* pFormView = m_pFormShell->GetFormView();
        DBG_ASSERT(pFormView != NULL, "FmExplorerModel::Update : keine FormView");
        BroadcastMarkedObjects(pFormView->GetMarkList());
    }
}

//------------------------------------------------------------------------
void FmExplorerModel::Update( FmFormShell* pShell )
{
    //////////////////////////////////////////////////////////////////////
    // Wenn Shell sich nicht veraendert hat, nichts machen
    FmFormPage* pNewPage = pShell ? pShell->GetCurPage() : NULL;
    if ((pShell == m_pFormShell) && (m_pFormPage == pNewPage))
        return;

    //////////////////////////////////////////////////////////////////////
    // Als Listener abmelden
    if( m_pFormShell )
    {
        if (m_pFormModel)
            EndListening( *m_pFormModel );
        m_pFormModel = NULL;
        EndListening( *m_pFormShell );
        Clear();
    }

    //////////////////////////////////////////////////////////////////////
    // Vollupdate
    m_pFormShell = pShell;
    if (m_pFormShell)
    {
        m_pFormPage = pNewPage;
        Update(m_pFormPage->GetForms());
    } else
        m_pFormPage = NULL;

    //////////////////////////////////////////////////////////////////////
    // Als Listener neu anmelden
    if( m_pFormShell )
    {
        StartListening( *m_pFormShell );
        m_pFormModel = m_pFormShell->GetFormModel();
        if( m_pFormModel )
            StartListening( *m_pFormModel );
    }
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  FmExplorerModel::GetFormComponents( FmFormData* pFormData )
{
    //////////////////////////////////////////////////////////////////////
    // Von der ::com::sun::star::form::Form ::com::sun::star::frame::Components holen
    if (pFormData)
        return ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pFormData->GetFormIface(), ::com::sun::star::uno::UNO_QUERY);

    return ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ();
}

//------------------------------------------------------------------------
sal_Bool FmExplorerModel::CheckEntry( FmEntryData* pEntryData )
{
    //////////////////////////////////////////////////////////////////////
    // Nur ::com::sun::star::form::Forms duerfen auf Doppeldeutigkeit untersucht werden
    if( !pEntryData->ISA(FmFormData) ) return sal_True;

    //////////////////////////////////////////////////////////////////////
    // ChildListe des Parents holen
    FmFormData* pParentData = (FmFormData*)pEntryData->GetParent();
    FmEntryDataList* pChildList;
    if( !pParentData )
        pChildList = GetRootList();
    else
        pChildList = pParentData->GetChildList();

    //////////////////////////////////////////////////////////////////////
    // In ChildListe nach doppelten Namen suchen
    ::rtl::OUString aChildText;
    FmEntryData* pChildData;

    for( sal_uInt16 i=0; i<pChildList->Count(); i++ )
    {
        pChildData = pChildList->GetObject(i);
        aChildText = pChildData->GetText();

        //////////////////////////////////////////////////////////////////////
        // Gleichen Eintrag gefunden
        if( aChildText.equalsIgnoreCase(pEntryData->GetText()) &&
            (pEntryData!=pChildData) )
        {
            SvxDBMsgBox aErrorBox( GetpApp()->GetAppWindow(), SVX_RES(RID_ERR_CONTEXT_ADDFORM),
                SVX_RES(RID_ERR_DUPLICATE_NAME), WB_OK | WB_DEF_OK, SvxDBMsgBox::Error );
            aErrorBox.Execute();

            return sal_False;
        }
    }

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool FmExplorerModel::Rename( FmEntryData* pEntryData, const ::rtl::OUString& rNewText )
{
    //////////////////////////////////////////////////////////////////////
    // Wenn Name schon vorhanden, Fehlermeldung
    pEntryData->SetText( rNewText );

    //////////////////////////////////////////////////////////////////////
    // PropertySet besorgen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent;

    if( pEntryData->ISA(FmFormData) )
    {
        FmFormData* pFormData = (FmFormData*)pEntryData;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm( pFormData->GetFormIface());
        xFormComponent = ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > (xForm, ::com::sun::star::uno::UNO_QUERY);
    }

    if( pEntryData->ISA(FmControlData) )
    {
        FmControlData* pControlData = (FmControlData*)pEntryData;
        xFormComponent = pControlData->GetFormComponent();
    }

    if( !xFormComponent.is() ) return sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xFormComponent, ::com::sun::star::uno::UNO_QUERY);
    if( !xSet.is() ) return sal_False;

    //////////////////////////////////////////////////////////////////////
    // Namen setzen
    xSet->setPropertyValue( FM_PROP_NAME, ::com::sun::star::uno::makeAny(rNewText) );

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool FmExplorerModel::IsNameAlreadyDefined( const ::rtl::OUString& rName, FmFormData* pParentData )
{
    //////////////////////////////////////////////////////////////////////
    // ::com::sun::star::form::Form in der Root
    if( !pParentData )
    {
        if (GetForms()->hasByName(rName))
            return sal_True;
    }

    //////////////////////////////////////////////////////////////////////
    // Restliche ::com::sun::star::frame::Components
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  xFormComponents(GetFormComponents(pParentData), ::com::sun::star::uno::UNO_QUERY);
        if (xFormComponents.is() && xFormComponents->hasByName(rName))
            return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------
SdrObject* FmExplorerModel::GetSdrObj( FmControlData* pControlData )
{
    if (!pControlData || !m_pFormShell)
        return NULL;

    //////////////////////////////////////////////////////////////////////
    // In der Page das entsprechende SdrObj finden und selektieren
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent( pControlData->GetFormComponent());
    if (!xFormComponent.is())
        return NULL;

    FmFormView*     pFormView       = m_pFormShell->GetFormView();
    SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
    SdrPage*        pPage           = pPageView->GetPage();

    SdrObjListIter  aIter( *pPage );
    return Search(aIter, xFormComponent);
}

//------------------------------------------------------------------
SdrObject* FmExplorerModel::Search(SdrObjListIter& rIter, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xComp)
{
    while (rIter.IsMore())
    {
        SdrObject* pObj = rIter.Next();
        //////////////////////////////////////////////////////////////////////
        // Es interessieren nur Uno-Objekte
        if (pObj->GetObjInventor() == FmFormInventor)
        {                                           //////////////////////////////////////////////////////////////////////
            // Ist dieses Objekt ein ::com::sun::star::form::XFormComponent?
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormViewControl(((SdrUnoObj*)pObj)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
            if (xFormViewControl == xComp)
                return pObj;
        }
        else if (pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj->GetSubList());
            pObj = Search(aIter, xComp);
            if (pObj)
                return pObj;
        }
    }
    return NULL;
}

//========================================================================
// class FmExplorer
//========================================================================

//------------------------------------------------------------------------
FmExplorer::FmExplorer( Window* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER )
    ,nEditEvent(0)
    ,m_pEditEntry(NULL)
    ,m_nSelectLock(0)
    ,m_aTimerTriggered(-1,-1)
    ,m_ilNavigatorImages( SVX_RES(RID_SVXIMGLIST_FMEXPL) )
    ,m_bRootSelected(sal_False)
    ,m_nFormsSelected(0)
    ,m_nControlsSelected(0)
    ,m_nHiddenControls(0)
    ,m_bPrevSelectionMixed(sal_False)
    ,m_bInitialUpdate(sal_True)
    ,m_bMarkingObjects(sal_False)
    ,m_sdiState(SDI_DIRTY)
    ,m_bShellOrPageChanged(sal_False)
    ,m_bDragDataDirty(sal_False)
    ,m_pRootEntry(NULL)
{
    SetHelpId( HID_FORM_NAVIGATOR );

    m_aRootImg = m_ilNavigatorImages.GetImage( RID_SVXIMG_FORMS );
    Image m_aCollapsedNodeImg = m_ilNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE );
    Image m_aExpandedNodeImg = m_ilNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE );
    SetNodeBitmaps( m_aCollapsedNodeImg, m_aExpandedNodeImg );

    EnableDrop();
    SetDragDropMode(0xFFFF);
    EnableInplaceEditing( sal_True );
    SetSelectionMode(MULTIPLE_SELECTION);

    m_pExplModel = new FmExplorerModel(m_ilNavigatorImages);
    Clear();

    StartListening( *m_pExplModel );

    m_aDropActionTimer.SetTimeoutHdl(LINK(this, FmExplorer, OnDropActionTimer));

    m_aSynchronizeTimer.SetTimeoutHdl(LINK(this, FmExplorer, OnSynchronizeTimer));
    SetSelectHdl(LINK(this, FmExplorer, OnEntrySelDesel));
    SetDeselectHdl(LINK(this, FmExplorer, OnEntrySelDesel));
}

//------------------------------------------------------------------------
FmExplorer::~FmExplorer()
{
    if( nEditEvent )
        Application::RemoveUserEvent( nEditEvent );

    if (m_aSynchronizeTimer.IsActive())
        m_aSynchronizeTimer.Stop();

    DBG_ASSERT(GetExplModel() != NULL, "FmExplorer::~FmExplorer : unerwartet : kein ExplorerModel");
    EndListening( *m_pExplModel );
    Clear();
    delete m_pExplModel;
}

//------------------------------------------------------------------------
void FmExplorer::Clear()
{
    m_pExplModel->Clear();
}

//------------------------------------------------------------------------
void FmExplorer::Update( FmFormShell* pFormShell )
{
    if (m_bInitialUpdate)
    {
        GrabFocus();
        m_bInitialUpdate = sal_False;
    }

    FmFormShell* pOldShell = GetExplModel()->GetFormShell();
    FmFormPage* pOldPage = GetExplModel()->GetFormPage();
    FmFormPage* pNewPage = pFormShell ? pFormShell->GetCurPage() : NULL;

    if ((pOldShell != pFormShell) || (pOldPage != pNewPage))
    {
        // neue Shell, waehrend ich gerade editiere ?
        if (IsEditingActive())
            CancelTextEditing();

        m_bShellOrPageChanged = sal_True;
        m_bDragDataDirty = sal_True;    // sicherheitshalber, auch wenn ich gar nicht dragge
    }
    GetExplModel()->Update( pFormShell );

    // wenn es eine ::com::sun::star::form::Form gibt, die Root expandieren
    if (m_pRootEntry && !IsExpanded(m_pRootEntry))
        Expand(m_pRootEntry);
    // wenn es GENAU eine ::com::sun::star::form::Form gibt, auch diese expandieren
    if (m_pRootEntry)
    {
        SvLBoxEntry* pFirst = FirstChild(m_pRootEntry);
        if (pFirst && !NextSibling(pFirst))
            Expand(pFirst);
    }
}

//------------------------------------------------------------------------------
void FmExplorer::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch( rEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
        {
            EndSelection();
            Pointer aMovePtr( POINTER_COPYDATA ),
                    aCopyPtr( POINTER_COPYDATA ),
                    aLinkPtr( POINTER_LINKDATA );

            SvLBoxEntry* pCurEntry = GetCurEntry();

            // die Informationen fuer das QueryDrop und Drop
            CollectSelectionData(SDI_ALL);
            if (!m_arrCurrentSelection.Count())
                // nothing to do
                return;
            vector<SvLBoxEntry*> lstToDrag;
            int i;
            for (i=0; i<m_arrCurrentSelection.Count(); i++)
                lstToDrag.push_back(m_arrCurrentSelection[i]);
            SvxFmExplCtrlExch* pCtrlExch = new SvxFmExplCtrlExch(lstToDrag, GetExplModel()->GetFormShell(), GetExplModel()->GetFormPage());
            pCtrlExch->BuildPathFormat(this, m_pRootEntry);

            // testen, ob es sich vielleicht ausschliesslich um hidden controls handelt (dann koennte ich pCtrlExch noch ein
            // zusaetzliches Format geben)
            sal_Bool bHasNonHidden = sal_False;
            for (i=0; i<m_arrCurrentSelection.Count(); i++)
            {
                FmEntryData* pCurrent = (FmEntryData*)(m_arrCurrentSelection[i]->GetUserData());
                if (IsHiddenControl(pCurrent))
                    continue;
                bHasNonHidden = sal_True;
                break;
            }
            if (!bHasNonHidden)
            {
                // eine entsprechende Sequenz aufbauen
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqIFaces(m_arrCurrentSelection.Count());
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > * pArray = seqIFaces.getArray();
                for (i=0; i<m_arrCurrentSelection.Count(); i++)
                    pArray[i] = ((FmEntryData*)(m_arrCurrentSelection[i]->GetUserData()))->GetElement();

                // und das neue Format
                pCtrlExch->AddHiddenControlsFormat(seqIFaces);
            }

            // jetzt haben wir alle in der aktuelle Situation moeglichen Formate eingesammelt, es kann also losgehen ...
            m_bShellOrPageChanged = m_bDragDataDirty = sal_False;
            short nDragResult = pCtrlExch->ExecuteDrag( this, aMovePtr, aCopyPtr, aLinkPtr, DRAG_MOVEABLE | DRAG_COPYABLE );
            if (nDragResult == DROP_CANCEL)
            {
                if (!m_bShellOrPageChanged)   // wenn die Shell (oder Page) umgeschaltet wurde, ist pCurEntry nicht mehr gueltig
                {
                    SetCursor(pCurEntry, sal_True);
                    MakeVisible(pCurEntry);
                }
            }
            bHandled = sal_True;
            break;
        }

        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            Point ptWhere;
            if (rEvt.IsMouseEvent())
            {
                ptWhere = rEvt.GetMousePosPixel();
                SvLBoxEntry* ptClickedOn = GetEntry(ptWhere);
                if (ptClickedOn == NULL)
                    break;
                if ( !IsSelected(ptClickedOn) )
                {
                    SelectAll(sal_False);
                    Select(ptClickedOn, sal_True);
                    SetCurEntry(ptClickedOn);
                }
            } else
            {
                if (m_arrCurrentSelection.Count() == 0) // kann nur bei Kontextmenue ueber Tastatur passieren
                    break;

                SvLBoxEntry* pCurrent = GetCurEntry();
                if (!pCurrent)
                    break;
                ptWhere = GetEntryPos(pCurrent);
            }

            // meine Selektionsdaten auf den aktuellen Stand
            CollectSelectionData(SDI_ALL);

            // wenn mindestens ein Nicht-Root-Eintrag selektiert ist und die Root auch, dann nehme ich letztere aus der Selektion
            // fix wieder raus
            if ( (m_arrCurrentSelection.Count() > 1) && m_bRootSelected )
            {
                Select( m_pRootEntry, sal_False );
                SetCursor( m_arrCurrentSelection.GetObject(0), sal_True);
            }
            sal_Bool bSingleSelection = (m_arrCurrentSelection.Count() == 1);


            DBG_ASSERT( (m_arrCurrentSelection.Count() > 0) | m_bRootSelected, "keine Eintraege selektiert" );
                // solte nicht passieren, da ich oben bei der IsSelected-Abfrage auf jeden Fall einen selektiert haette,
                // wenn das vorher nicht der Fall gewesen waere


            // das Menue zusammenbasteln
            FmFormShell* pFormShell = GetExplModel()->GetFormShell();
            FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
            if( pFormShell && pFormModel )
            {
                PopupMenu aContextMenu(SVX_RES(RID_FMEXPLORER_POPUPMENU));
                PopupMenu* pSubMenuNew = aContextMenu.GetPopupMenu( SID_FM_NEW );

                // das 'Neu'-Untermenue gibt es nur, wenn genau die Root oder genau ein Formular selektiert sind
                aContextMenu.EnableItem( SID_FM_NEW, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );

                // 'Neu'\'Formular' unter genau den selben Bedingungen
                pSubMenuNew->EnableItem( SID_FM_NEW_FORM, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );
                pSubMenuNew->SetItemImage(SID_FM_NEW_FORM, m_ilNavigatorImages.GetImage(RID_SVXIMG_FORM));
                pSubMenuNew->SetItemImage(SID_FM_NEW_HIDDEN, m_ilNavigatorImages.GetImage(RID_SVXIMG_HIDDEN));

                // 'Neu'\'verstecktes...', wenn genau ein Formular selektiert ist
                pSubMenuNew->EnableItem( SID_FM_NEW_HIDDEN, bSingleSelection && m_nFormsSelected );

                // 'Loeschen' : alles, was nicht Root ist, darf geloescht werden
                aContextMenu.EnableItem( SID_FM_DELETE, !m_bRootSelected );

                // der TabDialog, wenn es genau ein Formular ist ...
                aContextMenu.EnableItem( SID_FM_TAB_DIALOG, bSingleSelection && m_nFormsSelected );

                // Properties, wenn es nur Controls oder genau ein Formular ist
                // (und der ProBrowser nicht sowieso schon offen ist)
                if( pFormShell->GetImpl()->IsPropBrwOpen() )
                    aContextMenu.RemoveItem( aContextMenu.GetItemPos(SID_FM_SHOW_PROPERTY_BROWSER) );
                else
                    aContextMenu.EnableItem( SID_FM_SHOW_PROPERTY_BROWSER,
                        (m_nControlsSelected && !m_nFormsSelected) || (!m_nControlsSelected && m_nFormsSelected) );

                // Umbenennen gdw wenn ein Element und nicht die Root
                aContextMenu.EnableItem( SID_FM_RENAME_OBJECT, bSingleSelection && !m_bRootSelected );

                // der Reandonly-Eintrag ist nur auf der Root erlaubt
                aContextMenu.EnableItem( SID_FM_OPEN_READONLY, m_bRootSelected );

                // die ConvertTo-Slots sind enabled, wenn genau ein Control selektiert ist, der
                // dem Control entsprechende Slot ist disabled
                if (!m_bRootSelected && !m_nFormsSelected && (m_nControlsSelected == 1))
                {
                    aContextMenu.SetPopupMenu( SID_FM_CHANGECONTROLTYPE, FmXFormShell::GetConversionMenu() );
                    FmControlData* pCurrent = (FmControlData*)(m_arrCurrentSelection[0]->GetUserData());
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCurrentProps(pCurrent->GetFormComponent(), ::com::sun::star::uno::UNO_QUERY);
                    pFormShell->GetImpl()->CheckControlConversionSlots(xCurrentProps, *aContextMenu.GetPopupMenu(SID_FM_CHANGECONTROLTYPE));
                        // die Shell filtert nach weiteren Bedingungen, zum Beispiel kein 'hidden control'
                }
                else
                    aContextMenu.EnableItem( SID_FM_CHANGECONTROLTYPE, sal_False );

                // jetzt alles, was disabled wurde, wech
                aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
                //////////////////////////////////////////////////////////
                // OpenReadOnly setzen

                sal_Bool bOpenDesignMode = pFormModel->GetOpenInDesignMode();
                aContextMenu.CheckItem( SID_FM_OPEN_READONLY, bOpenDesignMode );

                sal_uInt16 nSlotId = aContextMenu.Execute( this, ptWhere );
                switch( nSlotId )
                {
                    case SID_FM_NEW_FORM:
                    {
                        XubString aStr(SVX_RES(RID_STR_FORM));
                        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                        aUndoStr.SearchAndReplace('#', aStr);

                        pFormModel->BegUndo(aUndoStr);
                        // der Slot war nur verfuegbar, wenn es genau einen selektierten Eintrag gibt und dieser die Root
                        // oder ein Formular ist
                        NewForm( m_arrCurrentSelection.GetObject(0) );
                        pFormModel->EndUndo();

                    }   break;
                    case SID_FM_NEW_HIDDEN:
                    {
                        XubString aStr(SVX_RES(RID_STR_CONTROL));
                        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                        aUndoStr.SearchAndReplace('#', aStr);

                        pFormModel->BegUndo(aUndoStr);
                        // dieser Slot war guletig bei (genau) einem selektierten Formular
                        rtl::OUString fControlName = FM_COMPONENT_HIDDEN;
                        NewControl( fControlName, m_arrCurrentSelection.GetObject(0) );
                        pFormModel->EndUndo();

                    }   break;
                    case SID_FM_DELETE:
                    {
                        DeleteSelection();
                    }
                    break;
                    case SID_FM_TAB_DIALOG:
                    {
                        // dieser Slot galt bei genau einem selektierten Formular
                        SvLBoxEntry* pSelectedForm = m_arrCurrentSelection.GetObject(0);
                        DBG_ASSERT( IsFormEntry(pSelectedForm), "FmExplorer::Command: Dieser Eintrag muss ein FormEntry sein." );

                        FmFormData* pFormData = (FmFormData*)pSelectedForm->GetUserData();
                        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(  pFormData->GetFormIface());

                        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  xTabController(xForm, ::com::sun::star::uno::UNO_QUERY);
                        if( !xTabController.is() ) break;
                        FmTabOrderDlg aTabDlg( GetpApp()->GetAppWindow(), GetExplModel()->GetFormShell() );
                        aTabDlg.Execute();

                    }
                    break;
                    case SID_FM_SHOW_PROPERTY_BROWSER:
                    {
                        ShowSelectionProperties(sal_True);
                    }
                    break;
                    case SID_FM_RENAME_OBJECT:
                    {
                        // das war bei genau einem Nicht-Root-Eintrag erlaubt
                        EditEntry( m_arrCurrentSelection.GetObject(0) );
                    }
                    break;
                    case SID_FM_OPEN_READONLY:
                    {
                        if( pFormModel )
                        {
                            sal_Bool bOpenDesignMode = pFormModel->GetOpenInDesignMode();
                            pFormModel->SetOpenInDesignMode( !bOpenDesignMode );
                            SFX_BINDINGS().Invalidate(SID_FM_OPEN_READONLY);
                        }
                    }
                    break;
                    default:
                        if (pFormShell->GetImpl()->IsControlConversionSlot(nSlotId))
                        {
                            FmControlData* pCurrent = (FmControlData*)(m_arrCurrentSelection[0]->GetUserData());
                            if (pFormShell->GetImpl()->ExecuteControlConversionSlot(pCurrent->GetFormComponent(), nSlotId))
                                ShowSelectionProperties();
                        }
                }
            }
            bHandled = sal_True;
        } break;
    }

    if (!bHandled)
        SvTreeListBox::Command( rEvt );
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::IsDeleteAllowed()
{
    //////////////////////////////////////////////////////////////////////
    // Haben wir eine ::com::sun::star::form::Form...
    SvLBoxEntry* pCurEntry = GetCurEntry();
    sal_uInt32 nCurEntryPos = GetModel()->GetAbsPos( pCurEntry );

    if( nCurEntryPos==0 )           // Root kann nicht geloescht werden
        return sal_False;
    else
        return IsFormEntry(pCurEntry) || IsFormComponentEntry(pCurEntry);
}

//------------------------------------------------------------------------
SvLBoxEntry* FmExplorer::FindEntry( FmEntryData* pEntryData )
{
    if( !pEntryData ) return NULL;
    SvLBoxEntry* pCurEntry = First();
    FmEntryData* pCurEntryData;
    while( pCurEntry )
    {
        pCurEntryData = (FmEntryData*)pCurEntry->GetUserData();
        if( pCurEntryData && pCurEntryData->IsEqualWithoutChilds(pEntryData) )
            return pCurEntry;

        pCurEntry = Next( pCurEntry );
    }

    return NULL;
}

//------------------------------------------------------------------------
void FmExplorer::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA(FmExplRemovedHint) )
    {
        FmExplRemovedHint* pRemovedHint = (FmExplRemovedHint*)&rHint;
        FmEntryData* pEntryData = pRemovedHint->GetEntryData();
        Remove( pEntryData );
    }

    else if( rHint.ISA(FmExplInsertedHint) )
    {
        FmExplInsertedHint* pInsertedHint = (FmExplInsertedHint*)&rHint;
        FmEntryData* pEntryData = pInsertedHint->GetEntryData();
        sal_uInt32 nRelPos = pInsertedHint->GetRelPos();
        Insert( pEntryData, nRelPos );
    }

    else if( rHint.ISA(FmExplModelReplacedHint) )
    {
        FmEntryData* pData = ((FmExplModelReplacedHint*)&rHint)->GetEntryData();
        SvLBoxEntry* pEntry = FindEntry( pData );
        if (pEntry)
        {   // das Image neu setzen
            SetCollapsedEntryBmp(pEntry, pData->GetCollapsedImage());
            SetExpandedEntryBmp(pEntry, pData->GetExpandedImage());
        }
    }

    else if( rHint.ISA(FmExplNameChangedHint) )
    {
        FmExplNameChangedHint* pNameChangedHint = (FmExplNameChangedHint*)&rHint;
        SvLBoxEntry* pEntry = FindEntry( pNameChangedHint->GetEntryData() );
        SetEntryText( pEntry, pNameChangedHint->GetNewName() );
    }

    else if( rHint.ISA(FmExplClearedHint) )
    {
        SvTreeListBox::Clear();

        //////////////////////////////////////////////////////////////////////
        // Default-Eintrag "Formulare"
        m_pRootEntry = InsertEntry( SVX_RES(RID_STR_FORMS), m_aRootImg, m_aRootImg,
            NULL, sal_False, 0, NULL );
    }
    else if (!m_bMarkingObjects && rHint.ISA(FmExplRequestSelectHint))
    {   // wenn m_bMarkingObjects sal_True ist, markiere ich gerade selber Objekte, und da der ganze Mechanismus dahinter synchron ist,
        // ist das genau der Hint, der durch mein Markieren ausgeloest wird, also kann ich ihn ignorieren
        FmExplRequestSelectHint* pershHint = (FmExplRequestSelectHint*)&rHint;
        FmEntryDataArray& arredToSelect = pershHint->GetItems();
        SynchronizeSelection(arredToSelect);

        if (pershHint->IsMixedSelection())
            // in diesem Fall habe ich alles deselektiert, obwohl die ::com::sun::star::sdbcx::View u.U. eine gemischte Markierung hatte
            // ich muss also im naechsten Select den Navigator an die ::com::sun::star::sdbcx::View anpassen
            m_bPrevSelectionMixed = sal_True;
    }
}

//------------------------------------------------------------------------
SvLBoxEntry* FmExplorer::Insert( FmEntryData* pEntryData, sal_uInt32 nRelPos )
{
    //////////////////////////////////////////////////////////////////////
    // Aktuellen Eintrag einfuegen
    SvLBoxEntry* pParentEntry = FindEntry( pEntryData->GetParent() );
    SvLBoxEntry* pNewEntry;

    if( !pParentEntry )
        pNewEntry = InsertEntry( pEntryData->GetText(),
            pEntryData->GetExpandedImage(), pEntryData->GetCollapsedImage(),
            m_pRootEntry, sal_False, nRelPos, pEntryData );

    else
        pNewEntry = InsertEntry( pEntryData->GetText(),
            pEntryData->GetExpandedImage(), pEntryData->GetCollapsedImage(),
            pParentEntry, sal_False, nRelPos, pEntryData );

    //////////////////////////////////////////////////////////////////////
    // Wenn Root-Eintrag Root expandieren
    if( !pParentEntry )
        Expand( m_pRootEntry );

    //////////////////////////////////////////////////////////////////////
    // Childs einfuegen
    FmEntryDataList* pChildList = pEntryData->GetChildList();
    sal_uInt32 nChildCount = pChildList->Count();
    FmEntryData* pChildData;
    for( sal_uInt32 i=0; i<nChildCount; i++ )
    {
        pChildData = pChildList->GetObject(i);
        Insert( pChildData, LIST_APPEND );
    }

    return pNewEntry;
}

//------------------------------------------------------------------------
void FmExplorer::Remove( FmEntryData* pEntryData )
{
    if( !pEntryData )
        return;

    // der Entry zu den Daten
    SvLBoxEntry* pEntry = FindEntry( pEntryData );
    if (!pEntry)
        return;

    // Eintrag aus TreeListBox entfernen
    // ich darf das Select, das ich ausloese, nicht behandeln :
    // Select aendert die MarkList der ::com::sun::star::sdbcx::View, wenn das gerade auch jemand anders macht und dabei ein Remove
    // triggert, haben wir mit ziemlicher Sicherheit ein Problem - Paradebeispiel war das Gruppieren von Controls mit
    // offenem Navigator ...)
    LockSelectionHandling();

    // ein kleines Problem : ich merke mir meine selektierten Daten, wenn mir jetzt jemand einen selektierten Eintrag
    // unter dem Hintern wegschiesst, werde ich inkonsistent ... was schlecht waere
    Select(pEntry, sal_False);

    // beim eigentlichen Entfernen kann die ::com::sun::star::awt::Selection geaendert werden, da ich aber das SelectionHandling abgeschaltet
    // habe, muss ich mich hinterher darum kuemmern
    int nExpectedSelectionCount = GetSelectionCount();

    if( pEntry )
        GetModel()->Remove( pEntry );

    if (nExpectedSelectionCount != GetSelectionCount())
        SynchronizeSelection();

    // und standardmaessig behandle ich das Select natuerlich
    UnlockSelectionHandling();
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::IsFormEntry( SvLBoxEntry* pEntry )
{
    FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
    return !pEntryData || pEntryData->ISA(FmFormData);
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::IsFormComponentEntry( SvLBoxEntry* pEntry )
{
    FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
    return pEntryData && pEntryData->ISA(FmControlData);
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::QueryDrop( DropEvent& rDEvt )
{
    Point aDropPos = rDEvt.GetPosPixel();

    // kuemmern wir uns erst mal um moeglich DropActions (Scrollen und Aufklappen)
    if (rDEvt.IsLeaveWindow())
    {
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();
    } else
    {
        sal_Bool bNeedTrigger = sal_False;
        // auf dem ersten Eintrag ?
        if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLUP;
            bNeedTrigger = sal_True;
        } else
            // auf dem letzten (bzw. in dem Bereich, den ein Eintrag einnehmen wuerde, wenn er unten genau buendig
            // abschliessen wuerde) ?
            if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLDOWN;
                bNeedTrigger = sal_True;
            } else
            {   // auf einem Entry mit Childs, der nicht aufgeklappt ist ?
                SvLBoxEntry* pDropppedOn = GetEntry(aDropPos);
                if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                {
                    // -> aufklappen
                    m_aDropActionType = DA_EXPANDNODE;
                    bNeedTrigger = sal_True;
                }
            }

        if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
        {
            // neu anfangen zu zaehlen
            m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
            // die Pos merken, da ich auch QueryDrops bekomme, wenn sich die Maus gar nicht bewegt hat
            m_aTimerTriggered = aDropPos;
            // und den Timer los
            if (!m_aDropActionTimer.IsActive()) // gibt es den Timer schon ?
            {
                m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                m_aDropActionTimer.Start();
            }
        } else if (!bNeedTrigger)
            m_aDropActionTimer.Stop();
    }


    //////////////////////////////////////////////////////////////////////
    // Hat das Object das richtige Format?
    SvDataObjectRef xDataObj( SvDataObject::PasteDragServer( rDEvt ));

    const SvDataTypeList& rTypeList = xDataObj->GetTypeList();
    sal_Bool bHasDefControlFormat = NULL != rTypeList.Get(Exchange::RegisterFormatName(SVX_FM_CONTROL_EXCH));
    sal_Bool bHasControlPathFormat = NULL != rTypeList.Get(Exchange::RegisterFormatName(SVX_FM_CONTROLS_AS_PATH));
    sal_Bool bHasHiddenControlsFormat = NULL != rTypeList.Get(Exchange::RegisterFormatName(SVX_FM_HIDDEN_CONTROLS));
    if (!bHasDefControlFormat && !bHasControlPathFormat && !bHasHiddenControlsFormat)
        return sal_False;

    // die Liste der gedroppten Eintraege aus dem DragServer
    SvxFmExplCtrlExch* xDragExch = (SvxFmExplCtrlExch*)&xDataObj;

    sal_Bool bForeignShellOrPage = xDragExch->GetShell() != GetExplModel()->GetFormShell()
        || xDragExch->GetPage() != GetExplModel()->GetFormPage();
    if (bForeignShellOrPage || (bHasHiddenControlsFormat && (rDEvt.GetAction() == DROP_COPY)))
    {
        // ueber Shells/Pages hinweg kann ich nur hidden Controls austauschen
        if (!bHasHiddenControlsFormat)
            return sal_False;

        SvLBoxEntry* pDropTarget = GetEntry(aDropPos);
        if (!pDropTarget || (pDropTarget == m_pRootEntry) || !IsFormEntry(pDropTarget))
            return sal_False;

        rDEvt.SetAction(DROP_COPY);     // hidden controls ueber Shell-Grenzen werden nur kopiert, nie verschoben
        return sal_True;
    }

    if (rDEvt.GetAction() != DROP_MOVE) // normale Control innerhalb einer Shell werden nur verschoben
        return sal_False;

    if (m_bDragDataDirty)
    {
        if (!bHasControlPathFormat)
            // ich befinde mich zwar in der Shell/Page, aus der die Controls stammen, habe aber kein Format, das den stattgefundenen
            // Shell-Wechsel ueberlebt hat (SVX_FM_CONTROLS_AS_PATH)
            return sal_False;

        // da die Shell waehrend des Draggens umgeschaltet wude, muss ich die Liste des ExchangeObjektes wieder neu aufbauen
        // (dort stehen SvLBoxEntries drin, und die sind bei der Umschaltung floeten gegangen)
        xDragExch->BuildListFromPath(this, m_pRootEntry);
        m_bDragDataDirty = sal_False;
    }

    // die Liste der gedroppten Eintraege aus dem DragServer
    vector<SvLBoxEntry*> lstDropped = xDragExch->GetDraggedEntries();
    DBG_ASSERT(lstDropped.size() >= 1, "FmExplorer::QueryDrop : keine Eintraege !");

    // das Ziel des Droppens (plus einige Daten, die ich nicht in jeder Schleife ermitteln will)
    SvLBoxEntry* pDropTarget = GetEntry( aDropPos );
    if (!pDropTarget)
        return sal_False;
    sal_Bool bDropTargetIsComponent = IsFormComponentEntry(pDropTarget);
    SvLBoxEntry* pDropTargetParent = GetParent( pDropTarget );

    // so, folgende Bedingungen, unter denen ich das Drop verbiete :
    // 0) die Root ist in der Liste mit drin
    // 1) einer der zu droppenden Eintraege soll in sein eigenes Parent gedroppt werden
    // 2) -               "               - wird auf sich selber gezogen
    // 3) -               "               - ist eine ::com::sun::star::form::Form und wird auf einen unter ihm stehenden Eintrag gezogen
    // 4) einer der Eintraege ist ein Control und wird auf die Root gedroppt
    // 5) ein Control oder Formular wird auf ein Control NICHT auf der selben Hierarchie-Ebene gezogen (auf eines der selben
    //      heisst Platz vertauschen, ist also erlaubt)

    // um 3) etwas fixer testen zu koennen, sammle ich ich die Vorfahren des DropTargets ein
    SvLBoxEntrySortedArray arrDropAnchestors;
    SvLBoxEntry* pLoop = pDropTarget;
    while (pLoop)
    {
        arrDropAnchestors.Insert(pLoop);
        pLoop = GetParent(pLoop);
    }

    for (int i=0; i<lstDropped.size(); i++)
    {
        SvLBoxEntry* pCurrent = lstDropped[i];
        SvLBoxEntry* pCurrentParent = GetParent(pCurrent);

        // Test auf 0)
        if (pCurrent == m_pRootEntry)
            return sal_False;

        // Test auf 1)
        if ( pDropTarget == pCurrentParent )
            return sal_False;

        // Test auf 2)
        if (pCurrent == pDropTarget)
            return sal_False;

        // Test auf 5)
//      if ( bDropTargetIsComponent && (pDropTargetParent != pCurrentParent) )
        if ( bDropTargetIsComponent )   // TODO : die obige Zeile wieder rein, dann muss aber Drop das Vertauschen auch beherrschen
            return sal_False;

        // Test auf 3)
        if ( IsFormEntry(pCurrent) )
        {
            sal_uInt16 nPosition;
            if ( arrDropAnchestors.Seek_Entry(pCurrent, &nPosition) )
                return sal_False;
        } else
            if ( IsFormComponentEntry(pCurrent) )
            {
                // Test auf 4)
                if (pDropTarget == m_pRootEntry)
                    return sal_False;
            }
    }

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::Drop( const DropEvent& rDEvt )
{
    // ware schlecht, wenn nach dem Droppen noch gescrollt wird ...
    if (m_aDropActionTimer.IsActive())
        m_aDropActionTimer.Stop();

    // Format-Ueberpruefung
    SvDataObjectRef xDataObj( SvDataObject::PasteDragServer( rDEvt ));
    SvxFmExplCtrlExchRef xDragExch( (SvxFmExplCtrlExch*)&xDataObj);

    sal_Bool bHasHiddenControlsFormat = NULL != xDataObj->GetTypeList().Get(Exchange::RegisterFormatName(SVX_FM_HIDDEN_CONTROLS));
#ifdef DBG_UTIL
    sal_Bool bForeignShellOrPage = xDragExch->GetShell() != GetExplModel()->GetFormShell()
        || xDragExch->GetPage() != GetExplModel()->GetFormPage();
    DBG_ASSERT(!bForeignShellOrPage || bHasHiddenControlsFormat, "FmExplorer::Drop : invalid format (QueryDrop shouldn't have let this pass) !");
    DBG_ASSERT(bForeignShellOrPage || !m_bDragDataDirty, "FmExplorer::Drop : invalid state (shell changed since last exchange resync) !");
        // das sollte in QueryDrop erledigt worden sein : dort wird in xDragExch die Liste der Controls aufgebaut und m_bDragDataDirty
        // zurueckgesetzt
#endif

    // das Ziel des Drop sowie einige Daten darueber
    Point aDropPos = rDEvt.GetPosPixel();
    SvLBoxEntry* pDropTarget = GetEntry( aDropPos );
    if (!pDropTarget)
        return sal_False;

//  sal_uInt32 nDropEntryPos = GetModel()->GetRelPos( pDropTarget );  // brauche ich spaeter mal fuer das Verschieben
    sal_Bool bDropTargetIsForm = IsFormEntry(pDropTarget);
    FmFormData* pTargetData = bDropTargetIsForm ? (FmFormData*)pDropTarget->GetUserData() : NULL;

    if (rDEvt.GetAction() == DROP_COPY)
    {
        DBG_ASSERT(bHasHiddenControlsFormat, "FmExplorer::Drop : only copying of hidden controls is supported !");
            // das sollte das QueryDrop abgefangen haben

        // da ich gleich die Zielobjekte alle selektieren will (und nur die)
        SelectAll(sal_False);

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqControls = xDragExch->GetHiddenControls();
        sal_Int32 nCount = seqControls.getLength();
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > * pControls = seqControls.getConstArray();

        FmFormShell* pFormShell = GetExplModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;

        // innerhalb eines Undo ...
        if (pFormModel)
        {
            XubString aStr(SVX_RES(RID_STR_CONTROL));
            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
            aUndoStr.SearchAndReplace('#', aStr);
            pFormModel->BegUndo(aUndoStr);
        }

        // die Conrtols kopieren
        for (sal_Int32 i=0; i<nCount; ++i)
        {
            // neues Control anlegen
            rtl::OUString fControlName = FM_COMPONENT_HIDDEN;
            FmControlData* pNewControlData = NewControl( fControlName, pDropTarget, sal_False);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xNewPropSet(pNewControlData->GetElement(), ::com::sun::star::uno::UNO_QUERY);

            // und die Properties des alten in das neue kopieren
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCurrent(pControls[i], ::com::sun::star::uno::UNO_QUERY);
#if DEBUG || DBG_UTIL
            // nur mal eben sehen, ob das Ding tatsaechlich ein hidden control ist
            sal_Int16 nClassId = ::utl::getINT16(xCurrent->getPropertyValue(FM_PROP_CLASSID));
            DBG_ASSERT(nClassId == ::com::sun::star::form::FormComponentType::HIDDENCONTROL, "FmExplorer::Drop : invalid control in drop list !");
                // wenn das SVX_FM_HIDDEN_CONTROLS-Format vorhanden ist, dann sollten wirklich nur hidden controls in der Sequenz
                // stecken
#endif // DEBUG || DBG_UTIL
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xPropInfo( xCurrent->getPropertySetInfo());
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> seqAllCurrentProps = xPropInfo->getProperties();
            ::com::sun::star::beans::Property* pAllCurrentProps = seqAllCurrentProps.getArray();
            for (sal_Int32 j=0; j<seqAllCurrentProps.getLength(); ++j)
            {
                ::rtl::OUString ustrCurrentProp = pAllCurrentProps[j].Name;
                if (((pAllCurrentProps[j].Attributes & ::com::sun::star::beans::PropertyAttribute::READONLY) == 0) && (ustrCurrentProp != FM_PROP_NAME))
                {   // (read-only attribs werden natuerlich nicht gesetzt, dito der Name, den hat das NewControl schon eindeutig
                    // festgelegt)
                    xNewPropSet->setPropertyValue(ustrCurrentProp, xCurrent->getPropertyValue(ustrCurrentProp));
                }
            }

            SvLBoxEntry* pToSelect = FindEntry(pNewControlData);
            Select(pToSelect, sal_True);
            if (i == 0)
                SetCurEntry(pToSelect);
        }

        if (pFormModel)
            pFormModel->EndUndo();
        return sal_True;
    }


    // die Liste der gedraggten Eintraege
    vector<SvLBoxEntry*> lstDropped = xDragExch->GetDraggedEntries();
    DBG_ASSERT(lstDropped.size() >= 1, "FmExplorer::Drop : keine Eintraege !");

    // die Shell und das Model
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
    if (!pFormModel)
        return sal_False;

    // fuer's Undo
    XubString strUndoDescription(SVX_RES(RID_STR_UNDO_CONTAINER_REPLACE));
        // TODO : den ::rtl::OUString aussagekraeftiger machen
    pFormModel->BegUndo(strUndoDescription);

    // ich nehme vor dem Einfuegen eines Eintrages seine ::com::sun::star::awt::Selection raus, damit die Markierung dabei nicht flackert
    // -> das Handeln des Select locken
    LockSelectionHandling();

    // jetzt durch alle gedroppten Eintraege ...
    for (int i=0; i<lstDropped.size(); ++i)
    {
        // ein paar Daten zum aktuellen Element
        SvLBoxEntry* pCurrent = lstDropped[i];
        DBG_ASSERT(pCurrent != NULL, "FmExplorer::Drop : ungueltiger Eintrag");
        DBG_ASSERT(GetParent(pCurrent) != NULL, "FmExplorer::Drop : ungueltiger Eintrag");
            // die Root darf nicht gedraggt werden

        FmEntryData* pCurrentUserData = (FmEntryData*)pCurrent->GetUserData();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xCurrentChild(pCurrentUserData->GetElement(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xCurrentChild->getParent(), ::com::sun::star::uno::UNO_QUERY);


        FmFormData* pCurrentParentUserData = (FmFormData*)pCurrentUserData->GetParent();
        DBG_ASSERT(pCurrentParentUserData == NULL || pCurrentParentUserData->ISA(FmFormData), "FmExplorer::Drop : ungueltiges Parent");

        // beim Vater austragen
        if (pCurrentParentUserData)
            pCurrentParentUserData->GetChildList()->Remove(pCurrentUserData);
        else
            GetExplModel()->GetRootList()->Remove(pCurrentUserData);

        // aus dem ::com::sun::star::sdbcx::Container entfernen
        sal_Int32 nIndex = getElementPos(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xContainer, ::com::sun::star::uno::UNO_QUERY), xCurrentChild);
        GetExplModel()->m_pPropChangeList->Lock();
        // die Undo-::com::sun::star::chaos::Action fuer das Rausnehmen
        if (GetExplModel()->m_pPropChangeList->CanUndo())
            pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Removed,
                                                        xContainer, xCurrentChild, nIndex));

        // Events mitkopieren
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContainer, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor > aEvts;

        if (xManager.is() && nIndex >= 0)
            aEvts = xManager->getScriptEvents(nIndex);
        xContainer->removeByIndex(nIndex);

        // die ::com::sun::star::awt::Selection raus
        Select(pCurrent, sal_False);
        // und weg
        Remove(pCurrentUserData);


        // die Stelle innerhalb des DropParents, an der ich die gedroppten Eintraege einfuegen muss
        if (pTargetData)
            xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pTargetData->GetElement(), ::com::sun::star::uno::UNO_QUERY);
        else
            xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (GetExplModel()->GetForms(), ::com::sun::star::uno::UNO_QUERY);

        // immer ganz hinten einfuegen
        nIndex = xContainer->getCount();

        // UndoAction fuer das Einfuegen
        if (GetExplModel()->m_pPropChangeList->CanUndo())
            pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Inserted,
                                                     xContainer, xCurrentChild, nIndex));

        // einfuegen im neuen ::com::sun::star::sdbcx::Container
        if (pTargetData)
        {
             // es wird in eine ::com::sun::star::form::Form eingefuegt, dann brauche ich eine ::com::sun::star::form::FormComponent
            xContainer->insertByIndex(nIndex,
                ::com::sun::star::uno::makeAny(::com::sun::star::uno::Reference<
                ::com::sun::star::form::XFormComponent>(xCurrentChild, ::com::sun::star::uno::UNO_QUERY)));
        }
        else
        {
            xContainer->insertByIndex(nIndex,
                ::com::sun::star::uno::makeAny(::com::sun::star::uno::Reference<
                ::com::sun::star::form::XForm>(xCurrentChild, ::com::sun::star::uno::UNO_QUERY)));
        }

        if (aEvts.getLength())
        {
            xManager = ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager > (xContainer, ::com::sun::star::uno::UNO_QUERY);
            if (xManager.is())
                xManager->registerScriptEvents(nIndex, aEvts);
        }

        GetExplModel()->m_pPropChangeList->UnLock();

        // zuerst dem Eintrag das neue Parent
        pCurrentUserData->SetParent(pTargetData);

        // dann dem Parent das neue Child
        if (pTargetData)
            pTargetData->GetChildList()->Insert(pCurrentUserData, nIndex);
        else
            GetExplModel()->GetRootList()->Insert(pCurrentUserData, nIndex);

        // dann bei mir selber bekanntgeben und neu selektieren
        SvLBoxEntry* pNew = Insert( pCurrentUserData, nIndex );
    }

    UnlockSelectionHandling();
    pFormModel->EndUndo();

    // waehrend des Verschiebens der Eintraege hat sich die Markierung der unterliegenden ::com::sun::star::sdbcx::View nicht geaendert (da sie mit der
    // logischen Seite nichts zu tun hat), wohl aber meine Selektion, die ich also wieder an der Markierung ausrichten muss
    SynchronizeSelection();

    return sal_True;
}

//------------------------------------------------------------------------
void FmExplorer::MouseButtonUp( const MouseEvent& rMEvt )
{
    SvTreeListBox::MouseButtonUp( rMEvt );
}

//------------------------------------------------------------------------
void FmExplorer::KeyInput(const KeyEvent& rKEvt)
{
    // loeschen ?
    if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE && !rKEvt.GetKeyCode().GetModifier())
    {
        DeleteSelection();
        return;
    }

    SvTreeListBox::KeyInput(rKEvt);
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection )
{
    if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
        return sal_False;

    return (pEntry && (pEntry->GetUserData() != NULL));
        // die Wurzel, die ich nicht umbenennen darf, hat als UserData NULL
}

//------------------------------------------------------------------------
void FmExplorer::NewForm( SvLBoxEntry* pParentEntry )
{
    //////////////////////////////////////////////////////////////////////
    // ParentFormData holen
    if( !IsFormEntry(pParentEntry) )
        return;

    FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();

    //////////////////////////////////////////////////////////////////////
    // Neue ::com::sun::star::form::Form erzeugen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xNewForm(::utl::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), ::com::sun::star::uno::UNO_QUERY);
    if (!xNewForm.is())
        return;

    FmFormData* pNewFormData = new FmFormData( xNewForm, m_ilNavigatorImages, pParentFormData );

    //////////////////////////////////////////////////////////////////////
    // Namen setzen
    ::rtl::OUString aName = GenerateName(pNewFormData);
    pNewFormData->SetText(aName);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPropertySet(xNewForm, ::com::sun::star::uno::UNO_QUERY);
    if (!xPropertySet.is())
        return;
    try
    {
        xPropertySet->setPropertyValue( FM_PROP_NAME, ::com::sun::star::uno::makeAny(aName) );
    // a form should always have the command type table as default
        xPropertySet->setPropertyValue( FM_PROP_COMMANDTYPE, ::com::sun::star::uno::makeAny(sal_Int32(::com::sun::star::sdb::CommandType::TABLE)));
    }
    catch(...)
    {
        DBG_ERROR("FmExplorer::NewForm : could not set esssential properties !");
    }


    //////////////////////////////////////////////////////////////////////
    // ::com::sun::star::form::Form einfuegen
    GetExplModel()->Insert( pNewFormData, LIST_APPEND, sal_True );

    //////////////////////////////////////////////////////////////////////
    // Neue ::com::sun::star::form::Form als aktive ::com::sun::star::form::Form setzen
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if( pFormShell )
    {
        pFormShell->GetImpl()->setCurForm( xNewForm );
        pFormShell->GetCurPage()->GetImpl()->setCurForm( xNewForm );

        SFX_BINDINGS().Invalidate(SID_FM_PROPERTIES,sal_True,sal_True);
    }
    GetExplModel()->SetModified();

    //////////////////////////////////////////////////////////////////////
    // In EditMode schalten
    SvLBoxEntry* pNewEntry = FindEntry( pNewFormData );
    EditEntry( pNewEntry );
}

//------------------------------------------------------------------------
FmControlData* FmExplorer::NewControl( const ::rtl::OUString& rServiceName, SvLBoxEntry* pParentEntry, sal_Bool bEditName )
{
    //////////////////////////////////////////////////////////////////////
    // ParentForm holen
    if (!GetExplModel()->GetFormShell())
        return NULL;
    if (!IsFormEntry(pParentEntry))
        return NULL;

    FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xParentForm( pParentFormData->GetFormIface());

    //////////////////////////////////////////////////////////////////////
    // Neue Component erzeugen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xNewComponent(::utl::getProcessServiceFactory()->createInstance(rServiceName), ::com::sun::star::uno::UNO_QUERY);
    if (!xNewComponent.is())
        return NULL;

    FmControlData* pNewFormControlData = new FmControlData( xNewComponent, m_ilNavigatorImages, pParentFormData );

    //////////////////////////////////////////////////////////////////////
    // Namen setzen
    FmFormView*     pFormView       = GetExplModel()->GetFormShell()->GetFormView();
    SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
    FmFormPage*     pPage           = (FmFormPage*)pPageView->GetPage();

    ::rtl::OUString sName = pPage->GetImpl()->setUniqueName( xNewComponent, xParentForm );

    pNewFormControlData->SetText( sName );

    //////////////////////////////////////////////////////////////////////
    // ::com::sun::star::form::FormComponent einfuegen
    GetExplModel()->Insert( pNewFormControlData, LIST_APPEND, sal_True );
    GetExplModel()->SetModified();

    if (bEditName)
    {
        //////////////////////////////////////////////////////////////////////
        // In EditMode schalten
        SvLBoxEntry* pNewEntry = FindEntry( pNewFormControlData );
        Select( pNewEntry, sal_True );
        EditEntry( pNewEntry );
    }

    return pNewFormControlData;
}

//------------------------------------------------------------------------
::rtl::OUString FmExplorer::GenerateName( FmEntryData* pEntryData )
{
    const sal_uInt16 nMaxCount = 99;
    ::rtl::OUString aNewName;

    //////////////////////////////////////////////////////////////////////
    // BasisNamen erzeugen
    UniString aBaseName;
    if( pEntryData->ISA(FmFormData) )
        aBaseName = SVX_RES( RID_STR_STDFORMNAME );

    else if( pEntryData->ISA(FmControlData) )
        aBaseName = SVX_RES( RID_STR_CONTROL_CLASSNAME );

    //////////////////////////////////////////////////////////////////////
    // Neuen Namen erstellen
    FmFormData* pFormParentData = (FmFormData*)pEntryData->GetParent();

    for( sal_Int32 i=0; i<nMaxCount; i++ )
    {
        aNewName = aBaseName;
        if( i>0 )
        {
            aNewName += ::rtl::OUString::createFromAscii(" ");
            aNewName += ::rtl::OUString::valueOf(i).getStr();
        }

        if( GetExplModel()->FindData(aNewName, pFormParentData,sal_False) == NULL )
            break;
    }

    return aNewName;
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText )
{
    if (EditingCanceled())
        return sal_True;

    GrabFocus();
    FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
    sal_Bool bRes = GetExplModel()->Rename( pEntryData, rNewText);
    if( !bRes )
    {
        m_pEditEntry = pEntry;
        nEditEvent = Application::PostUserEvent( LINK(this, FmExplorer, OnEdit) );
    } else
        SetCursor(pEntry, sal_True);

    return bRes;
}

//------------------------------------------------------------------------
IMPL_LINK( FmExplorer, OnEdit, void*, EMPTYARG )
{
    nEditEvent = 0;
    EditEntry( m_pEditEntry );
    m_pEditEntry = NULL;

    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( FmExplorer, OnDropActionTimer, void*, EMPTYARG )
{
    if (--m_aTimerCounter > 0)
        return 0L;

    if (m_aDropActionType == DA_EXPANDNODE)
    {
        SvLBoxEntry* pToExpand = GetEntry(m_aTimerTriggered);
        if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
            // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
            // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
            // aber ich denke, die BK sollte es auch so vertragen
            Expand(pToExpand);

        // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
        m_aDropActionTimer.Stop();
    } else
    {
        switch (m_aDropActionType)
        {
            case DA_SCROLLUP :
            {
                ScrollOutputArea(1);
                break;
            }

            case DA_SCROLLDOWN :
                ScrollOutputArea(-1);
                break;
        }

        m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
    }

    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK(FmExplorer, OnEntrySelDesel, FmExplorer*, pThis)
{
    m_sdiState = SDI_DIRTY;

    if (IsSelectionHandlingLocked())
        return 0L;

    if (m_aSynchronizeTimer.IsActive())
        m_aSynchronizeTimer.Stop();

    m_aSynchronizeTimer.SetTimeout(EXPLORER_SYNC_DELAY);
    m_aSynchronizeTimer.Start();

    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK(FmExplorer, OnSynchronizeTimer, void*, EMPTYARG)
{
    SynchronizeMarkList();
    return 0L;
}


//------------------------------------------------------------------------
void FmExplorer::ShowSelectionProperties(sal_Bool bForce)
{
    // zuerst brauche ich die FormShell
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if (!pFormShell)
        // keine Shell -> ich koennte kein curObject setzen -> raus
        return;

    CollectSelectionData(SDI_ALL);
    DBG_ASSERT( (m_nFormsSelected >= 0) && (m_nControlsSelected >= 0) && (m_nHiddenControls >= 0),
        "FmExplorer::ShowSelectionProperties : selection counter(s) invalid !");
    DBG_ASSERT( m_nFormsSelected + m_nControlsSelected + (m_bRootSelected ? 1 : 0) == m_arrCurrentSelection.Count(),
        "FmExplorer::ShowSelectionProperties : selection meta data invalid !");


    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterfaceSelected;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xFormSelected;
    sal_Bool bPropertiesAvailable = sal_True;
    sal_Bool bNeedSetCurControl = sal_True;

    if (m_bRootSelected)
        bPropertiesAvailable = sal_False;   // fuer die Root habe ich keine - weder einzeln noch in einer Gruppe - Properties
    else if (m_nFormsSelected + m_nControlsSelected == 0)   // keines der beiden sollte kleiner 0 sein !
        bPropertiesAvailable = sal_False;   // keine Selektion -> keine Properties
    else if (m_nFormsSelected * m_nControlsSelected != 0)
        bPropertiesAvailable = sal_False;   // gemischte Selektion -> keine Properties
    else
    {   // so, hier bin ich, wenn entweder nur ::com::sun::star::form::Forms oder nur Controls selektiert sind
       if (m_arrCurrentSelection.Count() == 1)
        {
            if (m_nFormsSelected > 0)
            {   // es ist genau eine ::com::sun::star::form::Form selektiert
                FmFormData* pFormData = (FmFormData*)m_arrCurrentSelection.GetObject(0)->GetUserData();
                xFormSelected = pFormData->GetFormIface();
                xInterfaceSelected = xFormSelected;
            } else
            {   // es ist genau ein Control selektiert (egal ob hidden oder normal)
                FmEntryData* pEntryData = (FmEntryData*)m_arrCurrentSelection.GetObject(0)->GetUserData();
                xInterfaceSelected = pEntryData->GetElement();
                pFormShell->GetImpl()->setCurControl(xInterfaceSelected);
                bNeedSetCurControl = sal_False;

                if (pEntryData->GetParent())
                    xFormSelected = ((FmFormData*)pEntryData->GetParent())->GetFormIface();
            }
        } else
        {   // wir haben eine MultiSelection, also muessen wir ein MultiSet dafuer aufbauen
            if (m_nFormsSelected > 0)
            {   // ... nur ::com::sun::star::form::Forms
                // erstmal die PropertySet-Interfaces der ::com::sun::star::form::Forms einsammeln
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > > seqForms(m_nFormsSelected);
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > * pPropSets = seqForms.getArray();
                for (int i=0; i<m_nFormsSelected; i++)
                {
                    FmFormData* pFormData = (FmFormData*)m_arrCurrentSelection.GetObject(i)->GetUserData();
                    pPropSets[i] = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (pFormData->GetElement(), ::com::sun::star::uno::UNO_QUERY);
                }
                // dann diese in ein MultiSet packen
                FmXMultiSet* pSelectionSet = new FmXMultiSet( seqForms );
                xInterfaceSelected = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ( (::com::sun::star::beans::XPropertySet*)pSelectionSet );
            }
            else
            {   // ... nur Controls
                if (m_nHiddenControls == m_nControlsSelected)
                {   // ein MultiSet fuer die Properties der hidden controls
                    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > > seqHiddenControls(m_nHiddenControls);
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > * pPropSets = seqHiddenControls.getArray();
                    FmEntryData* pParentData = ((FmEntryData*)m_arrCurrentSelection.GetObject(0)->GetUserData())->GetParent();
                    for (int i=0; i<m_nHiddenControls; i++)
                    {
                        FmEntryData* pEntryData = (FmEntryData*)m_arrCurrentSelection.GetObject(i)->GetUserData();
                        pPropSets[i] = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (pEntryData->GetElement(), ::com::sun::star::uno::UNO_QUERY);

                        if (pParentData && pParentData != pEntryData->GetParent())
                            pParentData = NULL;
                    }

                    // dann diese in ein MultiSet packen
                    FmXMultiSet* pSelectionSet = new FmXMultiSet( seqHiddenControls );
                    xInterfaceSelected = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ( (::com::sun::star::beans::XPropertySet*)pSelectionSet );
                    // und der Shell als aktuelles Objekt anzeigen
                    pFormShell->GetImpl()->setCurControl( xInterfaceSelected );
                    bNeedSetCurControl = sal_False;

                    if (pParentData)
                        // alle hidden controls gehoeren zu einer ::com::sun::star::form::Form
                        xFormSelected = ((FmFormData*)pParentData)->GetFormIface();
                }
                else if (m_nHiddenControls == 0)
                {   // nur normale Controls
                    // ein MultiSet aus der MarkList der ::com::sun::star::sdbcx::View aufbauen ...
                    const SdrMarkList& mlMarked = pFormShell->GetFormView()->GetMarkList();
                    FmXMultiSet* pSelectionSet = new FmXMultiSet( mlMarked );
                    xInterfaceSelected = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ( (::com::sun::star::beans::XPropertySet*)pSelectionSet );
                    pFormShell->GetImpl()->setCurControl( xInterfaceSelected );
                    bNeedSetCurControl = sal_False;

                    // jetzt das Formular setzen
                    sal_Bool bMixedWithFormControls;
                    xFormSelected = pFormShell->GetImpl()->DetermineCurForm(mlMarked,bMixedWithFormControls);
                } else
                    // gemischte Selektion aus hidden und normalen Controls -> keine Properties
                    bPropertiesAvailable = sal_False;
            }
        }

    }

    // um das Setzen des current Controls kann sich die Shell kuemmern (da gibt es einige Feinheiten, die ich hier nicht
    // neu implementieren moechte)
    if (bNeedSetCurControl)
        pFormShell->GetImpl()->DetermineSelection(pFormShell->GetFormView()->GetMarkList());
    // und dann meine ::com::sun::star::form::Form und mein SelObject
    pFormShell->GetImpl()->setSelObject(xInterfaceSelected);
    pFormShell->GetImpl()->setCurForm(xFormSelected);

    if (pFormShell->GetImpl()->IsPropBrwOpen() || bForce)
    {
        // und jetzt kann ich das Ganze dem PropertyBrowser uebergeben
        FmInterfaceItem aInterfaceItem( SID_FM_SHOW_PROPERTY_BROWSER, xInterfaceSelected );
        SFX_DISPATCHER().Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON,
            &aInterfaceItem, 0L );
    }
}

//------------------------------------------------------------------------
void FmExplorer::DeleteSelection()
{
    // die Root darf ich natuerlich nicht mitloeschen
    sal_Bool bRootSelected = IsSelected(m_pRootEntry);
    int nSelectionCount = GetSelectionCount();
    if (bRootSelected && (nSelectionCount > 1))     // die Root plus andere Elemente ?
        Select(m_pRootEntry, sal_False);                // ja -> die Root raus

    if ((nSelectionCount == 0) || bRootSelected)    // immer noch die Root ?
        return;                                     // -> sie ist das einzige selektierte -> raus

    DBG_ASSERT(!m_bPrevSelectionMixed, "FmExplorer::DeleteSelection() : loeschen nicht erlaubt wenn Markierung und Selektion nciht konsistent");

    // ich brauche unten das FormModel ...
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if (!pFormShell)
        return;
    FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
    if (!pFormModel)
        return;

    // die Undo-Beschreibung
    UniString aUndoStr = SVX_RES(RID_STR_UNDO_DELETE_LOGICAL);
/*  falls irgendwann mal Ordnung in das Undo kommt (so dass wir wirklich die aeusserste Klammer sind und beim Loeschen nur
//  eine, nicht zwei UndoActions erzeugen), koennen wir folgenden Code ja wieder benutzen ...
    if (m_arrCurrentSelection.Count() == 1)
    {
        aUndoStr = SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE);
        if (m_nFormsSelected)
            aUndoStr.SearchAndReplace("#", SVX_RES(RID_STR_FORM));
        else
            // bei selektierter Root war loeschen nicht erlaubt, also ist es ein Control
            aUndoStr.SearchAndReplace("#", SVX_RES(RID_STR_CONTROL));
    }
    else
    {
        aUndoStr = SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
        aUndoStr.SearchAndReplace("#", UniString(::rtl::OUString(m_arrCurrentSelection.Count())));
            // der Umweg ueber den ::rtl::OUString ist notwendig, da nur der den Constructor hat, der gleich eine Zahl formatiert
    }
*/

    // jetzt muss ich noch die DeleteList etwas absichern : wenn man ein Formular und ein abhaengiges
    // Element loescht - und zwar in dieser Reihenfolge - dann ist der SvLBoxEntryPtr des abhaengigen Elementes
    // natuerlich schon ungueltig, wenn es geloescht werden soll ... diesen GPF, den es dann mit Sicherheit gibt,
    // gilt es zu verhindern, also die 'normalisierte' Liste
    CollectSelectionData(SDI_NORMALIZED);

    // folgendes Problem : ich muss das ExplorerModel::Remove verwenden, da nur dieses sich um das korrekte Loeschen von ::com::sun::star::form::Form-
    // Objekten kuemmert. Andererseits muss ich die Controls selber ueber DeleteMarked loeschen (irgendwo im Writer gibt
    // es sonst Probleme). Wenn ich erst die Struktur, dann die Controls loesche, klappt das Undo nicht (da dann erst die Controls
    // eingefuegt werden, dann die Struktur, sprich ihr Parent-Formular). Andersrum sind die EntryDatas ungueltig, wenn ich die
    // Controls geloescht habe und zur Struktur gehe. Also muss ich die Formulare NACH den normalen Controls loeschen, damit sie
    // beim Undo VOR denen wiederhergestellt werden.
    pFormShell->GetImpl()->EnableTrackProperties(sal_False);
    int i;
    for (i = m_arrCurrentSelection.Count()-1; i>=0; --i)
    {
        FmEntryData* pCurrent = (FmEntryData*)(m_arrCurrentSelection.GetObject(i)->GetUserData());

        // eine ::com::sun::star::form::Form ?
        sal_Bool bIsForm = pCurrent->ISA(FmFormData);

        // da ich das Loeschen im folgenden der ::com::sun::star::sdbcx::View ueberlasse und dabei auf deren MarkList aufbaue, im Normalfall aber bei
        // einem makierten Formular nur die direkt, nicht die indirekt abhaengigen Controls markiert werden, muss ich das hier
        // noch nachholen
        if (bIsForm)
            MarkViewObj((FmFormData*)pCurrent, sal_True, sal_True);     // das zweite sal_True heisst "deep"


        // ein hidden control ?
        sal_Bool bIsHidden = IsHiddenControl(pCurrent);

        // ::com::sun::star::form::Forms und hidden Controls muss ich behalten, alles andere nicht
        if (!bIsForm && !bIsHidden)
            m_arrCurrentSelection.Remove((sal_uInt16)i, 1);
    }
    pFormShell->GetImpl()->EnableTrackProperties(sal_True);

    // und jetzt das eigentliche Loeschen
    // die Controls wech
    pFormShell->GetFormView()->DeleteMarked();

    // das UNDO beginne ich erst jetzt : Das DeleteMarked erzeugt eine eigene Undo-::com::sun::star::chaos::Action, in die ich mich eigentlich einklinken
    // muesste, was leider nicht geht (das laeuft irgendwo im SwDoc), also erzeuge ich eine zweite, mit der man das Loeschen der
    // logischen Struktur zuruecknehmen kann (das ist nicht schoen, dass eine eigentlich atomare Aktion zwei UndoActions erzeugt,
    // aber das einzige, was wir gefunden haben)
    pFormModel->BegUndo(aUndoStr);

    // die Struktur wech
    for (i=0; i<m_arrCurrentSelection.Count(); ++i)
    {
        FmEntryData* pCurrent = (FmEntryData*)(m_arrCurrentSelection.GetObject(i)->GetUserData());
        // noch ein kleines Problem, bevor ich das ganz loesche : wenn es eine ::com::sun::star::form::Form ist und die Shell diese als CurrentObject
        // kennt, dann muss ich ihr das natuerlich ausreden
        if (pCurrent->ISA(FmFormData))
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xCurrentForm(pCurrent->GetElement(), ::com::sun::star::uno::UNO_QUERY);
            if (pFormShell->GetImpl()->getCurForm() == xCurrentForm)    // die Shell kennt die zu loeschende ::com::sun::star::form::Form ?
                pFormShell->GetImpl()->setCurForm(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > ());          // -> wegnehmen ...
        }
        GetExplModel()->Remove(pCurrent, sal_True);
    }
    pFormModel->EndUndo();
}

//------------------------------------------------------------------------
void FmExplorer::CollectSelectionData(SELDATA_ITEMS sdiHow)
{
    DBG_ASSERT(sdiHow != SDI_DIRTY, "FmExplorer::CollectSelectionData : ever thought about your parameter ? DIRTY ?");
    if (sdiHow == m_sdiState)
        return;

    m_arrCurrentSelection.Remove((sal_uInt16)0, m_arrCurrentSelection.Count());
    m_nFormsSelected = m_nControlsSelected = m_nHiddenControls = 0;
    m_bRootSelected = sal_False;

    SvLBoxEntry* pSelectionLoop = FirstSelected();
    while (pSelectionLoop)
    {
        // erst mal die Zaehlung der verschiedenen Elemente
        if (pSelectionLoop == m_pRootEntry)
            m_bRootSelected = sal_True;
        else
        {
            if (IsFormEntry(pSelectionLoop))
                ++m_nFormsSelected;
            else
            {
                ++m_nControlsSelected;
                if (IsHiddenControl((FmEntryData*)(pSelectionLoop->GetUserData())))
                    ++m_nHiddenControls;
            }
        }

        if (sdiHow == SDI_NORMALIZED)
        {
            // alles, was schon einen selektierten Vorfahr hat, nicht mitnehmen
            if (pSelectionLoop == m_pRootEntry)
                m_arrCurrentSelection.Insert(pSelectionLoop);
            else
            {
                SvLBoxEntry* pParentLoop = GetParent(pSelectionLoop);
                while (pParentLoop)
                {
                    // eigentlich muesste ich testen, ob das Parent in der m_arrCurrentSelection steht ...
                    // Aber wenn es selektiert ist, dann steht es in m_arrCurrentSelection, oder wenigstens einer seiner Vorfahren,
                    // wenn der auch schon selektiert war. In beiden Faellen reicht also die Abfrage IsSelected
                    if (IsSelected(pParentLoop))
                        break;
                    else
                    {
                        if (m_pRootEntry == pParentLoop)
                        {
                            // bis (exclusive) zur Root gab es kein selektiertes Parent -> der Eintrag gehoert in die normalisierte Liste
                            m_arrCurrentSelection.Insert(pSelectionLoop);
                            break;
                        }
                        else
                            pParentLoop = GetParent(pParentLoop);
                    }
                }
            }
        }
        else if (sdiHow == SDI_NORMALIZED_FORMARK)
        {
            SvLBoxEntry* pParent = GetParent(pSelectionLoop);
            if (!pParent || !IsSelected(pParent) || IsFormEntry(pSelectionLoop))
                m_arrCurrentSelection.Insert(pSelectionLoop);
        }
        else
            m_arrCurrentSelection.Insert(pSelectionLoop);


        pSelectionLoop = NextSelected(pSelectionLoop);
    }

    m_sdiState = sdiHow;
}

//------------------------------------------------------------------------
void FmExplorer::SynchronizeSelection(FmEntryDataArray& arredToSelect)
{
    LockSelectionHandling();
    if (arredToSelect.Count() == 0)
    {
        SelectAll(sal_False);
    }
    else
    {
        // erst mal gleiche ich meine aktuelle Selektion mit der geforderten SelectList ab
        SvLBoxEntry* pSelection = FirstSelected();
        while (pSelection)
        {
            FmEntryData* pCurrent = (FmEntryData*)pSelection->GetUserData();
            if (pCurrent != NULL)
            {
                sal_uInt16 nPosition;
                if ( arredToSelect.Seek_Entry(pCurrent, &nPosition) )
                {   // der Entry ist schon selektiert, steht aber auch in der SelectList -> er kann aus letzterer
                    // raus
                    arredToSelect.Remove(nPosition, 1);
                } else
                {   // der Entry ist selektiert, aber steht nicht in der SelectList -> Selektion rausnehmen
                    Select(pSelection, sal_False);
                    // und sichtbar machen (kann ja sein, dass das die einzige Modifikation ist, die ich hier in dem
                    // ganzen Handler mache, dann sollte das zu sehen sein)
                    MakeVisible(pSelection);
                }
            }
            else
                Select(pSelection, sal_False);

            pSelection = NextSelected(pSelection);
        }

        // jetzt habe ich in der SelectList genau die Eintraege, die noch selektiert werden muessen
        // zwei Moeglichkeiten : 1) ich gehe durch die SelectList, besorge mir zu jedem Eintrag meinen SvLBoxEntry
        // und selektiere diesen (waere irgendwie intuitiver ;)) 2) ich gehe durch alle meine SvLBoxEntries und selektiere
        // genau die, die ich in der SelectList finde
        // 1) braucht O(k*n) (k=Laenge der SelectList, n=Anzahl meiner Entries), plus den Fakt, dass FindEntry nicht den
        // Pointer auf die UserDaten vergleicht, sondern ein aufwendigeres IsEqualWithoutChilds durchfuehrt
        // 2) braucht O(n*log k), dupliziert aber etwas Code (naemlich den aus FindEntry)
        // da das hier eine relativ oft aufgerufenen Stelle sein koennte (bei jeder Aenderung in der Markierung in der ::com::sun::star::sdbcx::View !),
        // nehme ich doch lieber letzteres
        SvLBoxEntry* pLoop = First();
        while( pLoop )
        {
            FmEntryData* pCurEntryData = (FmEntryData*)pLoop->GetUserData();
            sal_uInt16 nPosition;
            if ( arredToSelect.Seek_Entry(pCurEntryData, &nPosition) )
            {
                Select(pLoop, sal_True);
                MakeVisible(pLoop);
                SetCursor(pLoop, sal_True);
            }

            pLoop = Next( pLoop );
        }
    }
    UnlockSelectionHandling();
}

//------------------------------------------------------------------------
void FmExplorer::SynchronizeSelection()
{
    // Shell und ::com::sun::star::sdbcx::View
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if(!pFormShell) return;

    FmFormView* pFormView = pFormShell->GetFormView();
    if (!pFormView) return;

    GetExplModel()->BroadcastMarkedObjects(pFormView->GetMarkList());
}

//------------------------------------------------------------------------
void FmExplorer::SynchronizeMarkList()
{
    // die Shell werde ich brauchen ...
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if (!pFormShell) return;

    CollectSelectionData(SDI_NORMALIZED_FORMARK);

    // Die ::com::sun::star::sdbcx::View soll jetzt kein Notify bei einer Aenderung der MarkList rauslassen
    pFormShell->GetImpl()->EnableTrackProperties(sal_False);

    UnmarkAllViewObj();

    for (sal_uInt32 i=0; i<m_arrCurrentSelection.Count(); ++i)
    {
        SvLBoxEntry* pSelectionLoop = m_arrCurrentSelection.GetObject(i);
        // Bei Formselektion alle Controls dieser ::com::sun::star::form::Form markieren
        if (IsFormEntry(pSelectionLoop) && (pSelectionLoop != m_pRootEntry))
            MarkViewObj((FmFormData*)pSelectionLoop->GetUserData(), sal_True, sal_False);

        // Bei Controlselektion Control-SdrObjects markieren
        else if (IsFormComponentEntry(pSelectionLoop))
        {
            FmControlData* pControlData = (FmControlData*)pSelectionLoop->GetUserData();
            if (pControlData)
            {
                /////////////////////////////////////////////////////////////////
                // Beim ::com::sun::star::form::HiddenControl kann kein Object selektiert werden
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent( pControlData->GetFormComponent());
                if (!xFormComponent.is())
                    continue;
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xFormComponent, ::com::sun::star::uno::UNO_QUERY);
                if (!xSet.is())
                    continue;

                sal_uInt16 nClassId = ::utl::getINT16(xSet->getPropertyValue(FM_PROP_CLASSID));
                if (nClassId != ::com::sun::star::form::FormComponentType::HIDDENCONTROL)
                    MarkViewObj(pControlData, sal_True, sal_True);
            }
        }
    }

    // wenn der PropertyBrowser offen ist, muss ich den entsprechend meiner Selektion anpassen
    // (NICHT entsprechend der MarkList der ::com::sun::star::sdbcx::View : wenn ich ein Formular selektiert habe, sind in der
    // ::com::sun::star::sdbcx::View alle zugehoerigen Controls markiert, trotzdem moechte ich natuerlich die Formular-Eigenschaften
    // sehen)
    ShowSelectionProperties(sal_False);

    // Flag an ::com::sun::star::sdbcx::View wieder zuruecksetzen
    pFormShell->GetImpl()->EnableTrackProperties(sal_True);

    // wenn jetzt genau eine ::com::sun::star::form::Form selektiert ist, sollte die Shell das als CurrentForm mitbekommen
    // (wenn SelectionHandling nicht locked ist, kuemmert sich die ::com::sun::star::sdbcx::View eigentlich in MarkListHasChanged drum,
    // aber der Mechanismus greift zum Beispiel nicht, wenn die ::com::sun::star::form::Form leer ist)
    if ((m_arrCurrentSelection.Count() == 1) && (m_nFormsSelected == 1))
    {
        FmEntryData* pSingleSelectionData = (FmEntryData*)(FirstSelected()->GetUserData());
        pFormShell->GetImpl()->setCurForm(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > (pSingleSelectionData->GetElement(), ::com::sun::star::uno::UNO_QUERY));
    }
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::IsHiddenControl(FmEntryData* pEntryData)
{
    if (pEntryData == NULL) return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xProperties(pEntryData->GetElement(), ::com::sun::star::uno::UNO_QUERY);
    if (::utl::hasProperty(FM_PROP_CLASSID, xProperties))
    {
        ::com::sun::star::uno::Any aClassID = xProperties->getPropertyValue( FM_PROP_CLASSID );
        return (::utl::getINT16(aClassID) == ::com::sun::star::form::FormComponentType::HIDDENCONTROL);
    }
    return sal_False;
}

//------------------------------------------------------------------------
sal_Bool FmExplorer::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
        return sal_True;

    return SvTreeListBox::Select(pEntry, bSelect );
}

//------------------------------------------------------------------------
void FmExplorer::UnmarkAllViewObj()
{
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if( !pFormShell )
        return;
    FmFormView* pFormView = pFormShell->GetFormView();
    pFormView->UnMarkAll();
}

//------------------------------------------------------------------------
void FmExplorer::MarkViewObj( FmFormData* pFormData, sal_Bool bMark, sal_Bool bDeep )
{
    //////////////////////////////////////////////////////////////////////
    // Alle Controls der ::com::sun::star::form::Form markieren
    FmEntryDataList* pChildList = pFormData->GetChildList();
    FmEntryData* pEntryData;
    FmControlData* pControlData;

    for( sal_uInt32 i=0; i<pChildList->Count(); i++ )
    {
        pEntryData = pChildList->GetObject(i);
        if( pEntryData->ISA(FmControlData) )
        {
            pControlData = (FmControlData*)pEntryData;
            MarkViewObj( pControlData, (i==pChildList->Count()-1), bMark );
        } else if (bDeep && (pEntryData->ISA(FmFormData)))
            MarkViewObj((FmFormData*)pEntryData, bMark, bDeep);
    }
}

//------------------------------------------------------------------------
void FmExplorer::MarkViewObj( FmControlData* pControlData, sal_Bool bMarkHandles, sal_Bool bMark)
{
    if( !pControlData ) return;
    FmFormShell* pFormShell = GetExplModel()->GetFormShell();
    if( !pFormShell ) return;

    //////////////////////////////////////////////////////////////////////
    // In der Page das entsprechende SdrObj finden und selektieren
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent( pControlData->GetFormComponent());
    FmFormView*     pFormView       = pFormShell->GetFormView();
    SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
    SdrPage*        pPage           = pPageView->GetPage();

    SdrObjListIter  aIter( *pPage );
    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();

        //////////////////////////////////////////////////////////////////////
        // Es interessieren nur Uno-Objekte
        if( pObj->IsUnoObj() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xControlModel(((SdrUnoObj*)pObj)->GetUnoControlModel());

            //////////////////////////////////////////////////////////////////////
            // Ist dieses Objekt ein ::com::sun::star::form::XFormComponent?
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormViewControl(xControlModel, ::com::sun::star::uno::UNO_QUERY);
            if( !xFormViewControl.is() )
                return;

            if (xFormViewControl == xFormComponent )
            {
                // Objekt markieren
                if (bMark != pFormView->IsObjMarked(pObj))
                    // der Writer mag das leider nicht, wenn schon markierte Objekte noch mal markiert werden ...
                    pFormView->MarkObj( pObj, pPageView, !bMark, sal_False );

                // Markierung in allen Fenstern in den sichtbaren Bereich verschieben
                if( bMarkHandles && bMark)
                {
                    Rectangle aMarkRect( pFormView->GetAllMarkedRect());

                    for( sal_uInt16 i=0; i<pFormView->GetWinCount(); i++ )
                        pFormView->MakeVisible( aMarkRect, *(Window*)pFormView->GetWin(i) );
                }
            }
        }
    }
}


//========================================================================
// class FmExplorerWin
//========================================================================
DBG_NAME(FmExplorerWin);
//------------------------------------------------------------------------
FmExplorerWin::FmExplorerWin( SfxBindings *pBindings, SfxChildWindow *pMgr,
                              Window* pParent )
  :SfxDockingWindow( pBindings, pMgr, pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
  ,SfxControllerItem( SID_FM_FMEXPLORER_CONTROL, *pBindings )
{
    DBG_CTOR(FmExplorerWin,NULL);
    SetHelpId( HID_FORM_NAVIGATOR_WIN );

    m_pFmExplorer = new FmExplorer( this );
    m_pFmExplorer->Show();
    SetText( SVX_RES(RID_STR_FMEXPLORER) );
    SetSizePixel( Size(200,200) );
}

//------------------------------------------------------------------------
FmExplorerWin::~FmExplorerWin()
{
    delete m_pFmExplorer;
    DBG_DTOR(FmExplorerWin,NULL);
}

//-----------------------------------------------------------------------
void FmExplorerWin::Update( FmFormShell* pFormShell )
{
    m_pFmExplorer->Update( pFormShell );
}

//-----------------------------------------------------------------------
void FmExplorerWin::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( !pState  || SID_FM_FMEXPLORER_CONTROL != nSID )
        return;

    if( eState >= SFX_ITEM_AVAILABLE )
    {
        FmFormShell* pShell = PTR_CAST( FmFormShell,((SfxObjectItem*)pState)->GetShell() );
        Update( pShell );
    }
    else
        Update( NULL );
}

//-----------------------------------------------------------------------
sal_Bool FmExplorerWin::Close()
{
    Update( NULL );
    return SfxDockingWindow::Close();
}

//-----------------------------------------------------------------------
void FmExplorerWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}

//-----------------------------------------------------------------------
Size FmExplorerWin::CalcDockingSize( SfxChildAlignment eAlign )
{
    Size aSize = SfxDockingWindow::CalcDockingSize( eAlign );

    switch( eAlign )
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
            return Size();
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
            break;
    }

    return aSize;
}

//-----------------------------------------------------------------------
SfxChildAlignment FmExplorerWin::CheckAlignment( SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
{
    switch (eAlign)
    {
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_NOALIGNMENT:
            return (eAlign);
    }

    return (eActAlign);
}

//------------------------------------------------------------------------
void FmExplorerWin::Resize()
{
    SfxDockingWindow::Resize();

    Size aLogOutputSize = PixelToLogic( GetOutputSizePixel(), MAP_APPFONT );
    Size aLogExplSize = aLogOutputSize;
    aLogExplSize.Width() -= 6;
    aLogExplSize.Height() -= 6;

    Point aExplPos = LogicToPixel( Point(3,3), MAP_APPFONT );
    Size aExplSize = LogicToPixel( aLogExplSize, MAP_APPFONT );

    m_pFmExplorer->SetPosSizePixel( aExplPos, aExplSize );
}


//========================================================================
// class FmExplorerWinMgr
//========================================================================

//-----------------------------------------------------------------------
SFX_IMPL_DOCKINGWINDOW( FmExplorerWinMgr, SID_FM_SHOW_FMEXPLORER )

//-----------------------------------------------------------------------
FmExplorerWinMgr::FmExplorerWinMgr( Window *pParent, sal_uInt16 nId,
                                    SfxBindings *pBindings, SfxChildWinInfo* pInfo )
                 :SfxChildWindow( pParent, nId )
{
    pWindow = new FmExplorerWin( pBindings, this, pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxDockingWindow*)pWindow)->Initialize( pInfo );
}

