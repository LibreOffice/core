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
#ifndef _SFXVIEWFRM_HXX
#define _SFXVIEWFRM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <svl/lstner.hxx>
#include <sfx2/module.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/poolitem.hxx>
#include <com/sun/star/frame/status/Verb.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController2.hpp>

#include <tools/svborder.hxx>

class SvBorder;
class SfxDispatcher;
class SfxObjectShell;
class SfxBindings;
class SfxProgress;
class SvData;
class SfxViewShell;
class SystemWindow;
class Fraction;
class Point;
class Size;
class SfxChildWindow;

namespace sfx2
{
class SvLinkSource;
}
namespace svtools
{
    class AsynchronLink;
}

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

//========================================================================
DBG_NAMEEX(SfxViewFrame)
class SFX2_DLLPUBLIC SfxViewFrame: public SfxShell, public SfxListener
{
    struct SfxViewFrame_Impl*   pImp;

    SfxObjectShellRef           xObjSh;
    SfxDispatcher*              pDispatcher;
    SfxBindings*                pBindings;
    sal_uInt16                      nAdjustPosPixelLock;

private:
#ifndef _SFX_HXX
    SAL_DLLPRIVATE void Construct_Impl( SfxObjectShell *pObjSh=NULL );
#endif

protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

#ifndef _SFX_HXX
    SAL_DLLPRIVATE void KillDispatcher_Impl();
#endif

    virtual                 ~SfxViewFrame();

public:
                            SfxViewFrame( SfxFrame& rFrame, SfxObjectShell *pDoc = NULL );

                            TYPEINFO();
                            SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWFRM)

    static void             SetViewFrame( SfxViewFrame* );

    static SfxViewFrame*    LoadHiddenDocument( SfxObjectShell& i_rDoc, const sal_uInt16 i_nViewId );
    static SfxViewFrame*    LoadDocument( SfxObjectShell& i_rDoc, const sal_uInt16 i_nViewId );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const SfxFrameItem* i_pFrameItem, const sal_uInt16 i_nViewId = 0 );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrameItem, const sal_uInt16 i_nViewId = 0 );
    static SfxViewFrame*    DisplayNewDocument( SfxObjectShell& i_rDoc, const SfxRequest& i_rCreateDocRequest, const sal_uInt16 i_nViewId = 0 );

    static SfxViewFrame*    Current();
    static SfxViewFrame*    GetFirst( const SfxObjectShell* pDoc = 0, sal_Bool bOnlyVisible = sal_True );
    static SfxViewFrame*    GetNext( const SfxViewFrame& rPrev, const SfxObjectShell* pDoc = 0, sal_Bool bOnlyVisible = sal_True );

    static SfxViewFrame*    Get( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>& i_rController, const SfxObjectShell* i_pDoc = NULL );

            void            DoActivate(sal_Bool bMDI, SfxViewFrame *pOld=NULL);
            void            DoDeactivate(sal_Bool bMDI, SfxViewFrame *pOld=NULL);

    SfxViewFrame*           GetParentViewFrame() const;

    using SfxShell::GetDispatcher;
        SfxDispatcher*          GetDispatcher() { return pDispatcher; }
    SfxBindings&            GetBindings() { return *pBindings; }
    const SfxBindings&      GetBindings() const  { return *pBindings; }
    Window&                 GetWindow() const;
    virtual void            SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    SfxProgress*            GetProgress() const;

    SfxObjectShell*         GetObjectShell() const
                            { return xObjSh; }

    void                    LockAdjustPosSizePixel()
                            { nAdjustPosPixelLock++; }
    void                    UnlockAdjustPosSizePixel()
                            { nAdjustPosPixelLock--; }
    void                    DoAdjustPosSizePixel( SfxViewShell * pSh,
                                        const Point &rPos, const Size &rSize );
    void                    Show();
    sal_Bool                    IsVisible() const;
    void                    ToTop();
    void                    Enable( sal_Bool bEnable );
    virtual sal_Bool            Close();
    virtual void            Activate( sal_Bool bUI );
    virtual void            Deactivate( sal_Bool bUI );

    // DDE-Interface
    virtual long            DdeExecute( const String& rCmd );
    virtual long            DdeGetData( const String& rItem,
                                        const String& rMimeType,
                                        ::com::sun::star::uno::Any & rValue );
    virtual long            DdeSetData( const String& rItem,
                                        const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource*   DdeCreateLinkSource( const String& rItem );

    void                    UpdateTitle();

    static void ActivateToolPanel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const ::rtl::OUString& i_rPanelURL );

    // interne Handler
    SAL_DLLPRIVATE virtual sal_Bool SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    SAL_DLLPRIVATE virtual const SvBorder& GetBorderPixelImpl( const SfxViewShell *pSh ) const;
    SAL_DLLPRIVATE virtual void InvalidateBorderImpl( const SfxViewShell *pSh );

    virtual SfxObjectShell* GetObjectShell();
    sal_uInt16                  GetCurViewId() const;
    SfxFrame&               GetFrame() const;
    SfxViewFrame*           GetTopViewFrame() const;

    sal_Bool                    DoClose();
    sal_uIntPtr                 GetFrameType() const
                            { return GetFrame().GetFrameType(); }
    SfxFrame&               GetTopFrame() const
                            { return GetFrame().GetTopFrame(); }
    void                    GetTargetList( TargetList& rList ) const
                            { GetFrame().GetTargetList( rList ); }
    void                    CancelTransfers()
                            { GetFrame().CancelTransfers(); }

    void                    SetModalMode( sal_Bool );
    sal_Bool                    IsInModalMode() const;
    void                    Resize(sal_Bool bForce=sal_False);

    void                        SetChildWindow(sal_uInt16 nId, sal_Bool bVisible, sal_Bool bSetFocus=sal_True);
    void                        ToggleChildWindow(sal_uInt16);
    sal_Bool                        HasChildWindow(sal_uInt16);
    sal_Bool                        KnowsChildWindow(sal_uInt16);
    void                        ShowChildWindow(sal_uInt16,sal_Bool bVisible=sal_True);
    SfxChildWindow*             GetChildWindow(sal_uInt16);
    void                        ChildWindowExecute(SfxRequest&);
    void                        ChildWindowState(SfxItemSet&);

    SAL_DLLPRIVATE void SetDowning_Impl();
    SAL_DLLPRIVATE void GetDocNumber_Impl();
    SAL_DLLPRIVATE sal_Bool IsDowning_Impl() const;
    SAL_DLLPRIVATE void SetViewShell_Impl( SfxViewShell *pVSh );
    SAL_DLLPRIVATE void ReleaseObjectShell_Impl();

    SAL_DLLPRIVATE void GetState_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateReload_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecView_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateView_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecHistory_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateHistory_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE SfxViewFrame* GetParentViewFrame_Impl() const;
    SAL_DLLPRIVATE void ForceOuterResize_Impl(sal_Bool bOn=sal_True);
    SAL_DLLPRIVATE sal_Bool IsResizeInToOut_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsAdjustPosSizePixelLocked_Impl() const
                            { return nAdjustPosPixelLock != 0; }
    SAL_DLLPRIVATE void UpdateDocument_Impl();

    SAL_DLLPRIVATE void LockObjectShell_Impl(sal_Bool bLock=sal_True);

    SAL_DLLPRIVATE void MakeActive_Impl( sal_Bool bActivate );
    SAL_DLLPRIVATE void SetQuietMode_Impl( sal_Bool );
    SAL_DLLPRIVATE const Size& GetMargin_Impl() const;
    SAL_DLLPRIVATE void SetActiveChildFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE SfxViewFrame* GetActiveChildFrame_Impl() const;
    SAL_DLLPRIVATE String GetActualPresentationURL_Impl() const;
    SAL_DLLPRIVATE void MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl( sal_uInt16 nId );
    SAL_DLLPRIVATE void AddDispatchMacroToBasic_Impl(const ::rtl::OUString& sMacro);

    SAL_DLLPRIVATE void Exec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void SetCurViewId_Impl( const sal_uInt16 i_nID );
    SAL_DLLPRIVATE void ActivateToolPanel_Impl( const ::rtl::OUString& i_rPanelURL );

