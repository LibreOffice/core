/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

class SfxMacro;
class SvBorder;
class SfxDispatcher;
class SfxObjectShell;
class SfxBindings;
class SfxProgress;
class SvData;
class SfxViewShell;
class SvPseudoObject;
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
    USHORT                      nAdjustPosPixelLock;

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

    static SfxViewFrame*    LoadHiddenDocument( SfxObjectShell& i_rDoc, const USHORT i_nViewId );
    static SfxViewFrame*    LoadDocument( SfxObjectShell& i_rDoc, const USHORT i_nViewId );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const SfxFrameItem* i_pFrameItem, const USHORT i_nViewId = 0 );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrameItem, const USHORT i_nViewId = 0 );
    static SfxViewFrame*    DisplayNewDocument( SfxObjectShell& i_rDoc, const SfxRequest& i_rCreateDocRequest, const USHORT i_nViewId = 0 );

    static SfxViewFrame*    Current();
    static SfxViewFrame*    GetFirst( const SfxObjectShell* pDoc = 0, BOOL bOnlyVisible = TRUE );
    static SfxViewFrame*    GetNext( const SfxViewFrame& rPrev, const SfxObjectShell* pDoc = 0, BOOL bOnlyVisible = TRUE );
    static USHORT           Count();

    static SfxViewFrame*    Get( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>& i_rController, const SfxObjectShell* i_pDoc = NULL );

            void            DoActivate(BOOL bMDI, SfxViewFrame *pOld=NULL);
            void            DoDeactivate(BOOL bMDI, SfxViewFrame *pOld=NULL);

    SfxViewFrame*           GetParentViewFrame() const;

    using SfxShell::GetDispatcher;
        SfxDispatcher*          GetDispatcher() { return pDispatcher; }
    SfxBindings&            GetBindings() { return *pBindings; }
    const SfxBindings&      GetBindings() const  { return *pBindings; }
    Window&                 GetWindow() const;
    virtual void            SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    SfxProgress*            GetProgress() const;
#ifdef ENABLE_INIMANAGER//MUSTINI
    SfxIniManager*          GetIniManager() const;
