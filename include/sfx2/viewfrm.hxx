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
#ifndef INCLUDED_SFX2_VIEWFRM_HXX
#define INCLUDED_SFX2_VIEWFRM_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <svl/lstner.hxx>
#include <sfx2/module.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/poolitem.hxx>
#include <vcl/button.hxx>

#include <tools/svborder.hxx>

class SvBorder;
class SfxDispatcher;
class SfxBindings;
class SfxProgress;
class SvData;
class SfxViewShell;
class SystemWindow;
class Fraction;
class Point;
class Size;
class SfxChildWindow;
class SfxInfoBarWindow;
enum class InfoBarType;

namespace sfx2
{
class SvLinkSource;
}
namespace svtools
{
    class AsynchronLink;
}
namespace basegfx
{
    class BColor;
}

class SFX2_DLLPUBLIC SfxViewFrame: public SfxShell, public SfxListener
{
    std::unique_ptr<struct SfxViewFrame_Impl>   m_pImpl;

    SfxObjectShellRef           m_xObjSh;
    std::unique_ptr<SfxDispatcher> m_pDispatcher;
    SfxBindings*                m_pBindings;
    sal_uInt16                  m_nAdjustPosPixelLock;

private:
    SAL_DLLPRIVATE void Construct_Impl( SfxObjectShell *pObjSh );

protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    DECL_LINK(GetInvolvedHandler, Button*, void);
    DECL_LINK(SwitchReadOnlyHandler, Button*, void);
    DECL_LINK(SignDocumentHandler, Button*, void);
    SAL_DLLPRIVATE void KillDispatcher_Impl();

    virtual                 ~SfxViewFrame() override;

public:
                            SfxViewFrame( SfxFrame& rFrame, SfxObjectShell *pDoc );