private:
    SAL_DLLPRIVATE sal_Bool SwitchToViewShell_Impl( sal_uInt16 nNo, sal_Bool bIsIndex = sal_False );
    SAL_DLLPRIVATE void PopShellAndSubShells_Impl( SfxViewShell& i_rViewShell );
    SAL_DLLPRIVATE void SaveCurrentViewData_Impl( const sal_uInt16 i_nNewViewId );

    /** loads the given existing document into the given frame

        This is done using the XComponentLoader interface of the frame, so the SFX document loader is invoked.

        @param i_rDoc
            the document to load
        @param i_rFrame
            the frame to load the document into
        @param i_rLoadArgs
            the arguments to pass to the component loader. If this sequence is empty, then the current arguments of the
            model will be obtained, and passed to the loader. This ensures that any arguments in the model will be preserved,
            instead of being reset.
        @param i_nViewId
            the ID of the view to create
        @throws Exception
            if something goes wrong. The caller is responsible for handling this.
    */
    SAL_DLLPRIVATE static SfxViewShell* LoadViewIntoFrame_Impl(
                            const SfxObjectShell& i_rDoc,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rLoadArgs,
                            const sal_uInt16 i_nViewId,
                            const bool i_bHidden
                        );

    /** loads the given existing document into the given frame

        This is done using the XComponentLoader interface of the frame, so the SFX document loader is invoked.

        If no frame is given, a blank top level frame is created.

        If anything fails during the process, as much as possible is cleaned up.

        @param i_rDoc
            the document to load
        @param i_rFrame
            the frame to load the document into. Might be <NULL/>, in which case a new frame is created.
        @param i_nViewId
            the ID of the view to create
    */
    SAL_DLLPRIVATE static SfxViewFrame* LoadViewIntoFrame_Impl_NoThrow(
                            const SfxObjectShell& i_rDoc,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
                            const sal_uInt16 i_nViewId,
                            const bool i_bHidden
                        );
};

//--------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxViewFrameItem: public SfxPoolItem
{
    SfxViewFrame*           pFrame;

public:
                            TYPEINFO();
                            SfxViewFrameItem( SfxViewFrame *pViewFrame ):
                                SfxPoolItem( 0 ),
                                pFrame( pViewFrame)
                            {}
                            SfxViewFrameItem( sal_uInt16 nWhichId, SfxViewFrame *pViewFrame ):
                                SfxPoolItem( nWhichId ),
                                pFrame( pViewFrame)
                            {}

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual String          GetValueText() const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    SfxViewFrame*           GetFrame() const
                            { return pFrame; }
};

class SfxVerbListItem : public SfxPoolItem
{
    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > aVerbs;

public:
                            TYPEINFO();
                            SfxVerbListItem( sal_uInt16 nWhichId = SID_OBJECT ) :
                                SfxPoolItem( nWhichId )
                            {}

                            SfxVerbListItem( sal_uInt16 nWhichId, const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& );

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& GetVerbList() const { return aVerbs; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
