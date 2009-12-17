/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frame.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXFRAME_HXX
#define _SFXFRAME_HXX

#ifndef TF_NEWDESKTOP
#define TF_NEWDESKTOP 1
#endif

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace awt
            {
                class XWindow;
            }
            namespace frame
            {
                class XFrame;
                class XFrameLoader;
                class XController;
                class XDispatchProviderInterceptor;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}
#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <tools/list.hxx>
#include <svl/poolitem.hxx>

class SvBorder;
class SfxWorkWindow;
class Window;
class SfxFrame;
class SfxFrame_Impl;
class SfxObjectShell;
class SfxObjectFactory;
class SfxViewFrame;
class SfxFrameDescriptor;
class SfxFrameSetDescriptor;
class SfxMedium;
class SfxFrameHistory_Impl;
class SfxDispatcher;
class SfxMedium;
class SfxCancellable;
class Rectangle;
class SfxRequest;
class SfxUnoControllerItem;
class SvCompatWeakHdl;

typedef SfxFrame* SfxFramePtr;
class SfxFrameArr_Impl;

DECLARE_LIST( TargetList, String* )

#define SFXFRAME_INTERNAL     0x0001
#define SFXFRAME_EXTERNAL     0x0002
#define SFXFRAME_OWNSDOCUMENT 0x0004
#define SFXFRAME_PLUGIN       0x0008
#define SFXFRAME_HASTITLE     0x0010
#define SFXFRAME_SERVER       0x0020 // Is es ein Frame, der in einem Container steckt ?
#define SFXFRAME_FRAMESET     0x0040

#define BROWSE_NORMAL         0
#define BROWSE_FORWARD        1
#define BROWSE_BACKWARD       2
#define NO_BROWSE             3
#define BROWSE_FRAME          4
#define BROWSE_MAX_MODE       10

//==========================================================================
// Ein SfxFrame ist eine Verwaltungsklasse f"ur Fenster und deren Inhalte.
// Eine SfxApplication pr"asentiert sich als Hierarchie von SfxFrames, wobei
// die konkreten Inhalte in den abgeleiteten Klassen festgelegt werden.
// Die Basisklasse SfxFrame implementiert 2 Aspekte der Frames: Benennung und
// Kontrolle der Lebensdauer.
// Innerhalb einer Frames-Hierarchie kontrolliert immer der ParentFrame die
// Lebensdauer seiner ChildFrames, auch wenn sie in der Regel gar nicht von
// ihm selbst erzeugt wurden. Durch Aufruf vonn DoClose() an irgendeinem
// Frame in der Hierarchie kann ein Teil des "Frameworks" entfernt werden,
// wobei sich Frames an ihren ParentFrames selbst abmelden.
//==========================================================================

class SfxFrameArr_Impl;
class SfxCancelManager;
struct SfxFramePickEntry_Impl;
class SfxUnoFrame;
class SFX2_DLLPUBLIC SfxFrame
{
    friend class SfxFrameIterator;
    friend struct SfxFramePickEntry_Impl;
    friend class SfxUnoFrame;
    friend class SfxViewFrame; /*HACK!*/

private:
    String              aName;
    SfxFrame*           pParentFrame;
    SfxFrameArr_Impl*   pChildArr;
    SfxFrame_Impl*      pImp;
    SfxUnoFrame*        pUnoImp;

    SAL_DLLPRIVATE void UpdateUndoHistory_Impl(
                            SfxObjectShell *pDocSh, const String* pNew, const String *pTitle = 0  );
    SAL_DLLPRIVATE void UpdateCurrentHistory_Impl(
                            SfxObjectShell *pDocSh, const String* pNew  );

protected:
    virtual sal_Bool    Close() = 0;
    virtual             ~SfxFrame();

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void SetCurrentDocument_Impl( SfxObjectShell* );
    SAL_DLLPRIVATE void InsertChildFrame_Impl( SfxFrame*, sal_uInt16 nPos = 0 );
    SAL_DLLPRIVATE void RemoveChildFrame_Impl( sal_uInt16 nPos );
    SAL_DLLPRIVATE void RemoveChildFrame_Impl( SfxFrame* );
    SAL_DLLPRIVATE void SetIsTop_Impl( sal_Bool bIsTop = sal_True );
//#endif

public:
                        TYPEINFO();
                        SfxFrame(SfxFrame* pParent = 0);

    SvCompatWeakHdl*    GetHdl();
    virtual Window&     GetWindow() const = 0;
    void                CancelTransfers( sal_Bool bCancelLoadEnv = sal_True );
    sal_Bool            DoClose();
    sal_uInt16          GetChildFrameCount() const;
    SfxFrame*           GetChildFrame( sal_uInt16 nPos ) const;
    void                SetFrameName( const String& rName );
    const String&       GetFrameName() const
                        { return aName; }
    SfxFrame*           GetParentFrame() const
                        { return pParentFrame; }
    String              GetContent() const;

