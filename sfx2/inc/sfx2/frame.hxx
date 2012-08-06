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
#include <tools/ref.hxx>
#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <svl/poolitem.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <vector>

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
class SfxDispatcher;
class SfxMedium;
class Rectangle;
class SfxRequest;
class SfxUnoControllerItem;
class SvCompatWeakHdl;
class SystemWindow;

class SfxFrame;
typedef ::std::vector<SfxFrame*> SfxFrameArr_Impl;

typedef ::std::vector< String* > TargetList;

#define SFXFRAME_HASTITLE     0x0001

//==========================================================================
// SfxFrame is a management class for windows and their content.
// A SfxApplication represent a hierarchy of SfxFrames, with which the actual
// content in the derived classes is defined. The base class SfxFrame
// implements two aspects of frames: naming and control of its lifespan.
// Inside a frame hierarchy the parent frame always controls the lifespan of
// its child frames, even though they usually are not even produced by the
// parent. By calling DoCloser() on any frame in the hierarchy,
// a part of the "framework" can be removed, where frames unsubscribe
// from their parent frames.
//==========================================================================

struct SfxFramePickEntry_Impl;
class SFX2_DLLPUBLIC SfxFrame
{
    friend class SfxFrameIterator;
    friend class SfxFrameWindow_Impl;

private:
    SfxFrame*           pParentFrame;
    SfxFrameArr_Impl*   pChildArr;
    SfxFrame_Impl*      pImp;
    Window*             pWindow;

protected:
    sal_Bool            Close();
    virtual             ~SfxFrame();

    SAL_DLLPRIVATE void RemoveChildFrame_Impl( SfxFrame* );

                        SfxFrame( );    // not implemented
    SAL_DLLPRIVATE      SfxFrame( Window& i_rContainerWindow, bool bHidden );

public:
                        TYPEINFO();

    static SfxFrame*    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        CreateBlankFrame();
    static SfxFrame*    Create( SfxObjectShell& rDoc, Window& rWindow, sal_uInt16 nViewId, bool bHidden );

    SvCompatWeakHdl*    GetHdl();
    Window&             GetWindow() const;
    void                CancelTransfers( sal_Bool bCancelLoadEnv = sal_True );
    sal_Bool            DoClose();
    sal_uInt16          GetChildFrameCount() const;
    SfxFrame*           GetChildFrame( sal_uInt16 nPos ) const;
    SfxFrame*           GetParentFrame() const
                        { return pParentFrame; }

    void                SetPresentationMode( sal_Bool bSet );
    SystemWindow*       GetSystemWindow() const;

    static SfxFrame*    GetFirst();
    static SfxFrame*    GetNext( SfxFrame& );

    static const SfxPoolItem*
                        OpenDocumentSynchron( SfxItemSet& aSet, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rTargetFrame );

    SfxObjectShell*     GetCurrentDocument() const;
    SfxViewFrame*       GetCurrentViewFrame() const;
    SfxFrame&           GetTopFrame() const;
    sal_Bool            IsParent( SfxFrame* ) const;

    sal_uInt32          GetFrameType() const;
    void                GetTargetList( TargetList& ) const;
    SAL_DLLPRIVATE SfxFrame* GetContainingDocFrame_Impl( SfxFrame* pSelf );
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

    sal_Bool            IsInPlace() const;

    SAL_DLLPRIVATE sal_Bool DoClose_Impl();
    SAL_DLLPRIVATE void SetFrameInterface_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
    SAL_DLLPRIVATE void ReleasingComponent_Impl( sal_Bool bSet );
    SAL_DLLPRIVATE void GetViewData_Impl();
    SAL_DLLPRIVATE void SetFrameType_Impl( sal_uInt32 );
    SAL_DLLPRIVATE sal_uInt16 PrepareClose_Impl( sal_Bool bUI, sal_Bool bForBrowsing=sal_False );
    SAL_DLLPRIVATE sal_Bool DocIsModified_Impl();
    SAL_DLLPRIVATE void SetCurrentViewFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE sal_Bool IsClosing_Impl() const;
    SAL_DLLPRIVATE void SetIsClosing_Impl();

    // Methods for accessing the current set
    SAL_DLLPRIVATE SfxFrameDescriptor* GetDescriptor() const;

    SAL_DLLPRIVATE void Lock_Impl( sal_Bool bLock );
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsAutoLoadLocked_Impl() const;

    SAL_DLLPRIVATE static void InsertTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE static void RemoveTopFrame_Impl( SfxFrame* pFrame );
    SAL_DLLPRIVATE void SetOwnsBindings_Impl( sal_Bool bSet );
    SAL_DLLPRIVATE sal_Bool OwnsBindings_Impl() const;
    SAL_DLLPRIVATE void InvalidateUnoControllers_Impl();
    SAL_DLLPRIVATE void RegisterUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE void ReleaseUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
    SAL_DLLPRIVATE void SetToolSpaceBorderPixel_Impl( const SvBorder& );
    SAL_DLLPRIVATE Rectangle GetTopOuterRectPixel_Impl() const;
    SAL_DLLPRIVATE void CreateWorkWindow_Impl();
    SAL_DLLPRIVATE void GrabFocusOnComponent_Impl();
    SAL_DLLPRIVATE void SetInPlace_Impl( sal_Bool );

    SAL_DLLPRIVATE void PrepareForDoc_Impl( SfxObjectShell& i_rDoc );
    SAL_DLLPRIVATE void LockResize_Impl( sal_Bool bLock );
    SAL_DLLPRIVATE void SetMenuBarOn_Impl( sal_Bool bOn );
    SAL_DLLPRIVATE sal_Bool IsMenuBarOn_Impl() const;
    SAL_DLLPRIVATE SystemWindow* GetTopWindow_Impl() const;
    SAL_DLLPRIVATE bool IsMarkedHidden_Impl() const;
private:
    SAL_DLLPRIVATE void Construct_Impl();
};

SV_DECL_COMPAT_WEAK( SfxFrame )

class SfxFrameIterator
{
    const SfxFrame*         pFrame;
    sal_Bool                bRecursive;

    SfxFrame*               NextSibling_Impl( SfxFrame& rPrev );

public:
                            SfxFrameIterator( const SfxFrame& rFrame, sal_Bool bRecursive=sal_True );
    SfxFrame*               FirstFrame();
    SfxFrame*               NextFrame( SfxFrame& rPrev );
};

//--------------------------------------------------------------------

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

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool                QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool                PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

class SFX2_DLLPUBLIC SfxUnoFrameItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                                m_xFrame;

public:
                                TYPEINFO();
                                SfxUnoFrameItem();
                                SfxUnoFrameItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame );
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&
                                GetFrame() const
                                { return m_xFrame; }
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool                QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool                PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

typedef SfxUsrAnyItem SfxUnoAnyItem;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
