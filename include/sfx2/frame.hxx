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
#ifndef INCLUDED_SFX2_FRAME_HXX
#define INCLUDED_SFX2_FRAME_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/shell.hxx>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.hxx>
#include <vcl/vclptr.hxx>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <tools/ref.hxx>
#include <memory>
#include <vector>


namespace com::sun::star::frame
{
    class XFrame;
    class XController;
}

class SvBorder;
class SfxWorkWindow;
namespace vcl { class Window; }
class SfxFrame_Impl;
class SfxObjectShell;
class SfxViewFrame;
class SfxFrameDescriptor;
namespace tools { class Rectangle; }
class SystemWindow;

typedef ::std::vector<OUString> TargetList;


// SfxFrame is a management class for windows and their content.
// A SfxApplication represent a hierarchy of SfxFrames, with which the actual
// content in the derived classes is defined. The base class SfxFrame
// implements two aspects of frames: naming and control of its lifespan.
// Inside a frame hierarchy the parent frame always controls the lifespan of
// its child frames, even though they usually are not even produced by the
// parent. By calling DoCloser() on any frame in the hierarchy,
// a part of the "framework" can be removed, where frames unsubscribe
// from their parent frames.


class SFX2_DLLPUBLIC SfxFrame final : public SvCompatWeakBase<SfxFrame>
{
    friend class SfxFrameIterator;
    friend class SfxFrameWindow_Impl;

private:
    std::unique_ptr< SfxFrame_Impl >     m_pImpl;
    VclPtr<vcl::Window> m_pWindow;

    virtual             ~SfxFrame();

    SAL_DLLPRIVATE      SfxFrame( vcl::Window& i_rContainerWindow );

public:
    static SfxFrame*    Create( const css::uno::Reference< css::frame::XFrame >& xFrame );
    static css::uno::Reference< css::frame::XFrame >
                        CreateBlankFrame();
    static SfxFrame*    CreateHidden( SfxObjectShell const & rDoc, vcl::Window& rWindow, SfxInterfaceId nViewId );

    vcl::Window&        GetWindow() const { return *m_pWindow;}
    void                CancelTransfers();
    bool                DoClose();

    void                SetPresentationMode( bool bSet );
    SystemWindow*       GetSystemWindow() const;

    SAL_WARN_UNUSED_RESULT static SfxFrame* GetFirst();
    SAL_WARN_UNUSED_RESULT static SfxFrame* GetNext( SfxFrame& );

    SAL_WARN_UNUSED_RESULT SfxObjectShell* GetCurrentDocument() const;
    SAL_WARN_UNUSED_RESULT SfxViewFrame* GetCurrentViewFrame() const;

    bool                GetHasTitle() const;
    static void         GetDefaultTargetList( TargetList& );
    void                UpdateDescriptor( SfxObjectShell const *pDoc );
    void                Resize();
    const css::uno::Reference< css::frame::XFrame >&
                        GetFrameInterface() const;
    void                Appear();
    void                AppearWithUpdate();
    css::uno::Reference< css::frame::XController >
                        GetController() const;

    bool                IsInPlace() const;

    SAL_DLLPRIVATE void DoClose_Impl();
    SAL_DLLPRIVATE void SetFrameInterface_Impl( const css::uno::Reference< css::frame::XFrame >& rFrame );
    SAL_DLLPRIVATE void ReleasingComponent_Impl();
    SAL_DLLPRIVATE void GetViewData_Impl();
    SAL_DLLPRIVATE void SetHasTitle( bool );
    SAL_DLLPRIVATE bool PrepareClose_Impl( bool bUI );
    SAL_DLLPRIVATE bool DocIsModified_Impl();
    SAL_DLLPRIVATE void SetCurrentViewFrame_Impl( SfxViewFrame* );
    bool IsClosing_Impl() const;
    SAL_DLLPRIVATE void SetIsClosing_Impl();

    // Methods for accessing the current set
    SAL_DLLPRIVATE SfxFrameDescriptor* GetDescriptor() const;

    SAL_DLLPRIVATE bool IsAutoLoadLocked_Impl() const;

    SAL_DLLPRIVATE static void InsertTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE static void RemoveTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE void SetOwnsBindings_Impl( bool bSet );
    SAL_DLLPRIVATE bool OwnsBindings_Impl() const;
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
    SAL_DLLPRIVATE void SetToolSpaceBorderPixel_Impl( const SvBorder& );
    SAL_DLLPRIVATE tools::Rectangle GetTopOuterRectPixel_Impl() const;
    SAL_DLLPRIVATE void CreateWorkWindow_Impl();
    SAL_DLLPRIVATE void GrabFocusOnComponent_Impl();

    SAL_DLLPRIVATE void PrepareForDoc_Impl( const SfxObjectShell& i_rDoc );
    SAL_DLLPRIVATE void LockResize_Impl( bool bLock );
    SAL_DLLPRIVATE void SetMenuBarOn_Impl( bool bOn );
    SAL_DLLPRIVATE bool IsMenuBarOn_Impl() const;
    SAL_DLLPRIVATE SystemWindow* GetTopWindow_Impl() const;
    SAL_DLLPRIVATE bool IsMarkedHidden_Impl() const;
private:
    SAL_DLLPRIVATE void Construct_Impl();
};

typedef SvCompatWeakRef<SfxFrame> SfxFrameWeakRef;

class SFX2_DLLPUBLIC SfxFrameItem final : public SfxPoolItem
{
    SfxFrame*               pFrame;
    SfxFrameWeakRef         wFrame;

public:
                            DECLARE_ITEM_TYPE_FUNCTION(SfxFrameItem)
                            SfxFrameItem( sal_uInt16 nWhich, SfxViewFrame const *p );
                            SfxFrameItem( sal_uInt16 nWhich, SfxFrame *p );

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxFrameItem*   Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    SfxFrame*               GetFrame() const { return wFrame; }
};

class SFX2_DLLPUBLIC SfxUnoAnyItem final : public SfxPoolItem
{
    css::uno::Any  aValue;
public:
                                static SfxPoolItem* CreateDefault();
                                DECLARE_ITEM_TYPE_FUNCTION(SfxUnoAnyItem)
                                SfxUnoAnyItem( sal_uInt16 nWhich, const css::uno::Any& rAny );
    const css::uno::Any&        GetValue() const
                                { return aValue; }
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual SfxUnoAnyItem*      Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool                QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool                PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

class SFX2_DLLPUBLIC SfxUnoFrameItem final : public SfxPoolItem
{
    css::uno::Reference< css::frame::XFrame >
                                m_xFrame;

public:
                                static SfxPoolItem* CreateDefault();
                                DECLARE_ITEM_TYPE_FUNCTION(SfxUnoFrameItem)
                                SfxUnoFrameItem();
                                SfxUnoFrameItem( sal_uInt16 nWhich, css::uno::Reference< css::frame::XFrame > i_xFrame );
    const css::uno::Reference< css::frame::XFrame >&
                                GetFrame() const
                                { return m_xFrame; }
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual SfxUnoFrameItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool                QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool                PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