    static SfxFrame*    GetFirst();
    static SfxFrame*    GetNext( SfxFrame& );

    virtual sal_Bool    InsertDocument( SfxObjectShell *pDoc );
    const SfxPoolItem*  LoadDocumentSynchron( SfxItemSet& aSet );
    void                DocumentInserted( SfxObjectShell* pDoc );

    SfxBroadcaster&     GetBroadcaster() const;
    SfxObjectShell*     GetCurrentDocument() const;
    SfxViewFrame*       GetCurrentViewFrame() const;
    SfxFrame*           GetTopFrame() const;
    sal_Bool            IsParent( SfxFrame* ) const;

    SfxFrame*           findFrame( const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags);
    SfxFrame*           SearchFrame( const String&, SfxMedium* pMedium = 0 );
    sal_uInt32          GetFrameType() const;
    void                GetTargetList( TargetList& ) const;
    void                ClearHistory();
    SAL_DLLPRIVATE SfxFrame* GetContainingDocFrame_Impl( SfxFrame* pSelf );
    sal_Bool            IsTop() const;
    sal_Bool            CloseChildFrames();
    void                UpdatePickEntries();
    void                UpdatePickEntries( const ::com::sun::star::uno::Any& rValue );
    void                UpdateHistory( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const ::rtl::OUString& rTitle );
    void                UpdateHistory(SfxObjectShell*, const String*pNew = 0 );
    void                UpdateDescriptor( SfxObjectShell *pDoc );
    void                Resize();
    sal_Bool            HasComponent() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                            GetComponent() const;
    void                ReleaseComponent();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                            GetFrameInterface() const;
    void                Appear();
    void                AppearWithUpdate();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                            GetController() const;

    static sal_Bool     LoadSfxComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > &,
                            const ::rtl::OUString&,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&,
                            const SfxObjectFactory* );

    sal_Bool            IsInPlace() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE sal_Bool DoClose_Impl();
    SAL_DLLPRIVATE void SetFrameInterface_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
    SAL_DLLPRIVATE void ReleasingComponent_Impl( sal_Bool bSet );
    SAL_DLLPRIVATE const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >
                            GetInterceptor_Impl();
    SAL_DLLPRIVATE void Clear_Impl();
    SAL_DLLPRIVATE sal_uInt16 LoadComponent_Impl( const ::rtl::OUString& rURL,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameLoader > & rLoader,
                            SfxItemSet* pSet=0, sal_Bool bDontClose=sal_False );
    SAL_DLLPRIVATE void LoadFinished_Impl();
    DECL_DLLPRIVATE_STATIC_LINK(   SfxFrame, BindingHasNewPart_Impl, void* );
    SAL_DLLPRIVATE SfxCancelManager* GetCancelManager() const;
    SAL_DLLPRIVATE void GetViewData_Impl();
    SAL_DLLPRIVATE sal_Bool CheckContentForLoad_Impl();
    SAL_DLLPRIVATE sal_Bool ExecuteHistoryMenu_Impl( sal_uInt16 nWnich, const Rectangle& rRect, sal_uInt16 nFlags=0 );
    SAL_DLLPRIVATE void ActivatePickEntry_Impl( SfxFramePickEntry_Impl*, sal_uInt16 nMode, SfxFrameDescriptor *pD = NULL );
    SAL_DLLPRIVATE void CopyHistory_Impl( SfxFrame *pFrame ) const;
    SAL_DLLPRIVATE SfxFrame* SearchChildrenForName_Impl( const String&, sal_Bool bDeep = sal_True ) const;
    SAL_DLLPRIVATE void SetFrameType_Impl( sal_uInt32 );
    SAL_DLLPRIVATE void Activate_Impl( sal_Bool bBeamerOn );
    SAL_DLLPRIVATE void Deactivate_Impl();
    SAL_DLLPRIVATE sal_uInt16 PrepareClose_Impl( sal_Bool bUI, sal_Bool bForBrowsing=sal_False );
    SAL_DLLPRIVATE sal_Bool DocIsModified_Impl();
    SAL_DLLPRIVATE void SetCurrentViewFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE SfxFrame* SearchFrame_Impl( sal_uInt16, sal_Bool bDeep=sal_False );
    SAL_DLLPRIVATE SfxFrame* SearchFrame_Impl( const String&, sal_Bool bDeep=sal_False );
    SAL_DLLPRIVATE void SetFrameId_Impl( sal_uInt16 );
    SAL_DLLPRIVATE sal_uInt16 GetFrameId_Impl() const;
    SAL_DLLPRIVATE void SetFrameIdName_Impl( const String& );
    SAL_DLLPRIVATE sal_Bool IsClosing_Impl() const;
    SAL_DLLPRIVATE void SetIsClosing_Impl();
    SAL_DLLPRIVATE sal_Bool BrowseInFrame( int nDelta );
    SAL_DLLPRIVATE sal_Bool Browse( sal_Bool bForward, sal_uInt16 nDelta = 1, sal_Bool bNewFrame=sal_False );
    SAL_DLLPRIVATE sal_Bool CanBrowseForward() const;
    SAL_DLLPRIVATE sal_Bool CanBrowseBackward() const;

                        // Methoden f"ur den Zugriff auf das aktuelle Set
    SAL_DLLPRIVATE void SetDescriptor( SfxFrameDescriptor* );
    SAL_DLLPRIVATE SfxFrameDescriptor* GetDescriptor() const;

    SAL_DLLPRIVATE void Lock_Impl( sal_Bool bLock );
    SAL_DLLPRIVATE sal_uInt16 GetLockCount_Impl() const;
    //sal_Bool            IsLocked_Impl() const
    //                  { return GetLockCount_Impl() > 0; }
    SAL_DLLPRIVATE void CloseOnUnlock_Impl();
    SAL_DLLPRIVATE SfxViewFrame* ActivateChildFrame_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsAutoLoadLocked_Impl() const;

    SAL_DLLPRIVATE static void InsertTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE static void RemoveTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE void SetItemSet_Impl( const SfxItemSet* pSet );
    SAL_DLLPRIVATE const SfxItemSet* GetItemSet_Impl();
    SAL_DLLPRIVATE void SetOwnsBindings_Impl( sal_Bool bSet );
    SAL_DLLPRIVATE sal_Bool OwnsBindings_Impl() const;
    SAL_DLLPRIVATE void InvalidateUnoControllers_Impl();
    SAL_DLLPRIVATE void RegisterUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE void ReleaseUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
    SAL_DLLPRIVATE void SetToolSpaceBorderPixel_Impl( const SvBorder& );
    SAL_DLLPRIVATE Rectangle GetTopOuterRectPixel_Impl() const;
    SAL_DLLPRIVATE void CreateWorkWindow_Impl();
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* pWorkwin );
    SAL_DLLPRIVATE const SvBorder& GetBorder_Impl() const;
    SAL_DLLPRIVATE void GrabFocusOnComponent_Impl();
    SAL_DLLPRIVATE void ReFill_Impl( const SfxFrameSetDescriptor* pSet );
    SAL_DLLPRIVATE void LockFocus_Impl( sal_Bool bLock );
    SAL_DLLPRIVATE sal_Bool IsFocusLocked_Impl() const;
    SAL_DLLPRIVATE void CloseDocument_Impl();
    SAL_DLLPRIVATE void SetInPlace_Impl( sal_Bool );
