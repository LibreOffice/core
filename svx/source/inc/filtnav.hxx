/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filtnav.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 17:10:27 $
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
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif
#ifndef SVX_SQLPARSERCLIENT_HXX
#include "sqlparserclient.hxx"
#endif

class FmFormShell;

//........................................................................
namespace svxform
{
//........................................................................

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
        :m_xORB( _rxFactory )
        ,m_pParent( pParent )
        ,m_aText( rText )
    {}
    virtual ~FmFilterData(){}

    void    SetText( const ::rtl::OUString& rText ){ m_aText = rText; }
    ::rtl::OUString  GetText() const { return m_aText; }
    FmParentData* GetParent() const {return m_pParent;}

    virtual Image GetImage( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
};

//========================================================================
class FmParentData : public FmFilterData
{
protected:
    ::std::vector<FmFilterData*> m_aChilds;


public:
    TYPEINFO();
    FmParentData(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmParentData* pParent, const ::rtl::OUString& rText)
        : FmFilterData(_rxFactory,pParent, rText)
    {}
    virtual ~FmParentData();

    ::std::vector<FmFilterData*>& GetChilds() {return m_aChilds;}
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
    virtual Image GetImage( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
};

//========================================================================
class FmFilterItems : public FmParentData
{
public:
    TYPEINFO();
    FmFilterItems(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory):FmParentData(_rxFactory,NULL, ::rtl::OUString()){}
    FmFilterItems(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,FmFormItem* pParent, const ::rtl::OUString& rText ):FmParentData(_rxFactory,pParent, rText){}

    FmFilterItem* Find(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > & xText) const;
    virtual Image GetImage( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
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

    virtual Image GetImage( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
};

//========================================================================
class FmFilterModel : public FmParentData
                     ,public SfxBroadcaster
                     ,public ::svxform::OSQLParserClient
{
    friend class FmFilterAdapter;

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
    void SetTextForItem(FmFilterItem* pItem, const ::rtl::OUString& rText);

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
    void Insert(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pFilterItem);
    void Remove(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pFilterItem);
    FmFormItem* Find(const ::std::vector<FmFilterData*>& rItems, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xController) const;
    FmFormItem* Find(const ::std::vector<FmFilterData*>& rItems, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm) const;
    void Update(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xControllers, FmParentData* pParent);
};

//========================================================================
//========================================================================
class OFilterItemExchange : public OLocalExchange
{
    ::std::vector<FmFilterItem*>    m_aDraggedEntries;
    FmFormItem*             m_pFormItem;        // ensure that we drop on the same form

public:
    OFilterItemExchange();

    static sal_uInt32       getFormatId( );
    inline static sal_Bool  hasFormat( const DataFlavorExVector& _rFormats );

    const ::std::vector<FmFilterItem*>& getDraggedEntries() const { return m_aDraggedEntries; }
    void setDraggedEntries(const ::std::vector<FmFilterItem*>& _rList) { m_aDraggedEntries = _rList; }
    FmFormItem* getFormItem() const { return m_pFormItem; }

    void setFormItem( FmFormItem* _pItem ) { m_pFormItem = _pItem; }
    void addSelectedItem( FmFilterItem* _pItem) { m_aDraggedEntries.push_back(_pItem); }

protected:
    virtual void AddSupportedFormats();
};

inline sal_Bool OFilterItemExchange::hasFormat( const DataFlavorExVector& _rFormats )
{
    return OLocalExchange::hasFormat( _rFormats, getFormatId() );
}

//====================================================================
//= OFilterExchangeHelper
//====================================================================
class OFilterExchangeHelper : public OLocalExchangeHelper
{
public:
    OFilterExchangeHelper(Window* _pDragSource) : OLocalExchangeHelper(_pDragSource) { }

    OFilterItemExchange* operator->() const { return static_cast<OFilterItemExchange*>(m_pTransferable); }

protected:
    virtual OLocalExchange* createExchange() const;
};

//========================================================================
class FmFilterNavigator : public SvTreeListBox, public SfxListener
{
    enum DROP_ACTION{ DA_SCROLLUP, DA_SCROLLDOWN, DA_EXPANDNODE };

    FmFilterModel*          m_pModel;
    SvLBoxEntry*            m_pEditingCurrently;
    OFilterExchangeHelper   m_aControlExchange;


    AutoTimer           m_aDropActionTimer;
    unsigned short      m_aTimerCounter;
    Point               m_aTimerTriggered;      // die Position, an der der DropTimer angeschaltet wurde
    DROP_ACTION         m_aDropActionType;

public:
    FmFilterNavigator( Window* pParent );
    virtual ~FmFilterNavigator();

    void UpdateContent(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xControllers, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > & xCurrent);
    FmFilterModel* const GetFilterModel() const {return m_pModel;}

protected:
    using Control::Notify;

    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void Command( const CommandEvent& rEvt );
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void InitEntry(SvLBoxEntry* pEntry, const XubString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind);
    virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    virtual sal_Bool EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection );
    virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    void DeleteSelection();
    void Clear();
    SvLBoxEntry* FindEntry(FmFilterData* pItem);
    void Insert(FmFilterData* pItem, sal_Int32 nPos);
    void Remove(FmFilterData* pItem);

    DECL_LINK(OnRemove, SvLBoxEntry*);
    DECL_LINK(OnEdited, SvLBoxEntry*);
    DECL_LINK(OnDropActionTimer, void*);

private:
    /** returns the first form item and the selected FilterItems in the vector
        @param  _rItemList
            Is filled inside. <OUT/>
        @return
            The first form item.
    */
    FmFormItem* getSelectedFilterItems(::std::vector<FmFilterItem*>& _rItemList);
    /* inserts the filter items into the tree model and creates new FilterItems if needed.
        @param  _rFilterList
            The items which should be inserted.
        @param  _pTargetItems
            The target where to insert the items.
        @param  _bCopy
            If <TRUE/> the items will not be removed from the model, otherwise they will.
    */
    void insertFilterItem(const ::std::vector<FmFilterItem*>& _rFilterList,FmFilterItems* _pTargetItems,sal_Bool _bCopy = sal_False);
    SvLBoxEntry* getPrevEntry(SvLBoxEntry* _pStartWith = NULL);
    SvLBoxEntry* getNextEntry(SvLBoxEntry* _pStartWith = NULL);

    using SvTreeListBox::Select;
    using SvTreeListBox::ExecuteDrop;
    using SvTreeListBox::Insert;
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

    void UpdateContent( FmFormShell* pFormShell );
    void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void FillInfo( SfxChildWinInfo& rInfo ) const;

    using SfxDockingWindow::StateChanged;

    virtual void GetFocus();
};

//========================================================================
class FmFilterNavigatorWinMgr : public SfxChildWindow
{
public:
    FmFilterNavigatorWinMgr( Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                      SfxChildWinInfo *pInfo );
    SFX_DECL_CHILDWINDOW( FmFilterNavigatorWinMgr );
};

//........................................................................
}   // namespace svxform
//........................................................................

#endif // _SVX_FILTNAV_HXX

