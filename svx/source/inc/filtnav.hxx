/*************************************************************************
 *
 *  $RCSfile: filtnav.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:54:19 $
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
#ifndef _SVX_FILTNAV_HXX
#define _SVX_FILTNAV_HXX

#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif

#ifndef _DTRANS_HXX
#include <so3/dtrans.hxx>
#endif

#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_DOCKWIN_HXX //autogen
#include <vcl/dockwin.hxx>
#endif

#ifndef _SVX_FMTOOLS_HXX
#include <fmtools.hxx>
#endif // _SVX_FMTOOLS_HXX
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

class FmFormShell;
class FmFilterItem;
class FmFilterItems;
class FmParentData;
class FmFilterAdapter;

//========================================================================
// data structure for the filter model
//========================================================================
class FmFilterData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;
    FmParentData*           m_pParent;
    ::rtl::OUString         m_aText;

public:
    TYPEINFO();
    FmFilterData(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmParentData* pParent = NULL, const ::rtl::OUString& rText = ::rtl::OUString())
        :m_pParent(pParent)
        ,m_aText(rText)
        ,m_xORB()
    {}
    virtual ~FmFilterData(){}

    void    SetText( const ::rtl::OUString& rText ){ m_aText = rText; }
    ::rtl::OUString  GetText() const { return m_aText; }
    FmParentData* GetParent() const {return m_pParent;}
    virtual Image GetImage() const;
};

//========================================================================
class FmParentData : public FmFilterData
{
protected:
    vector<FmFilterData*> m_aChilds;


public:
    TYPEINFO();
    FmParentData(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmParentData* pParent, const ::rtl::OUString& rText)
        : FmFilterData(_rxFactory,pParent, rText)
    {}
    virtual ~FmParentData();

    vector<FmFilterData*>& GetChilds() {return m_aChilds;}
};

//========================================================================
// Item representing the forms and subforms
class FmFormItem : public FmParentData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >     m_xController;
    sal_Int32           m_nCurrent;

public:
    TYPEINFO();
    FmFormItem(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory):FmParentData(_rxFactory,NULL, ::rtl::OUString()){}
    FmFormItem(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmParentData* _pParent,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & _xController,
                 const ::rtl::OUString& _rText):FmParentData(_rxFactory,_pParent, _rText)
                                     ,m_xController(_xController)
                                     ,m_nCurrent(0){}

    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & GetController(){return m_xController;}
    void SetCurrentPosition(sal_Int32 nCurrent){m_nCurrent = nCurrent;}
    sal_Int32 GetCurrentPosition() const {return m_nCurrent;}
    virtual Image GetImage() const;
};

//========================================================================
class FmFilterItems : public FmParentData
{
public:
    TYPEINFO();
    FmFilterItems(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory):FmParentData(_rxFactory,NULL, ::rtl::OUString()){}
    FmFilterItems(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmFormItem* pParent, const ::rtl::OUString& rText ):FmParentData(_rxFactory,pParent, rText){}

    FmFilterItem* Find(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > & xText) const;
    virtual Image GetImage() const;
};

//========================================================================
class FmFilterItem : public FmFilterData
{
    ::rtl::OUString m_aFieldName;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > m_xText;

public:
    TYPEINFO();
    FmFilterItem(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,
                FmFilterItems* pParent,
              const ::rtl::OUString& aFieldName,
              const ::rtl::OUString& aCondition,
              const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > & xText);

    const ::rtl::OUString& GetFieldName() const {return m_aFieldName;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > & GetTextComponent() const {return m_xText;}

    virtual Image GetImage() const;
};

//========================================================================
class FmFilterModel : public FmParentData
                     ,public SfxBroadcaster
{
    friend class FmFilterAdapter;

    connectivity::OSQLParser    m_aParser;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >       m_xControllers;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >         m_xController;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
    FmFilterAdapter*        m_pAdapter;
    FmFilterItems*          m_pCurrentItems;

public:
    TYPEINFO();
    FmFilterModel(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);
    virtual ~FmFilterModel();

    void Update(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xControllers, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xCurrent);
    void Clear();
    sal_Bool ValidateText(FmFilterItem* pItem, UniString& rText, UniString& rErrorMsg) const;
    void Append(FmFilterItems* pItems, FmFilterItem* pFilterItem);
    void SetText(FmFilterItem* pItem, const ::rtl::OUString& rText);

    FmFormItem* GetCurrentForm() const {return m_pCurrentItems ? (FmFormItem*)m_pCurrentItems->GetParent() : NULL;}
    FmFilterItems* GetCurrentItems() const {return m_pCurrentItems;}
    void SetCurrentItems(FmFilterItems* pCurrent);
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const { return m_xORB; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & GetControllers() const {return m_xControllers;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & GetCurrentController() const {return m_xController;}
    void SetCurrentController(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xController);

    void Remove(FmFilterData* pFilterItem);
    void AppendFilterItems(FmFormItem* pItem);
    void CheckIntegrity(FmParentData* pItem);

protected:
    void Insert(const vector<FmFilterData*>::iterator& rPos, FmFilterData* pFilterItem);
    void Remove(const vector<FmFilterData*>::iterator& rPos, FmFilterData* pFilterItem);
    FmFormItem* Find(const ::std::vector<FmFilterData*>& rItems, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xController) const;
    FmFormItem* Find(const ::std::vector<FmFilterData*>& rItems, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm) const;
    void Update(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xControllers, FmParentData* pParent);
};

//========================================================================
class FmFilterNavigator : public SvTreeListBox, public SfxListener
{
    enum DROP_ACTION{ DA_SCROLLUP, DA_SCROLLDOWN, DA_EXPANDNODE };

    FmFilterModel*      m_pModel;
    Timer               m_aSelectTimer;

    AutoTimer           m_aDropActionTimer;
    unsigned short      m_aTimerCounter;
    Point               m_aTimerTriggered;      // die Position, an der der DropTimer angeschaltet wurde
    DROP_ACTION         m_aDropActionType;

public:
    FmFilterNavigator( Window* pParent );
    virtual ~FmFilterNavigator();

    void Update(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xControllers, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xCurrent);
    FmFilterModel* const GetFilterModel() const {return m_pModel;}

protected:
    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void Command( const CommandEvent& rEvt );
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void InitEntry(SvLBoxEntry* pEntry, const XubString& rStr, const Image& rImg1, const Image& rImg2);
    virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    virtual sal_Bool EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection );
    virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );
    virtual sal_Bool QueryDrop( DropEvent& rDEvt );
    virtual sal_Bool Drop( const DropEvent& rDEvt );

    void DeleteSelection();
    void Clear();
    SvLBoxEntry* FindEntry(FmFilterData* pItem);
    void Insert(FmFilterData* pItem, sal_Int32 nPos);
    void Remove(FmFilterData* pItem);

    DECL_LINK(OnRemove, SvLBoxEntry*);
    DECL_LINK(OnEdited, SvLBoxEntry*);
    DECL_LINK(OnSelect, void*);
    DECL_LINK(OnDropActionTimer, void*);
};

//========================================================================
class FmFilterNavigatorWin : public SfxDockingWindow, public SfxControllerItem
{
private:
    FmFilterNavigator* m_pNavigator;

protected:
    virtual void Resize();
    virtual sal_Bool Close();
    virtual Size CalcDockingSize( SfxChildAlignment );
    virtual SfxChildAlignment CheckAlignment( SfxChildAlignment, SfxChildAlignment );

public:
    FmFilterNavigatorWin( SfxBindings *pBindings, SfxChildWindow *pMgr,
                   Window* pParent );
    virtual ~FmFilterNavigatorWin();

    void Update( FmFormShell* pFormShell );
    void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void FillInfo( SfxChildWinInfo& rInfo ) const;
};

//========================================================================
class FmFilterNavigatorWinMgr : public SfxChildWindow
{
public:
    FmFilterNavigatorWinMgr( Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                      SfxChildWinInfo *pInfo );
    SFX_DECL_CHILDWINDOW( FmFilterNavigatorWinMgr );
};

//========================================================================
class FmFormItem;
class FmFilterItem;
class SvxFmFilterExch : public SvDataObject
{
    SvDataTypeList          m_aDataTypeList;
    vector<FmFilterItem*>   m_aDraggedEntries;
    FmFormItem*             m_pFormItem;        // ensure that we drop on the same form

public:
    SvxFmFilterExch(FmFormItem*  pFormItem, const vector<FmFilterItem*>& lstWhich );

    virtual const SvDataTypeList& GetTypeList() const { return m_aDataTypeList; }
    virtual sal_Bool  GetData( SvData* );

    const vector<FmFilterItem*>& GetDraggedEntries() const { return m_aDraggedEntries; }
    FmFormItem* GetFormItem() const {return m_pFormItem;}
};

SV_DECL_IMPL_REF( SvxFmFilterExch );

#endif // _SVX_FILTNAV_HXX