//  sal_Bool            IsPlugin_Impl() const;
//#endif
};

SV_DECL_COMPAT_WEAK( SfxFrame )

class SfxFrameIterator
{
    const SfxFrame*         pFrame;
    sal_Bool                bRecursive;

//#if 0 // _SOLAR__PRIVATE
    SfxFrame*               NextSibling_Impl( SfxFrame& rPrev );
//#endif

public:
                            SfxFrameIterator( const SfxFrame& rFrame, sal_Bool bRecursive=sal_True );
    SfxFrame*               FirstFrame();
    SfxFrame*               NextFrame( SfxFrame& rPrev );
};

//--------------------------------------------------------------------

class SfxFrameItem_Impl;
class SFX2_DLLPUBLIC SfxFrameItem: public SfxPoolItem
{
    SfxFrame*               pFrame;
    SfxFrameWeak            wFrame;
    SAL_DLLPRIVATE void SetFramePtr_Impl( SfxFrame* /*pFrameP*/ ) { pFrame = wFrame; }

public:
                            TYPEINFO();

                            SfxFrameItem( sal_uInt16 nWhich, SfxViewFrame *p );
                            SfxFrameItem( SfxFrame *p=0 );
                            SfxFrameItem( sal_uInt16 nWhich, SfxFrame *p );

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual String          GetValueText() const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    sal_Bool                FrameKilled() const { return &wFrame != pFrame; }

    SfxFrame*               GetFrame() const
                            { return wFrame; }
};

class SFX2_DLLPUBLIC SfxUsrAnyItem : public SfxPoolItem
{
    ::com::sun::star::uno::Any  aValue;
public:
                                TYPEINFO();
                                SfxUsrAnyItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Any& rAny );
    ::com::sun::star::uno::Any  GetValue() const
                                { return aValue; }
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual String              GetValueText() const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

typedef SfxUsrAnyItem SfxUnoAnyItem;

#endif