#endif

    SfxObjectShell*         GetObjectShell() const
                            { return xObjSh; }

    void                    DoAdjustPosSize( SfxViewShell *pSh,
                                        const Point rPos, const Size &rSize );
    void                    LockAdjustPosSizePixel()
                            { nAdjustPosPixelLock++; }
    void                    UnlockAdjustPosSizePixel()
                            { nAdjustPosPixelLock--; }
    void                    DoAdjustPosSizePixel( SfxViewShell * pSh,
                                        const Point &rPos, const Size &rSize );
    void                    Hide();
    void                    Show();
    BOOL                    IsVisible() const;
    void                    ToTop();
    void                    Enable( BOOL bEnable );
    virtual BOOL            Close();
    virtual void            Activate( BOOL bUI );
    virtual void            Deactivate( BOOL bUI );

    // DDE-Interface
    virtual long            DdeExecute( const String& rCmd );
    virtual long            DdeGetData( const String& rItem,
                                        const String& rMimeType,
                                        ::com::sun::star::uno::Any & rValue );
    virtual long            DdeSetData( const String& rItem,
                                        const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource*   DdeCreateLinkSource( const String& rItem );

    void                    ShowStatusText( const String& rText );
    void                    HideStatusText();

    String                  UpdateTitle();

    static void ActivateToolPanel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const ::rtl::OUString& i_rPanelURL );

    // interne Handler
    SAL_DLLPRIVATE virtual BOOL SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    SAL_DLLPRIVATE virtual const SvBorder& GetBorderPixelImpl( const SfxViewShell *pSh ) const;
    SAL_DLLPRIVATE virtual void InvalidateBorderImpl( const SfxViewShell *pSh );

    virtual SfxObjectShell* GetObjectShell();
    USHORT                  GetCurViewId() const;
    SfxFrame&               GetFrame() const;
    SfxViewFrame*           GetTopViewFrame() const;

    BOOL                    DoClose();
    ULONG                   GetFrameType() const
                            { return GetFrame().GetFrameType(); }
    SfxFrame&               GetTopFrame() const
                            { return GetFrame().GetTopFrame(); }
    void                    GetTargetList( TargetList& rList ) const
                            { GetFrame().GetTargetList( rList ); }
    void                    CancelTransfers()
                            { GetFrame().CancelTransfers(); }

    void                    SetModalMode( BOOL );
    BOOL                    IsInModalMode() const;
    void                    Resize(BOOL bForce=FALSE);

    //void                        SetChildWindow(USHORT nId, BOOL bVisible );
    void                        SetChildWindow(USHORT nId, BOOL bVisible, BOOL bSetFocus=TRUE);
    void                        ToggleChildWindow(USHORT);
    BOOL                        HasChildWindow(USHORT);
    BOOL                        KnowsChildWindow(USHORT);
    void                        ShowChildWindow(USHORT,BOOL bVisible=TRUE);
    SfxChildWindow*             GetChildWindow(USHORT);
    void                        ChildWindowExecute(SfxRequest&);
    void                        ChildWindowState(SfxItemSet&);

    SAL_DLLPRIVATE void SetDowning_Impl();
    SAL_DLLPRIVATE void GetDocNumber_Impl();
    SAL_DLLPRIVATE BOOL IsDowning_Impl() const;
    SAL_DLLPRIVATE void SetViewShell_Impl( SfxViewShell *pVSh );
    SAL_DLLPRIVATE void ReleaseObjectShell_Impl();

    SAL_DLLPRIVATE void GetState_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq, BOOL bAsync );
    SAL_DLLPRIVATE void StateReload_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecView_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateView_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecHistory_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateHistory_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE SfxViewFrame* GetParentViewFrame_Impl() const;
    SAL_DLLPRIVATE void ForceOuterResize_Impl(BOOL bOn=TRUE);
    SAL_DLLPRIVATE BOOL IsResizeInToOut_Impl() const;
    SAL_DLLPRIVATE BOOL IsAdjustPosSizePixelLocked_Impl() const
                            { return nAdjustPosPixelLock != 0; }
    SAL_DLLPRIVATE void ForceInnerResize_Impl( BOOL bOn );
    SAL_DLLPRIVATE void UpdateDocument_Impl();

    SAL_DLLPRIVATE void LockObjectShell_Impl(BOOL bLock=TRUE);

    SAL_DLLPRIVATE void MakeActive_Impl( BOOL bActivate );
    SAL_DLLPRIVATE void SetQuietMode_Impl( BOOL );
    SAL_DLLPRIVATE const Size& GetMargin_Impl() const;
    SAL_DLLPRIVATE void SetActiveChildFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE SfxViewFrame* GetActiveChildFrame_Impl() const;
    SAL_DLLPRIVATE String GetActualPresentationURL_Impl() const;
    SAL_DLLPRIVATE static void CloseHiddenFrames_Impl();
    SAL_DLLPRIVATE void MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl( USHORT nId );
    SAL_DLLPRIVATE void AddDispatchMacroToBasic_Impl(const ::rtl::OUString& sMacro);

    SAL_DLLPRIVATE void Exec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void SetCurViewId_Impl( const USHORT i_nID );
    SAL_DLLPRIVATE void ActivateToolPanel_Impl( const ::rtl::OUString& i_rPanelURL );

private:
    SAL_DLLPRIVATE BOOL SwitchToViewShell_Impl( USHORT nNo, BOOL bIsIndex = FALSE );
    SAL_DLLPRIVATE void PopShellAndSubShells_Impl( SfxViewShell& i_rViewShell );
    SAL_DLLPRIVATE void SaveCurrentViewData_Impl( const USHORT i_nNewViewId );

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
                            const USHORT i_nViewId,
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
                            const USHORT i_nViewId,
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
                            SfxViewFrameItem( USHORT nWhichId, SfxViewFrame *pViewFrame ):
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
                            SfxVerbListItem( USHORT nWhichId = SID_OBJECT ) :
                                SfxPoolItem( nWhichId )
                            {}

                            SfxVerbListItem( USHORT nWhichId, const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& );

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& GetVerbList() const { return aVerbs; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