                            SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWFRM)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:

    static void             SetViewFrame( SfxViewFrame* );

    static SfxViewFrame*    LoadHiddenDocument( SfxObjectShell const & i_rDoc, SfxInterfaceId i_nViewId );
    static SfxViewFrame*    LoadDocument( SfxObjectShell const & i_rDoc, SfxInterfaceId i_nViewId );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell const & i_rDoc, const SfxFrameItem* i_pFrameItem, SfxInterfaceId i_nViewId );
    static SfxViewFrame*    LoadDocumentIntoFrame( SfxObjectShell const & i_rDoc, const css::uno::Reference< css::frame::XFrame >& i_rFrameItem );
    static SfxViewFrame*    DisplayNewDocument( SfxObjectShell const & i_rDoc, const SfxRequest& i_rCreateDocRequest );

    static SfxViewFrame*    Current();
    static SfxViewFrame*    GetFirst( const SfxObjectShell* pDoc = nullptr, bool bOnlyVisible = true );
    static SfxViewFrame*    GetNext( const SfxViewFrame& rPrev, const SfxObjectShell* pDoc = nullptr, bool bOnlyVisible = true );

    static SfxViewFrame*    Get( const css::uno::Reference< css::frame::XController>& i_rController, const SfxObjectShell* i_pDoc );

            void            DoActivate(bool bMDI);
            void            DoDeactivate(bool bMDI, SfxViewFrame const *pOld);

    using SfxShell::GetDispatcher;
    SfxDispatcher*          GetDispatcher() { return m_pDispatcher.get(); }
    SfxBindings&            GetBindings() { return *m_pBindings; }
    const SfxBindings&      GetBindings() const  { return *m_pBindings; }
    vcl::Window&            GetWindow() const;

    SfxProgress*            GetProgress() const;

    void                    LockAdjustPosSizePixel()
                            { m_nAdjustPosPixelLock++; }
    void                    UnlockAdjustPosSizePixel()
                            { m_nAdjustPosPixelLock--; }
    void                    DoAdjustPosSizePixel( SfxViewShell * pSh,
                                        const Point &rPos, const Size &rSize,
                                        bool inplaceEditModeChange );
    void                    Show();
    bool                    IsVisible() const;
    void                    ToTop();
    void                    Enable( bool bEnable );
    bool                    Close();
    virtual void            Activate( bool bUI ) override;
    virtual void            Deactivate( bool bUI ) override;

    void                    UpdateTitle();

    // interne Handler
    SAL_DLLPRIVATE void SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    SAL_DLLPRIVATE const SvBorder& GetBorderPixelImpl() const;
    SAL_DLLPRIVATE void InvalidateBorderImpl( const SfxViewShell *pSh );

    virtual SfxObjectShell* GetObjectShell() override;
    SfxInterfaceId          GetCurViewId() const;
    SfxFrame&               GetFrame() const;
    SfxViewFrame*           GetTopViewFrame() const;

    bool                    DoClose();
    bool                    GetHasTitle() const
                            { return GetFrame().GetHasTitle(); }
    static void             GetTargetList( TargetList& rList )
                            { SfxFrame::GetDefaultTargetList( rList ); }

    void                    SetModalMode( bool );
    bool                    IsInModalMode() const;
    void                    Resize(bool bForce=false);

    void                    SetChildWindow(sal_uInt16 nId, bool bVisible, bool bSetFocus=true);
    void                    ToggleChildWindow(sal_uInt16);
    bool                    HasChildWindow(sal_uInt16);
    bool                    KnowsChildWindow(sal_uInt16);
    void                    ShowChildWindow(sal_uInt16,bool bVisible=true);
    SfxChildWindow*         GetChildWindow(sal_uInt16);
    void                    ChildWindowExecute(SfxRequest&);
    void                    ChildWindowState(SfxItemSet&);

    /** Append a new InfoBar (see https://wiki.documentfoundation.org/Design/Whiteboards/Infobar).

        The buttons will be added from Right to Left at the right of the info bar. The parent, size
        and position of each button will be changed: only the width will remain unchanged.
      */
    VclPtr<SfxInfoBarWindow> AppendInfoBar(const OUString& sId,
                                    const OUString& sMessage,
                                    InfoBarType aInfoBarType);
    void              RemoveInfoBar(const OUString& sId);
    void              UpdateInfoBar(const OUString& sId,
                               const OUString& sMessage, InfoBarType eType);
    bool              HasInfoBarWithID(const OUString& sId);

    SAL_DLLPRIVATE void GetDocNumber_Impl();
    SAL_DLLPRIVATE void SetViewShell_Impl( SfxViewShell *pVSh );
    SAL_DLLPRIVATE void ReleaseObjectShell_Impl();

    SAL_DLLPRIVATE void GetState_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateReload_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecView_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateView_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecHistory_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateHistory_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ForceOuterResize_Impl();
    SAL_DLLPRIVATE void UpdateDocument_Impl();

    SAL_DLLPRIVATE void LockObjectShell_Impl();

    SAL_DLLPRIVATE void MakeActive_Impl( bool bActivate );
    SAL_DLLPRIVATE const Size& GetMargin_Impl() const;
    SAL_DLLPRIVATE OUString GetActualPresentationURL_Impl() const;
    SAL_DLLPRIVATE void MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl();
    SAL_DLLPRIVATE void AddDispatchMacroToBasic_Impl(const OUString& sMacro);

    SAL_DLLPRIVATE void Exec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void SetCurViewId_Impl( const SfxInterfaceId i_nID );

private:
    SAL_DLLPRIVATE bool SwitchToViewShell_Impl( sal_uInt16 nNo, bool bIsIndex = false );
    SAL_DLLPRIVATE void PopShellAndSubShells_Impl( SfxViewShell& i_rViewShell );
    SAL_DLLPRIVATE void SaveCurrentViewData_Impl( const SfxInterfaceId i_nNewViewId );

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
                            const css::uno::Reference< css::frame::XFrame >& i_rFrame,
                            const css::uno::Sequence< css::beans::PropertyValue >& i_rLoadArgs,
                            const SfxInterfaceId i_nViewId,
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
                            const css::uno::Reference< css::frame::XFrame >& i_rFrame,
                            const SfxInterfaceId i_nViewId,
                            const bool i_bHidden
                        );
};


class SFX2_DLLPUBLIC SfxViewFrameItem: public SfxPoolItem
{
    SfxViewFrame*           pFrame;

public:
                            SfxViewFrameItem( SfxViewFrame *pViewFrame ):
                                SfxPoolItem( 0 ),
                                pFrame( pViewFrame)
                            {}

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    SfxViewFrame*           GetFrame() const
                            { return pFrame; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
