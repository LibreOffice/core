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

#include <config_features.h>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <workwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include <splitwin.hxx>
#include <childwinimpl.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/request.hxx>
#include <sfx2/toolbarids.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>
#include <tools/diagnose_ex.h>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <type_traits>
#include <unordered_map>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

struct ResIdToResName
{
    ToolbarId const   eId;
    const char* pName;
};

static const ResIdToResName pToolBarResToName[] =
{
    { ToolbarId::FullScreenToolbox,       "fullscreenbar"        },
    { ToolbarId::EnvToolbox,              "standardbar",         },
    { ToolbarId::SvxTbx_Form_Navigation,  "formsnavigationbar"   },
    { ToolbarId::SvxTbx_Form_Filter,      "formsfilterbar"       },
    { ToolbarId::SvxTbx_Text_Control_Attributes, "formtextobjectbar"    },
    { ToolbarId::SvxTbx_Controls,         "formcontrols"         },
    { ToolbarId::SvxTbx_FormDesign,       "formdesign"           },
    { ToolbarId::Math_Toolbox,            "toolbar"              },      //math
    { ToolbarId::Text_Toolbox_Sc,         "textobjectbar"        },      //calc
    { ToolbarId::Draw_Objectbar,          "drawobjectbar"        },
    { ToolbarId::Graphic_Objectbar,       "graphicobjectbar"     },
    { ToolbarId::Objectbar_Format,        "formatobjectbar"      },
    { ToolbarId::Objectbar_Preview,       "previewbar"           },
    { ToolbarId::Objectbar_Tools,         "toolbar"              },      //calc
    { ToolbarId::Bezier_Toolbox_Sd,       "bezierobjectbar"      },      //draw/impress
    { ToolbarId::Gluepoints_Toolbox,      "gluepointsobjectbar"  },
    { ToolbarId::Draw_Graf_Toolbox,       "graphicobjectbar"     },
    { ToolbarId::Draw_Obj_Toolbox,        "drawingobjectbar"     },      //impress
    { ToolbarId::Draw_Text_Toolbox_Sd,    "textobjectbar"        },      //impress
    { ToolbarId::Draw_Toolbox_Sd,         "toolbar"              },      //impress
    { ToolbarId::Draw_Options_Toolbox,    "optionsbar"           },
    { ToolbarId::Draw_CommonTask_Toolbox, "commontaskbar"        },
    { ToolbarId::Graphic_Obj_Toolbox,     "drawingobjectbar"     },      //draw
    { ToolbarId::Outline_Toolbox,         "outlinetoolbar"       },      //impress
    { ToolbarId::Slide_Toolbox,           "slideviewtoolbar"     },
    { ToolbarId::Slide_Obj_Toolbox,       "slideviewobjectbar"   },
    { ToolbarId::Bezier_Toolbox_Sw,       "bezierobjectbar"      },
    { ToolbarId::Draw_Toolbox_Sw,         "drawingobjectbar"     },
    { ToolbarId::Draw_Text_Toolbox_Sw,    "drawtextobjectbar"    },
    { ToolbarId::Frame_Toolbox,           "frameobjectbar"       },
    { ToolbarId::Grafik_Toolbox,          "graphicobjectbar"     },
    { ToolbarId::Num_Toolbox,             "numobjectbar"         },
    { ToolbarId::Ole_Toolbox,             "oleobjectbar"         },
    { ToolbarId::Table_Toolbox,           "tableobjectbar"       },
    { ToolbarId::Text_Toolbox_Sw,         "textobjectbar"        },
    { ToolbarId::PView_Toolbox,           "previewobjectbar"     },      //writer
    { ToolbarId::Webtools_Toolbox,        "toolbar"              },      //web
    { ToolbarId::Webtext_Toolbox,         "textobjectbar"        },
    { ToolbarId::Tools_Toolbox,           "toolbar"              },      //writer
    { ToolbarId::Webframe_Toolbox,        "frameobjectbar"       },      //web
    { ToolbarId::Webgraphic_Toolbox,      "graphicobjectbar"     },
    { ToolbarId::Webole_Toolbox,          "oleobjectbar"         },
    { ToolbarId::Basicide_Objectbar,      "macrobar"             },
    { ToolbarId::Svx_Fontwork_Bar,        "fontworkobjectbar"    },      //global
    { ToolbarId::Svx_Extrusion_Bar,       "extrusionobjectbar"   },
    { ToolbarId::FormLayer_Toolbox,       "formsobjectbar"       },
    { ToolbarId::Module_Toolbox,          "viewerbar"            },      //writer (plugin)
    { ToolbarId::Objectbar_App,           "viewerbar"            },      //calc   (plugin)
    { ToolbarId::Draw_Viewer_Toolbox,     "viewerbar"            },      //impress(plugin)
    { ToolbarId::Draw_Media_Toolbox,      "mediaobjectbar"       },      //draw/impress
    { ToolbarId::Media_Objectbar,         "mediaobjectbar"       },      //calc
    { ToolbarId::Media_Toolbox,           "mediaobjectbar"       },      //writer
    { ToolbarId::None,                           ""                     }
};

// Sort the Children according their alignment
// The order corresponds to the enum SfxChildAlignment (->CHILDWIN.HXX).

static constexpr OUStringLiteral g_aLayoutManagerPropName = "LayoutManager";

// Help to make changes to the alignment compatible!
LayoutManagerListener::LayoutManagerListener(
    SfxWorkWindow* pWrkWin ) :
    m_bHasFrame( false ),
    m_pWrkWin( pWrkWin )
{
}

LayoutManagerListener::~LayoutManagerListener()
{
}

void LayoutManagerListener::setFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    SolarMutexGuard aGuard;
    if ( !m_pWrkWin || m_bHasFrame )
        return;

    m_xFrame    = xFrame;
    m_bHasFrame = true;

    if ( !xFrame.is() )
        return;

    css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
    css::uno::Reference< css::frame::XLayoutManagerEventBroadcaster > xLayoutManager;
    if ( !xPropSet.is() )
        return;

    try
    {
        Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
        aValue >>= xLayoutManager;

        if ( xLayoutManager.is() )
            xLayoutManager->addLayoutManagerEventListener(
                css::uno::Reference< css::frame::XLayoutManagerListener >(
                    static_cast< OWeakObject* >( this ), css::uno::UNO_QUERY ));

        xPropSet.set( xLayoutManager, UNO_QUERY );
        if ( xPropSet.is() )
        {
            aValue = xPropSet->getPropertyValue( "LockCount" );
            aValue >>= m_pWrkWin->m_nLock;
        }
    }
    catch ( css::lang::DisposedException& )
    {
    }
    catch ( const css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }
}


//  XComponent

void SAL_CALL LayoutManagerListener::addEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
{
    // do nothing, only internal class
}

void SAL_CALL LayoutManagerListener::removeEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
{
    // do nothing, only internal class
}

void SAL_CALL LayoutManagerListener::dispose()
{
    SolarMutexGuard aGuard;

    // reset member
    m_pWrkWin = nullptr;

    css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame.get(), css::uno::UNO_QUERY );
    if ( !xFrame.is() )
        return;

    m_xFrame.clear();
    m_bHasFrame = false;

    css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XLayoutManagerEventBroadcaster > xLayoutManager;
    if ( !xPropSet.is() )
        return;

    try
    {
        css::uno::Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
        aValue >>= xLayoutManager;

        // remove as listener from layout manager
        if ( xLayoutManager.is() )
            xLayoutManager->removeLayoutManagerEventListener(
                css::uno::Reference< css::frame::XLayoutManagerListener >(
                    static_cast< OWeakObject* >( this ), css::uno::UNO_QUERY ));
    }
    catch ( css::lang::DisposedException& )
    {
    }
    catch ( const css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }
}


//  XEventListener

void SAL_CALL LayoutManagerListener::disposing(
    const css::lang::EventObject& )
{
    SolarMutexGuard aGuard;
    m_pWrkWin = nullptr;
    m_bHasFrame = false;
    m_xFrame.clear();
}


// XLayoutManagerEventListener

void SAL_CALL LayoutManagerListener::layoutEvent(
    const css::lang::EventObject&,
    ::sal_Int16                   eLayoutEvent,
    const css::uno::Any&                        )
{
    SolarMutexGuard aGuard;
    if ( !m_pWrkWin )
        return;

    if ( eLayoutEvent == css::frame::LayoutManagerEvents::VISIBLE )
    {
        m_pWrkWin->MakeVisible_Impl( true );
        m_pWrkWin->ShowChildren_Impl();
        m_pWrkWin->ArrangeChildren_Impl();
    }
    else if ( eLayoutEvent == css::frame::LayoutManagerEvents::INVISIBLE )
    {
        m_pWrkWin->MakeVisible_Impl( false );
        m_pWrkWin->HideChildren_Impl();
        m_pWrkWin->ArrangeChildren_Impl();
    }
    else if ( eLayoutEvent == css::frame::LayoutManagerEvents::LOCK )
    {
        m_pWrkWin->Lock_Impl( true );
    }
    else if ( eLayoutEvent == css::frame::LayoutManagerEvents::UNLOCK )
    {
        m_pWrkWin->Lock_Impl( false );
    }
}

namespace
{
    struct ToolbarIdHash
    {
        size_t operator()(ToolbarId t) const
        {
            typedef std::underlying_type<ToolbarId>::type underlying_type;
            return std::hash<underlying_type>()(static_cast<underlying_type>(t));
        }
    };

    class FilledToolBarResIdToResourceURLMap
    {
    private:
        typedef std::unordered_map<ToolbarId, OUString, ToolbarIdHash> ToolBarResIdToResourceURLMap;
        ToolBarResIdToResourceURLMap m_aResIdToResourceURLMap;
    public:
        FilledToolBarResIdToResourceURLMap()
        {
            sal_Int32 nIndex( 0 );
            while (pToolBarResToName[nIndex].eId != ToolbarId::None)
            {
                OUString aResourceURL( OUString::createFromAscii( pToolBarResToName[nIndex].pName ));
                m_aResIdToResourceURLMap.emplace(pToolBarResToName[nIndex].eId, aResourceURL);
                ++nIndex;
            }
        }

        OUString findURL(ToolbarId eId) const
        {
            ToolBarResIdToResourceURLMap::const_iterator aIter = m_aResIdToResourceURLMap.find(eId);
            if ( aIter != m_aResIdToResourceURLMap.end() )
                return aIter->second;
            return OUString();
        }
    };

    class theFilledToolBarResIdToResourceURLMap
        : public rtl::Static<FilledToolBarResIdToResourceURLMap,
                             theFilledToolBarResIdToResourceURLMap>
    {
    };
}

static OUString GetResourceURLFromToolbarId(ToolbarId eId)
{
    return theFilledToolBarResIdToResourceURLMap::get().findURL(eId);
}

static sal_uInt16 TbxMatch( sal_uInt16 nPos )
{
    switch ( nPos )
    {
        case SFX_OBJECTBAR_APPLICATION :
            return 0;
        case SFX_OBJECTBAR_OPTIONS:
            return 1;
        case SFX_OBJECTBAR_MACRO:
            return 2;
        case SFX_OBJECTBAR_OBJECT:
            return 3;
        case SFX_OBJECTBAR_TOOLS:
            return 4;
        case SFX_OBJECTBAR_FULLSCREEN:
        case SFX_OBJECTBAR_COMMONTASK:
        case SFX_OBJECTBAR_RECORDING:
            return nPos+1;
        default:
            return nPos;
    }
}

static sal_uInt16 ChildAlignValue(SfxChildAlignment eAlign)
{
    sal_uInt16 ret = 17;

    switch (eAlign)
    {
        case SfxChildAlignment::HIGHESTTOP:
            ret = 1;
            break;
        case SfxChildAlignment::LOWESTBOTTOM:
            ret = 2;
            break;
        case SfxChildAlignment::FIRSTLEFT:
            ret = 3;
            break;
        case SfxChildAlignment::LASTRIGHT:
            ret = 4;
            break;
        case SfxChildAlignment::LEFT:
            ret = 5;
            break;
        case SfxChildAlignment::RIGHT:
            ret = 6;
            break;
        case SfxChildAlignment::FIRSTRIGHT:
            ret = 7;
            break;
        case SfxChildAlignment::LASTLEFT:
            ret = 8;
            break;
        case SfxChildAlignment::TOP:
            ret = 9;
            break;
        case SfxChildAlignment::BOTTOM:
            ret = 10;
            break;
        case SfxChildAlignment::TOOLBOXTOP:
            ret = 11;
            break;
        case SfxChildAlignment::TOOLBOXBOTTOM:
            ret = 12;
            break;
        case SfxChildAlignment::LOWESTTOP:
            ret = 13;
            break;
        case SfxChildAlignment::HIGHESTBOTTOM:
            ret = 14;
            break;
        case SfxChildAlignment::TOOLBOXLEFT:
            ret = 15;
            break;
        case SfxChildAlignment::TOOLBOXRIGHT:
            ret = 16;
            break;
        case SfxChildAlignment::NOALIGNMENT:
            break;  // -Wall not handled...
    }

    return ret;
}

void SfxWorkWindow::Sort_Impl()
{
    aSortedList.clear();
    for (size_t i = 0; i < aChildren.size(); ++i)
    {
        SfxChild_Impl *pCli = aChildren[i].get();
        if (pCli)
        {
            decltype(aSortedList)::size_type k;
            for (k=0; k<aSortedList.size(); k++)
                if (ChildAlignValue( aChildren[aSortedList[k]]->eAlign ) >
                    ChildAlignValue(pCli->eAlign))
                    break;
            aSortedList.insert( aSortedList.begin() + k, i );
        }
    }

    bSorted = true;
}

static constexpr OUStringLiteral g_aStatusBarResName( "private:resource/statusbar/statusbar" );
static constexpr OUStringLiteral g_aTbxTypeName( "private:resource/toolbar/" );
static constexpr OUStringLiteral g_aProgressBarResName( "private:resource/progressbar/progressbar" );

// constructor for workwin of a Frame

SfxWorkWindow::SfxWorkWindow( vcl::Window *pWin, SfxFrame *pFrm, SfxFrame* pMaster ) :
    pBindings(&pFrm->GetCurrentViewFrame()->GetBindings()),
    pWorkWin (pWin),
    pActiveChild( nullptr ),
    nUpdateMode(SfxVisibilityFlags::Standard),
    nChildren( 0 ),
    nOrigMode( SfxVisibilityFlags::Invisible ),
    bSorted( true ),
    bDockingAllowed(true),
    bInternalDockingAllowed(true),
    bAllChildrenVisible(true),
#if HAVE_FEATURE_DESKTOP
    bIsFullScreen( false ),
    bShowStatusBar( true ),
#else
    bIsFullScreen( sal_True ),
    bShowStatusBar( sal_False ),
#endif
    m_nLock( 0 ),
    pMasterFrame( pMaster ),
    pFrame( pFrm )
{
    DBG_ASSERT (pBindings, "No Bindings!");

    pBindings->SetWorkWindow_Impl( this );

    // For the ObjectBars a integral place in the Childlist is reserved,
    // so that they always come in a defined order.
    for (int i=0; i<SFX_OBJECTBAR_MAX; ++i)
        aChildren.push_back( nullptr );

    // create and initialize layout manager listener
    Reference< css::frame::XFrame > xFrame = GetFrameInterface();
    LayoutManagerListener* pLayoutManagerListener = new LayoutManagerListener( this );
    m_xLayoutManagerListener.set( static_cast< cppu::OWeakObject* >( pLayoutManagerListener ),
                                  css::uno::UNO_QUERY );
    pLayoutManagerListener->setFrame( xFrame );

    SfxShell* pConfigShell = pFrm->GetCurrentViewFrame();
    if ( pConfigShell && pConfigShell->GetObjectShell() )
    {
        bShowStatusBar = ( !pConfigShell->GetObjectShell()->IsInPlaceActive() );
        bDockingAllowed = true;
        bInternalDockingAllowed = true;
    }

    // The required split windows (one for each side) can be created
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // The SplitWindows excludes direct ChildWindows of the WorkWindows
        // and receives the docked window.

        SfxChildAlignment eAlign =
                        ( n == SFX_SPLITWINDOWS_LEFT ? SfxChildAlignment::LEFT :
                            n == SFX_SPLITWINDOWS_RIGHT ? SfxChildAlignment::RIGHT :
                            n == SFX_SPLITWINDOWS_TOP ? SfxChildAlignment::TOP :
                                SfxChildAlignment::BOTTOM );
        VclPtr<SfxSplitWindow> pSplitWin = VclPtr<SfxSplitWindow>::Create(pWorkWin, eAlign, this, true );
        pSplit[n] = pSplitWin;
    }

    nOrigMode = SfxVisibilityFlags::Standard;
    nUpdateMode = SfxVisibilityFlags::Standard;
}


// Destructor

SfxWorkWindow::~SfxWorkWindow()
{

    // Delete SplitWindows
    for (VclPtr<SfxSplitWindow> & p : pSplit)
    {
        if (p->GetWindowCount())
            ReleaseChild_Impl(*p);
        p.disposeAndClear();
    }

    // Delete help structure for Child-Windows
    DBG_ASSERT( aChildren.empty(), "dangling children" );

    if ( m_xLayoutManagerListener.is() )
        m_xLayoutManagerListener->dispose();
}

void SfxWorkWindow::Lock_Impl( bool bLock )
{
    if ( bLock )
        m_nLock++;
    else
        --m_nLock;
    if ( m_nLock<0 )
    {
        OSL_FAIL("Lock count underflow!");
        assert(m_nLock >= 0);
        m_nLock = 0;
    }

    if ( !m_nLock )
        ArrangeChildren_Impl();
}


// Helper method to release the child lists. Should the destructor not be
// called after this, instead work continues, then space for the object bars
// and split windows has to be reserved in the same way as in the constructor
// of SfxWorkWindow.

void SfxWorkWindow::DeleteControllers_Impl()
{

    // Lock SplitWindows (which means suppressing the Resize-Reaction of the
    // DockingWindows)
    for (size_t n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        VclPtr<SfxSplitWindow> const &p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    // Delete Child-Windows
    while(!aChildWins.empty())
    {
        std::unique_ptr<SfxChildWin_Impl> pCW = std::move(*aChildWins.begin());
        aChildWins.erase(aChildWins.begin());
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            pChild->Hide();

            // If the child window is a direct child window and not in a
            // SplitWindow, cancel it at the workwindow.
            // After TH a cancellation on the SplitWindow is not necessary
            // since this window is also destroyed (see below).
            if (pCW->pCli)
                ReleaseChild_Impl(*pChild->GetWindow());
            pCW->pWin = nullptr;
            pWorkWin->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( pChild->GetWindow() );
            pChild->Destroy();
        }

        // ATTENTION: The array itself is cleared after this loop!!
        // Therefore we have to set every array entry to zero as it could be
        // accessed by calling pChild->Destroy().
        // Window::NotifyAllChildren() calls SfxWorkWindow::DataChanged_Impl for
        // 8-bit displays (WM_QUERYPALETTECHANGED message due to focus change)!!
    }

    Reference< css::frame::XFrame > xFrame = GetFrameInterface();
    Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
            aValue >>= xLayoutManager;
        }
        catch ( Exception& )
        {
        }
    }

    if ( xLayoutManager.is() )
    {
        xLayoutManager->reset();

        // Delete StatusBar
        ResetStatusBar_Impl();

        // Delete ObjectBars (this is done last, so that aChildren does not
        // receive dead Pointers)
        for (SfxObjectBar_Impl & i : aObjBarList)
        {
            // Not every position must be occupied
            ToolbarId eId = i.eId;
            if (eId != ToolbarId::None)
                i.eId = ToolbarId::None;
        }
    }

    // ObjectBars are all released at once, since they occupy a
    // fixed contiguous area in the array pChild
    aChildren.clear();
    bSorted = false;

    nChildren = 0;
}


// for placing the child window.

void SfxWorkWindow::ArrangeChildren_Impl( bool bForce )
{
    if ( pFrame->IsClosing_Impl() || ( m_nLock && !bForce ))
        return;

    SfxInPlaceClient *pClient = nullptr;
    SfxViewFrame *pF = pFrame->GetCurrentViewFrame();
    if ( pF && pF->GetViewShell() )
        pClient = pF->GetViewShell()->GetIPClient();

    if ( pClient )
        return;

    aClientArea = GetTopRect_Impl();
    if ( aClientArea.IsEmpty() )
        return;

    SvBorder aBorder;
    if ( nChildren && IsVisible_Impl() )
        aBorder = Arrange_Impl();
    // If the current application document contains a IPClient, then the
    // object through SetTopToolFramePixel has to be assigned the available
    // space. The object will then point to its UITools and sets the app border
    // (-> SfxInPlaceEnv_Impl:: ArrangeChildren_Impl ()). Otherwise the
    // app border is set here directly to possibly overwrite the Border that
    // was set by an object from another document.  The object does not set
    // the SetAppBorder when it removes its UI tools so that no-dithering
    // ObjectBar arises.
    // (->SfxInPlaceEnv_Impl::ArrangeChildren_Impl())

    pMasterFrame->SetToolSpaceBorderPixel_Impl( aBorder );

    ArrangeAutoHideWindows( nullptr );
}

void SfxWorkWindow::FlushPendingChildSizes()
{
    // tdf#116865, if any windows are being resized, i.e. their
    // resize timer is active, then calling GetSizePixel on
    // them forces the timer to fire and sets the final
    // size to which they are getting resized towards.
    for (size_t i = 0; i < aChildren.size(); ++i)
    {
        SfxChild_Impl *pCli = aChildren[i].get();
        if (!pCli || !pCli->pWin)
            continue;
        (void)pCli->pWin->GetSizePixel();
    }
}

SvBorder SfxWorkWindow::Arrange_Impl()

/*  [Description]

    This method organizes all visible child windows so that the docked window
    sorted in order from the outside to the inside are placed after one
    another. If a visible window does not fit anymore into the free
    ClientArea, it is set to "not visible".
*/
{
    //tdf#116865 trigger pending sizing timers now so we arrange
    //with the final size of the client area.
    //
    //Otherwise calling GetSizePixel in the following loop will trigger the
    //timers, causing reentry into Arrange_Impl again where the inner
    //Arrange_Impl arranges with the final size, and then returns to this outer
    //Arrange_Impl which would rearrange with the old client area size
    FlushPendingChildSizes();
    aClientArea = GetTopRect_Impl();
    aUpperClientArea = aClientArea;

    SvBorder aBorder;
    if ( !nChildren )
        return aBorder;

    if (!bSorted)
        Sort_Impl();

    Point aPos;
    Size aSize;
    tools::Rectangle aTmp( aClientArea );

    for (sal_uInt16 n : aSortedList)
    {
        SfxChild_Impl* pCli = aChildren[n].get();
        if ( !pCli->pWin )
            continue;

        // First, we assume that there is room for the window.
        pCli->nVisible |= SfxChildVisibility::FITS_IN;

        // Skip invisible windows
        if (pCli->nVisible != SfxChildVisibility::VISIBLE)
            continue;

        if ( pCli->bResize )
            aSize = pCli->aSize;
        else
            aSize = pCli->pWin->GetSizePixel();

        SvBorder aTemp = aBorder;
        bool bAllowHiding = true;
        switch ( pCli->eAlign )
        {
            case SfxChildAlignment::HIGHESTTOP:
            case SfxChildAlignment::TOP:
            case SfxChildAlignment::TOOLBOXTOP:
            case SfxChildAlignment::LOWESTTOP:
                aSize.setWidth( aTmp.GetWidth() );
                if ( pCli->pWin->GetType() == WindowType::SPLITWINDOW )
                    aSize = static_cast<SplitWindow *>(pCli->pWin.get())->CalcLayoutSizePixel( aSize );
                bAllowHiding = false;
                aBorder.Top() += aSize.Height();
                aPos = aTmp.TopLeft();
                aTmp.AdjustTop(aSize.Height() );
                if ( pCli->eAlign == SfxChildAlignment::HIGHESTTOP )
                    aUpperClientArea.AdjustTop(aSize.Height() );
                break;

            case SfxChildAlignment::LOWESTBOTTOM:
            case SfxChildAlignment::BOTTOM:
            case SfxChildAlignment::TOOLBOXBOTTOM:
            case SfxChildAlignment::HIGHESTBOTTOM:
                aSize.setWidth( aTmp.GetWidth() );
                if ( pCli->pWin->GetType() == WindowType::SPLITWINDOW )
                    aSize = static_cast<SplitWindow *>(pCli->pWin.get())->CalcLayoutSizePixel( aSize );
                aBorder.Bottom() += aSize.Height();
                aPos = aTmp.BottomLeft();
                aPos.AdjustY( -(aSize.Height()-1) );
                aTmp.AdjustBottom( -(aSize.Height()) );
                if ( pCli->eAlign == SfxChildAlignment::LOWESTBOTTOM )
                    aUpperClientArea.AdjustBottom( -(aSize.Height()) );
                break;

            case SfxChildAlignment::FIRSTLEFT:
            case SfxChildAlignment::LEFT:
            case SfxChildAlignment::LASTLEFT:
            case SfxChildAlignment::TOOLBOXLEFT:
                aSize.setHeight( aTmp.GetHeight() );
                if ( pCli->pWin->GetType() == WindowType::SPLITWINDOW )
                    aSize = static_cast<SplitWindow *>(pCli->pWin.get())->CalcLayoutSizePixel( aSize );
                bAllowHiding = false;
                aBorder.Left() += aSize.Width();
                aPos = aTmp.TopLeft();
                aTmp.AdjustLeft(aSize.Width() );
                if ( pCli->eAlign != SfxChildAlignment::TOOLBOXLEFT )
                    aUpperClientArea.AdjustLeft(aSize.Width() );
                break;

            case SfxChildAlignment::FIRSTRIGHT:
            case SfxChildAlignment::RIGHT:
            case SfxChildAlignment::LASTRIGHT:
            case SfxChildAlignment::TOOLBOXRIGHT:
                aSize.setHeight( aTmp.GetHeight() );
                if ( pCli->pWin->GetType() == WindowType::SPLITWINDOW )
                    aSize = static_cast<SplitWindow *>(pCli->pWin.get())->CalcLayoutSizePixel( aSize );
                aBorder.Right() += aSize.Width();
                aPos = aTmp.TopRight();
                aPos.AdjustX( -(aSize.Width()-1) );
                aTmp.AdjustRight( -(aSize.Width()) );
                if ( pCli->eAlign != SfxChildAlignment::TOOLBOXRIGHT )
                    aUpperClientArea.AdjustRight( -(aSize.Width()) );
                break;

            default:
                pCli->aSize = pCli->pWin->GetSizePixel();
                pCli->bResize = false;
                continue;
        }

        pCli->pWin->SetPosSizePixel( aPos, aSize );
        pCli->bResize = false;
        pCli->aSize = aSize;
        if( bAllowHiding && !RequestTopToolSpacePixel_Impl( aBorder ) )
        {
            pCli->nVisible ^= SfxChildVisibility::FITS_IN;
            aBorder = aTemp;
        }
    }

    if ( aClientArea.GetWidth() >= aBorder.Left() + aBorder.Right() )
    {
        aClientArea.AdjustLeft(aBorder.Left() );
        aClientArea.AdjustRight( -(aBorder.Right()) );
    }
    else
    {
        aBorder.Left() = aClientArea.Left();
        aBorder.Right() = aClientArea.Right();
        aClientArea.SetRight( aTmp.Left() );
        aClientArea.SetLeft( aTmp.Left() );
    }

    if ( aClientArea.GetHeight() >= aBorder.Top() + aBorder.Bottom() )
    {
        aClientArea.AdjustTop(aBorder.Top() );
        aClientArea.AdjustBottom( -(aBorder.Bottom()) );
    }
    else
    {
        aBorder.Top() = aClientArea.Top();
        aBorder.Bottom() = aClientArea.Bottom();
        aClientArea.SetTop(aTmp.Top());
        aClientArea.SetBottom(aTmp.Top());
    }

    return IsDockingAllowed() ? aBorder : SvBorder();
}

bool SfxWorkWindow::PrepareClose_Impl()
{
    for (std::unique_ptr<SfxChildWin_Impl> &pCW : aChildWins)
    {
        SfxChildWindow *pChild = pCW->pWin;
        if ( pChild && !pChild->QueryClose() )
            return false;
    }

    return true;
}

SfxChild_Impl* SfxWorkWindow::RegisterChild_Impl( vcl::Window& rWindow,
                    SfxChildAlignment eAlign )
{
    DBG_ASSERT( aChildren.size() < 255, "too many children" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );
    DBG_ASSERT( !FindChild_Impl(rWindow), "child registered more than once" );


    if ( rWindow.GetParent() != pWorkWin )
        rWindow.SetParent( pWorkWin );

    auto pChild = std::make_unique<SfxChild_Impl>(rWindow, rWindow.GetSizePixel(),
                                    eAlign, rWindow.IsVisible());

    aChildren.push_back(std::move(pChild));
    bSorted = false;
    nChildren++;
    return aChildren.back().get();
}

SfxChild_Impl* SfxWorkWindow::RegisterChild_Impl(std::shared_ptr<SfxModelessDialogController>& rController,
                    SfxChildAlignment eAlign )
{
    DBG_ASSERT( aChildren.size() < 255, "too many children" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );

    auto pChild = std::make_unique<SfxChild_Impl>(rController, eAlign);

    aChildren.push_back(std::move(pChild));
    bSorted = false;
    nChildren++;
    return aChildren.back().get();
}

void SfxWorkWindow::ReleaseChild_Impl( vcl::Window& rWindow )
{

    SfxChild_Impl *pChild = nullptr;
    decltype(aChildren)::size_type nPos;
    for ( nPos = 0; nPos < aChildren.size(); ++nPos )
    {
        pChild = aChildren[nPos].get();
        if ( pChild && pChild->pWin == &rWindow )
        {
            bSorted = false;
            nChildren--;
            aChildren.erase(aChildren.begin() + nPos);
            return;
        }
    }
    OSL_FAIL( "releasing unregistered child" );
}

void SfxWorkWindow::ReleaseChild_Impl(SfxModelessDialogController& rController)
{

    SfxChild_Impl *pChild = nullptr;
    decltype(aChildren)::size_type nPos;
    for ( nPos = 0; nPos < aChildren.size(); ++nPos )
    {
        pChild = aChildren[nPos].get();
        if (pChild && pChild->xController.get() == &rController)
        {
            bSorted = false;
            nChildren--;
            aChildren.erase(aChildren.begin() + nPos);
            return;
        }
    }
    OSL_FAIL( "releasing unregistered child" );
}

SfxChild_Impl* SfxWorkWindow::FindChild_Impl( const vcl::Window& rWindow ) const
{

    sal_uInt16 nCount = aChildren.size();
    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
    {
        SfxChild_Impl *pChild = aChildren[nPos].get();
        if ( pChild && pChild->pWin == &rWindow )
            return pChild;
    }

    return nullptr;
}


void SfxWorkWindow::ShowChildren_Impl()
{

    bool bInvisible = ( !IsVisible_Impl() || ( !pWorkWin->IsReallyVisible() && !pWorkWin->IsReallyShown() ));

    for (std::unique_ptr<SfxChild_Impl>& pCli : aChildren)
    {
        if (!pCli)
            continue;
        SfxChildWin_Impl* pCW = nullptr;
        if (pCli->pWin || pCli->xController)
        {
            // We have to find the SfxChildWin_Impl to retrieve the
            // SFX_CHILDWIN flags that can influence visibility.
            for (std::unique_ptr<SfxChildWin_Impl>& pCWin : aChildWins)
            {
                SfxChild_Impl*    pChild  = pCWin->pCli;
                if ( pChild == pCli.get() )
                {
                    pCW = pCWin.get();
                    break;
                }
            }

            bool bVisible( !bInvisible );
            if ( pCW )
            {
                // Check flag SFX_CHILDWIN_NEVERHIDE that forces us to show
                // the child window even in situations where no child window is
                // visible.
                SfxChildWindowFlags nFlags = pCW->aInfo.nFlags;
                bVisible = !bInvisible || ( nFlags & SfxChildWindowFlags::NEVERHIDE );
            }

            if ( SfxChildVisibility::VISIBLE == (pCli->nVisible & SfxChildVisibility::VISIBLE) && bVisible )
            {
                ShowFlags nFlags = pCli->bSetFocus ? ShowFlags::NONE : ShowFlags::NoFocusChange | ShowFlags::NoActivate;
                if (pCli->xController)
                {
                    if (!pCli->xController->getDialog()->get_visible())
                    {
                        auto xController = pCli->xController;
                        weld::DialogController::runAsync(xController,
                            [=](sal_Int32 /*nResult*/){ xController->Close(); });
                    }
                }
                else
                    pCli->pWin->Show(true, nFlags);
                pCli->bSetFocus = false;
            }
            else
            {
                if (pCli->xController)
                {
                    if (pCli->xController->getDialog()->get_visible())
                        pCli->xController->response(RET_CLOSE);
                }
                else
                    pCli->pWin->Hide();
            }
        }
    }
}


void SfxWorkWindow::HideChildren_Impl()
{
    for ( sal_uInt16 nPos = aChildren.size(); nPos > 0; --nPos )
    {
        SfxChild_Impl *pChild = aChildren[nPos-1].get();
        if (!pChild)
            continue;
        if (pChild->xController)
            pChild->xController->response(RET_CLOSE);
        else if (pChild->pWin)
            pChild->pWin->Hide();
    }
}

void SfxWorkWindow::ResetObjectBars_Impl()
{
    for ( auto & n: aObjBarList )
        n.bDestroy = true;

    for ( auto & n: aChildWins )
        n->nId = 0;
}

void SfxWorkWindow::SetObjectBar_Impl(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId)
{
    DBG_ASSERT( nPos < SFX_OBJECTBAR_MAX, "object bar position overflow" );

    SfxObjectBar_Impl aObjBar;
    aObjBar.eId = eId;
    aObjBar.nMode = nFlags;

    for (SfxObjectBar_Impl & rBar : aObjBarList)
    {
        if ( rBar.eId == aObjBar.eId )
        {
            rBar = aObjBar;
            return;
        }
    }

    aObjBarList.push_back( aObjBar );
}

bool SfxWorkWindow::IsVisible_Impl( SfxVisibilityFlags nMode ) const
{
    switch( nUpdateMode )
    {
        case SfxVisibilityFlags::Standard:
            return true;
        case SfxVisibilityFlags::Invisible:
            return false;
        case SfxVisibilityFlags::Client:
        case SfxVisibilityFlags::Server:
            return bool(nMode & nUpdateMode);
        default:
            return (nMode & nOrigMode ) ||
                nOrigMode == SfxVisibilityFlags::Standard;
    }
}

void SfxWorkWindow::UpdateObjectBars_Impl()
{
    if ( pFrame->IsClosing_Impl() )
        return;

    UpdateObjectBars_Impl2();

    {
        ArrangeChildren_Impl( false );

        ShowChildren_Impl();
    }

    ShowChildren_Impl();
}

Reference< css::task::XStatusIndicator > SfxWorkWindow::GetStatusIndicator()
{
    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;
    Reference< css::task::XStatusIndicator > xStatusIndicator;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
        aValue >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            xLayoutManager->createElement( g_aProgressBarResName );
            xLayoutManager->showElement( g_aProgressBarResName );

            Reference< css::ui::XUIElement > xProgressBar =
                xLayoutManager->getElement( g_aProgressBarResName );
            if ( xProgressBar.is() )
            {
                xStatusIndicator.set( xProgressBar->getRealInterface(), UNO_QUERY );
            }
        }
    }

    return xStatusIndicator;
}


bool SfxWorkWindow::IsPluginMode( SfxObjectShell const * pObjShell )
{
    if ( pObjShell && pObjShell->GetMedium() )
    {
        const SfxBoolItem* pViewOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pObjShell->GetMedium()->GetItemSet(), SID_VIEWONLY, false);
        if ( pViewOnlyItem && pViewOnlyItem->GetValue() )
            return true;
    }

    return false;
}


css::uno::Reference< css::frame::XFrame > SfxWorkWindow::GetFrameInterface()
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    SfxDispatcher* pDispatcher( GetBindings().GetDispatcher() );
    if ( pDispatcher )
    {
        SfxViewFrame* pViewFrame = pDispatcher->GetFrame();
        if ( pViewFrame )
           xFrame = pViewFrame->GetFrame().GetFrameInterface();
    }

    return xFrame;
}


void SfxWorkWindow::UpdateObjectBars_Impl2()
{
    // Lock SplitWindows (which means suppressing the Resize-Reaction of the
    // DockingWindows)
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        VclPtr<SfxSplitWindow> const & p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
        aValue >>= xLayoutManager;
    }

    if ( !xLayoutManager.is() )
        return;

    bool       bPluginMode( false );
    SfxDispatcher* pDispatcher( GetBindings().GetDispatcher() );

    if ( pDispatcher )
    {
        SfxViewFrame* pViewFrame = pDispatcher->GetFrame();
        if ( pViewFrame )
           bPluginMode = IsPluginMode( pViewFrame->GetObjectShell() );
    }

    // Iterate over all Toolboxes
    xLayoutManager->lock();
    for ( auto const & n: aObjBarList )
    {
        ToolbarId eId = n.eId;
        bool    bDestroy = n.bDestroy;

        // Determine the valid mode for the ToolBox
        SfxVisibilityFlags nTbxMode = n.nMode;
        bool bFullScreenTbx( nTbxMode & SfxVisibilityFlags::FullScreen );
        nTbxMode &= ~SfxVisibilityFlags::FullScreen;
        nTbxMode &= ~SfxVisibilityFlags::Viewer;

        // Is a ToolBox required in this context ?
        bool bModesMatching = (nUpdateMode != SfxVisibilityFlags::Invisible) && ((nTbxMode & nUpdateMode) == nUpdateMode);
        if ( bDestroy || sfx2::SfxNotebookBar::IsActive())
        {
            OUString aTbxId = g_aTbxTypeName + GetResourceURLFromToolbarId(eId);
            xLayoutManager->destroyElement( aTbxId );
        }
        else if ( eId != ToolbarId::None && ( ( bModesMatching && !bIsFullScreen ) ||
                                ( bIsFullScreen && bFullScreenTbx ) ) )
        {
            OUString aTbxId = g_aTbxTypeName + GetResourceURLFromToolbarId(eId);
            if ( !IsDockingAllowed() && !xLayoutManager->isElementFloating( aTbxId ))
                xLayoutManager->destroyElement( aTbxId );
            else
            {
                xLayoutManager->requestElement( aTbxId );
                if ( bPluginMode )
                    xLayoutManager->lockWindow( aTbxId );
            }
        }
        else if ( eId != ToolbarId::None )
        {
            // Delete the Toolbox at this Position if possible
            OUString aTbxId = g_aTbxTypeName + GetResourceURLFromToolbarId(eId);
            xLayoutManager->destroyElement( aTbxId );
        }
    }

    UpdateStatusBar_Impl();

    // unlocking automatically forces Layout
    xLayoutManager->unlock();

    UpdateChildWindows_Impl();

    // Unlock the SplitWindows again
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        VclPtr<SfxSplitWindow> const & p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock(false);
    }
}

void SfxWorkWindow::UpdateChildWindows_Impl()
{
    // tdf#100870, tdf#101320: don't use range-based for loop when
    // container is modified
    for ( size_t n=0; n<aChildWins.size(); n++ )
    {
        // any current or in the context available Childwindows
        SfxChildWin_Impl *pCW = aChildWins[n].get();
        SfxChildWindow *pChildWin = pCW->pWin;
        bool bCreate = false;
        if ( pCW->nId && (pCW->aInfo.nFlags & SfxChildWindowFlags::ALWAYSAVAILABLE || IsVisible_Impl( pCW->nVisibility ) ) )
        {
            // In the context is an appropriate ChildWindow allowed;
            // it is also turned on?
            if ( pChildWin == nullptr && pCW->bCreate )
            {
                // Internal docking is only used for embedding into another
                // container. We force the floating state of all floatable
                // child windows.
                if ( !bInternalDockingAllowed )
                {
                    // Special case for all non-floatable child windows. We have
                    // to prevent the creation here!
                    bCreate = !( pCW->aInfo.nFlags & SfxChildWindowFlags::FORCEDOCK );
                }
                else if ( !IsDockingAllowed() || bIsFullScreen ) // || !bInternalDocking )
                {
                    // In Presentation mode or FullScreen only FloatingWindows
                    SfxChildAlignment eAlign;
                    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
                        bCreate = ( eAlign == SfxChildAlignment::NOALIGNMENT );
                }
                else
                    bCreate = true;

                // Currently, no window here, but it is enabled; windows
                // Create window and if possible theContext
                if ( bCreate )
                    CreateChildWin_Impl( pCW, false );

                if ( !bAllChildrenVisible && pCW->pCli )
                    pCW->pCli->nVisible &= ~SfxChildVisibility::ACTIVE;
            }
            else if ( pChildWin )
            {
                // Window already exists, it should also be visible?
                if ( ( !bIsFullScreen || pChildWin->GetAlignment() == SfxChildAlignment::NOALIGNMENT ) && bAllChildrenVisible )
                {
                    // Update Mode is compatible; definitely enable it
                    bCreate = true;
                    if ( pCW->pCli )
                    {
                        // The window is a direct Child
                        if ((IsDockingAllowed() && bInternalDockingAllowed)
                            || pCW->pCli->eAlign == SfxChildAlignment::NOALIGNMENT)
                            pCW->pCli->nVisible |= SfxChildVisibility::NOT_HIDDEN;
                    }
                    else
                    {
                        if ( pCW->bCreate && IsDockingAllowed() && bInternalDockingAllowed )
                            // The window ia within a SplitWindow
                            static_cast<SfxDockingWindow*>(pChildWin->GetWindow())->Reappear_Impl();
                    }

                    if ( pCW->nInterfaceId != pChildWin->GetContextId() )
                        pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );
                }
            }
        }

        if ( pChildWin && !bCreate )
        {
            if ( !pChildWin->QueryClose() || pChildWin->IsHideNotDelete() || Application::IsUICaptured() )
            {
                if ( pCW->pCli )
                {
                    if ( pCW->pCli->nVisible & SfxChildVisibility::NOT_HIDDEN )
                        pCW->pCli->nVisible ^= SfxChildVisibility::NOT_HIDDEN;
                }
                else
                    static_cast<SfxDockingWindow*>(pChildWin->GetWindow())->Disappear_Impl();
            }
            else
                RemoveChildWin_Impl( pCW );
        }
    }
}

void SfxWorkWindow::CreateChildWin_Impl( SfxChildWin_Impl *pCW, bool bSetFocus )
{
    pCW->aInfo.bVisible = true;

    SfxChildWindow *pChildWin = SfxChildWindow::CreateChildWindow( pCW->nId, pWorkWin, &GetBindings(), pCW->aInfo).release();
    if (!pChildWin)
        return;

    if ( bSetFocus )
        bSetFocus = pChildWin->WantsFocus();
    pChildWin->SetWorkWindow_Impl( this );

    // At least the extra string is changed during the evaluation,
    // also get it anewed
    SfxChildWinInfo aInfo = pChildWin->GetInfo();
    pCW->aInfo.aExtraString = aInfo.aExtraString;
    pCW->aInfo.bVisible = aInfo.bVisible;
    pCW->aInfo.nFlags |= aInfo.nFlags;

    // The creation was successful
    GetBindings().Invalidate(pCW->nId);

    sal_uInt16 nPos = pChildWin->GetPosition();
    if (nPos != CHILDWIN_NOPOS)
    {
        DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
        if ( aChildren[TbxMatch(nPos)] )// &&
        {
            // ChildWindow replaces ObjectBar
            aChildren[TbxMatch(nPos)]->nVisible ^= SfxChildVisibility::NOT_HIDDEN;
        }
    }

    // make childwin keyboard accessible
    pWorkWin->GetSystemWindow()->GetTaskPaneList()->AddWindow( pChildWin->GetWindow() );

    pCW->pWin = pChildWin;

    if ( pChildWin->GetAlignment() == SfxChildAlignment::NOALIGNMENT || pChildWin->GetWindow()->GetParent() == pWorkWin)
    {
        // The window is not docked or docked outside of one split windows
        // and must therefore be registered explicitly as a Child
        if (pChildWin->GetController())
            pCW->pCli = RegisterChild_Impl(pChildWin->GetController(), pChildWin->GetAlignment());
        else
            pCW->pCli = RegisterChild_Impl(*(pChildWin->GetWindow()), pChildWin->GetAlignment());
        pCW->pCli->nVisible = SfxChildVisibility::VISIBLE;
        if ( pChildWin->GetAlignment() != SfxChildAlignment::NOALIGNMENT && bIsFullScreen )
            pCW->pCli->nVisible ^= SfxChildVisibility::ACTIVE;
        pCW->pCli->bSetFocus = bSetFocus;
    }
    else
    {
        // A docked window which parent is not a WorkingWindow, must lie
        // in a SplitWindow and thus not be explicitly registered.
        // This happens already in the initialization of SfxDockingWindows!
    }

    if ( pCW->nInterfaceId != pChildWin->GetContextId() )
        pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );

    // Save the information in the INI file
    SaveStatus_Impl(pChildWin, pCW->aInfo);
}

void SfxWorkWindow::RemoveChildWin_Impl( SfxChildWin_Impl *pCW )
{
    sal_uInt16 nId = pCW->nSaveId;
    SfxChildWindow *pChildWin = pCW->pWin;

    // Save the information in the INI file
    SfxChildWindowFlags nFlags = pCW->aInfo.nFlags;
    pCW->aInfo = pChildWin->GetInfo();
    pCW->aInfo.nFlags |= nFlags;
    SaveStatus_Impl(pChildWin, pCW->aInfo);

    pChildWin->Hide();

    if ( pCW->pCli )
    {
        // Child window is a direct child window and must therefore unregister
        // itself from the  WorkWindow
        pCW->pCli = nullptr;
        if (pChildWin->GetController())
            ReleaseChild_Impl(*pChildWin->GetController());
        else
            ReleaseChild_Impl(*pChildWin->GetWindow());
    }
    else
    {
        // ChildWindow is within a SplitWindow and unregister itself in
        // the destructor.
    }

    pWorkWin->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( pChildWin->GetWindow() );
    pCW->pWin = nullptr;
    pChildWin->Destroy();

    GetBindings().Invalidate( nId );
}

void SfxWorkWindow::ResetStatusBar_Impl()
{
    aStatBar.eId = StatusBarId::None;
}

void SfxWorkWindow::SetStatusBar_Impl(StatusBarId eId)
{
    if (eId != StatusBarId::None && bShowStatusBar && IsVisible_Impl())
        aStatBar.eId = eId;
}

void SfxWorkWindow::UpdateStatusBar_Impl()
{
    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    Any aValue = xPropSet->getPropertyValue( g_aLayoutManagerPropName );
    aValue >>= xLayoutManager;

    // No status bar, if no ID is required or when in FullScreenView or
    // if disabled
    if (aStatBar.eId != StatusBarId::None && IsDockingAllowed() && bInternalDockingAllowed && bShowStatusBar &&
        !bIsFullScreen)
    {
        // Id has changed, thus create a suitable Statusbarmanager, this takes
        // over the  current status bar;
        if ( xLayoutManager.is() )
            xLayoutManager->requestElement( g_aStatusBarResName );
    }
    else
    {
        // Destroy the current StatusBar
        // The Manager only creates the Status bar, does not destroy it.
        if ( xLayoutManager.is() )
            xLayoutManager->destroyElement( g_aStatusBarResName );
    }
}

void SfxWorkWindow::MakeVisible_Impl( bool bVis )
{
    if ( bVis )
        nOrigMode = SfxVisibilityFlags::Standard;
    else
        nOrigMode = SfxVisibilityFlags::Invisible;

    if ( nOrigMode != nUpdateMode)
        nUpdateMode = nOrigMode;
}

bool SfxWorkWindow::IsVisible_Impl()
{
    return nOrigMode != SfxVisibilityFlags::Invisible;
}


void SfxWorkWindow::HidePopups_Impl(bool bHide, sal_uInt16 nId )
{
    if (comphelper::LibreOfficeKit::isActive() && bHide)
        return;

    for (std::unique_ptr<SfxChildWin_Impl>& i : aChildWins)
    {
        SfxChildWindow *pCW = i->pWin;
        if (pCW && pCW->GetAlignment() == SfxChildAlignment::NOALIGNMENT && pCW->GetType() != nId)
        {
            vcl::Window *pWin = pCW->GetWindow();
            SfxChild_Impl *pChild = FindChild_Impl(*pWin);
            if (bHide)
            {
                pChild->nVisible &= ~SfxChildVisibility::ACTIVE;
                pCW->Hide();
            }
            else if ( !comphelper::LibreOfficeKit::isActive() ||
                      SfxChildVisibility::ACTIVE != (pChild->nVisible & SfxChildVisibility::ACTIVE) )
            {
                pChild->nVisible |= SfxChildVisibility::ACTIVE;
                if ( SfxChildVisibility::VISIBLE == (pChild->nVisible & SfxChildVisibility::VISIBLE) )
                    pCW->Show( ShowFlags::NoFocusChange | ShowFlags::NoActivate );
            }
        }
    }
}


void SfxWorkWindow::ConfigChild_Impl(SfxChildIdentifier eChild,
            SfxDockingConfig eConfig, sal_uInt16 nId)
{
    SfxDockingWindow* pDockWin=nullptr;
    sal_uInt16 nPos = USHRT_MAX;
    vcl::Window *pWin=nullptr;
    SfxChildWin_Impl *pCW = nullptr;

    // configure direct childwindow
    for (std::unique_ptr<SfxChildWin_Impl>& i : aChildWins)
    {
        pCW = i.get();
        SfxChildWindow *pChild = pCW->pWin;
        if ( pChild && (pChild->GetType() == nId ))
        {
            if (SfxDockingWindow* pSfxDockingWindow = dynamic_cast<SfxDockingWindow*>(pChild->GetWindow()))
            {
                // it's a DockingWindow
                pDockWin = pSfxDockingWindow;
            }
            else
            {
                // FloatingWindow or ModelessDialog
                pWin = pChild->GetWindow();
            }
            break;
        }
    }

    if ( pDockWin )
    {
        if ( eChild == SfxChildIdentifier::DOCKINGWINDOW || pDockWin->GetAlignment() == SfxChildAlignment::NOALIGNMENT )
        {
            if ( eChild == SfxChildIdentifier::SPLITWINDOW && eConfig == SfxDockingConfig::TOGGLEFLOATMODE)
            {
                // DockingWindow was dragged out of a SplitWindow
                pCW->pCli = RegisterChild_Impl(*pDockWin, pDockWin->GetAlignment());
                pCW->pCli->nVisible = SfxChildVisibility::VISIBLE;
            }

            pWin = pDockWin;
        }
        else
        {
            SfxSplitWindow *pSplitWin = GetSplitWindow_Impl(pDockWin->GetAlignment());

            // configure DockingWindow inside a SplitWindow
            if ( eConfig == SfxDockingConfig::TOGGLEFLOATMODE)
            {
                // DockingWindow was dragged into a SplitWindow
                pCW->pCli = nullptr;
                ReleaseChild_Impl(*pDockWin);
            }

            pWin = pSplitWin->GetSplitWindow();
            if ( pSplitWin->GetWindowCount() == 1 )
                static_cast<SplitWindow*>(pWin)->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
        }
    }

    DBG_ASSERT( pCW, "Unknown window!" );

    if ( !bSorted )
        // windows may have been registered and released without an update until now
        Sort_Impl();

    decltype(aSortedList)::size_type n;
    for ( n=0; n<aSortedList.size(); ++n )
    {
        SfxChild_Impl *pChild = aChildren[aSortedList[n]].get();
        if ( pChild && pChild->pWin == pWin )
            break;
    }

    if ( n < aSortedList.size() )
        // sometimes called while toggeling float mode
        nPos = aSortedList[n];

    switch ( eConfig )
    {
        case SfxDockingConfig::SETDOCKINGRECTS :
        {
            if (nPos == USHRT_MAX || !pDockWin)
                return;

            tools::Rectangle aOuterRect( GetTopRect_Impl() );
            aOuterRect.SetPos( pWorkWin->OutputToScreenPixel( aOuterRect.TopLeft() ));
            tools::Rectangle aInnerRect( aOuterRect );

            // The current affected window is included in the calculation of
            // the inner rectangle!
            for (sal_uInt16 i : aSortedList)
            {
                SfxChild_Impl* pCli = aChildren[i].get();

                if ( pCli && pCli->nVisible == SfxChildVisibility::VISIBLE && pCli->pWin )
                {
                    switch ( pCli->eAlign )
                    {
                        case SfxChildAlignment::TOP:
                            // Object-Toolboxes come always last
                                aInnerRect.AdjustTop(pCli->aSize.Height() );
                            break;

                        case SfxChildAlignment::HIGHESTTOP:
                            // Always performed first
                            aInnerRect.AdjustTop(pCli->aSize.Height() );
                            break;

                        case SfxChildAlignment::LOWESTTOP:
                            // Is only counted if it is the current window
                            if ( i == nPos )
                                aInnerRect.AdjustTop(pCli->aSize.Height() );
                            break;

                        case SfxChildAlignment::BOTTOM:
                            // Object-Toolboxes come always last
                                aInnerRect.AdjustBottom( -(pCli->aSize.Height()) );
                            break;

                        case SfxChildAlignment::LOWESTBOTTOM:
                            // Always performed first
                            aInnerRect.AdjustBottom( -(pCli->aSize.Height()) );
                            break;

                        case SfxChildAlignment::HIGHESTBOTTOM:
                            // Is only counted if it is the current window
                            if ( i == nPos )
                                aInnerRect.AdjustBottom( -(pCli->aSize.Height()) );
                            break;

                        case SfxChildAlignment::LEFT:
                            // Toolboxes come always last
                                aInnerRect.AdjustLeft(pCli->aSize.Width() );
                            break;

                        case SfxChildAlignment::FIRSTLEFT:
                            // Always performed first
                            aInnerRect.AdjustLeft(pCli->aSize.Width() );
                            break;

                        case SfxChildAlignment::LASTLEFT:
                            // Is only counted if it is the current window
                            if (i == nPos)
                                aInnerRect.AdjustLeft(pCli->aSize.Width() );
                            break;

                        case SfxChildAlignment::RIGHT:
                            // Toolboxes come always last
                                aInnerRect.AdjustRight( -(pCli->aSize.Width()) );
                            break;

                        case SfxChildAlignment::FIRSTRIGHT:
                            // Is only counted if it is the current window
                            if (i == nPos)
                                aInnerRect.AdjustRight( -(pCli->aSize.Width()) );
                            break;

                        case SfxChildAlignment::LASTRIGHT:
                            // Always performed first
                            aInnerRect.AdjustRight( -(pCli->aSize.Width()) );
                            break;

                        default:
                            break;
                    }
                }
            }

            pDockWin->SetDockingRects(aOuterRect, aInnerRect);
            break;
        }

        case SfxDockingConfig::ALIGNDOCKINGWINDOW :
        case SfxDockingConfig::TOGGLEFLOATMODE:
        {
            if ( nPos == USHRT_MAX && !pCW )
                return;

            SfxChildAlignment eAlign = SfxChildAlignment::NOALIGNMENT;
            SfxChild_Impl *pCli = ( nPos != USHRT_MAX ) ? aChildren[nPos].get() : nullptr;
            if ( pCli && pDockWin )
            {
                eAlign = pDockWin->GetAlignment();
                if ( eChild == SfxChildIdentifier::DOCKINGWINDOW || eAlign == SfxChildAlignment::NOALIGNMENT)
                {
                    // configuration inside the SplitWindow, no change for the SplitWindows' configuration
                    pCli->bResize = true;
                    pCli->aSize = pDockWin->GetSizePixel();
                }
            }

            if ( pCli )
            {
                if( pCli->eAlign != eAlign )
                {
                    bSorted = false;
                    pCli->eAlign = eAlign;
                }

                ArrangeChildren_Impl();
                ShowChildren_Impl();
            }

            if ( pCW && pCW->pWin )
            {
                // store changed configuration
                SfxChildWindowFlags nFlags = pCW->aInfo.nFlags;
                pCW->aInfo = pCW->pWin->GetInfo();
                pCW->aInfo.nFlags |= nFlags;
                SaveStatus_Impl( pCW->pWin, pCW->aInfo);
            }

            break;
        }
    }
}


void SfxWorkWindow::SetChildWindowVisible_Impl( sal_uInt32 lId, bool bEnabled, SfxVisibilityFlags nMode )
{
    sal_uInt16 nInter = static_cast<sal_uInt16>( lId >> 16 );
    sal_uInt16 nId = static_cast<sal_uInt16>( lId & 0xFFFF );

    SfxChildWin_Impl *pCW=nullptr;

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n].get();
                break;
            }
    }

    if ( !pCW )
    {
        // If new, then initialize, add this here depending on the flag or
        // the Parent
        pCW = new SfxChildWin_Impl( lId );
        pCW->nId = nId;
        InitializeChild_Impl( pCW );
        aChildWins.push_back( std::unique_ptr<SfxChildWin_Impl>(pCW) );
    }

    pCW->nId = nId;
    if ( nInter )
        pCW->nInterfaceId = nInter;
    pCW->nVisibility = nMode;
    pCW->bEnable = bEnabled;
}


// The on/off status of a ChildWindow is switched

void SfxWorkWindow::ToggleChildWindow_Impl(sal_uInt16 nId, bool bSetFocus)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nId == nId)
            break;

    if ( n<nCount )
    {
        // The Window is already known
        SfxChildWin_Impl *pCW = aChildWins[n].get();
        SfxChildWindow *pChild = pCW->pWin;

        bool bCreationAllowed( true );
        if ( !bInternalDockingAllowed )
        {
            // Special case for all non-floatable child windows. We have
            // to prevent the creation here!
            bCreationAllowed = !( pCW->aInfo.nFlags & SfxChildWindowFlags::FORCEDOCK );
        }

        if ( bCreationAllowed )
        {
            if ( pCW->bCreate )
            {
                if ( pChild )
                {
                    if ( pChild->QueryClose() )
                    {
                        pCW->bCreate = false;
                        // The Window should be switched off
                        pChild->SetVisible_Impl( false );
                        RemoveChildWin_Impl( pCW );
                    }
                }
                else
                {
                    // no actual Window exists, yet => just remember the "switched off" state
                    pCW->bCreate = false;
                }
            }
            else
            {
                pCW->bCreate = true;
                if ( pChild )
                {
                    ShowChildWindow_Impl( nId, true, bSetFocus );
                }
                else
                {
                    // create actual Window
                    CreateChildWin_Impl( pCW, bSetFocus );
                    if ( !pCW->pWin )
                        // no success
                        pCW->bCreate = false;
                }
            }
        }

        ArrangeChildren_Impl();
        ShowChildren_Impl();

        if ( pCW->bCreate && bCreationAllowed )
        {
            if ( !pCW->pCli )
            {
                SfxDockingWindow *pDock =
                    static_cast<SfxDockingWindow*>( pCW->pWin->GetWindow() );
                if ( pDock->IsAutoHide_Impl() )
                    pDock->AutoShow_Impl();
            }
        }

        return;
    }

#ifdef DBG_UTIL
    nCount = aChildWins.size();
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
            break;

    if ( n < nCount )
    {
        OSL_FAIL("The ChildWindow is not in context!");
    }
    else
    {
        OSL_FAIL("The ChildWindow is not registered!");
    }
#endif
}


bool SfxWorkWindow::HasChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
            break;

    if (n<nCount)
    {
        SfxChildWin_Impl *pCW = aChildWins[n].get();
        SfxChildWindow *pChild = pCW->pWin;
        return ( pChild && pCW->bCreate );
    }

    return false;
}

bool SfxWorkWindow::IsFloating( sal_uInt16 nId )
{
    SfxChildWin_Impl *pCW=nullptr;

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n].get();
                break;
            }
    }

    if ( !pCW )
    {
        // If new, then initialize, add this here depending on the flag or
        // the Parent
        pCW = new SfxChildWin_Impl( nId );
        pCW->bEnable = false;
        pCW->nId = 0;
        pCW->nVisibility = SfxVisibilityFlags::Invisible;
        InitializeChild_Impl( pCW );
        aChildWins.push_back( std::unique_ptr<SfxChildWin_Impl>(pCW) );
    }

    SfxChildAlignment eAlign;
    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
        return( eAlign == SfxChildAlignment::NOALIGNMENT );
    else
        return true;
}


bool SfxWorkWindow::KnowsChildWindow_Impl(sal_uInt16 nId)
{
    SfxChildWin_Impl *pCW=nullptr;
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
    {
        pCW = aChildWins[n].get();
        if ( pCW->nSaveId == nId)
             break;
    }

    if (n<nCount)
    {
        if ( !(pCW->aInfo.nFlags & SfxChildWindowFlags::ALWAYSAVAILABLE) && !IsVisible_Impl(  pCW->nVisibility ) )
            return false;
        return pCW->bEnable;
    }
    else
        return false;
}


void SfxWorkWindow::SetChildWindow_Impl(sal_uInt16 nId, bool bOn, bool bSetFocus)
{
    SfxChildWin_Impl *pCW=nullptr;
    SfxWorkWindow *pWork = nullptr;

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n].get();
                pWork = this;
                break;
            }
    }

    if ( !pCW )
    {
        // If new, then initialize, add this here depending on the flag or
        // the Parent
        pCW = new SfxChildWin_Impl( nId );
        InitializeChild_Impl( pCW );
        if ( !pWork || pCW->aInfo.nFlags & SfxChildWindowFlags::TASK )
            pWork = this;
        pWork->aChildWins.push_back( std::unique_ptr<SfxChildWin_Impl>(pCW) );
    }

    if ( pCW->bCreate != bOn )
        pWork->ToggleChildWindow_Impl(nId,bSetFocus);
}


void SfxWorkWindow::ShowChildWindow_Impl(sal_uInt16 nId, bool bVisible, bool bSetFocus)
{
    sal_uInt16 nCount = aChildWins.size();
    SfxChildWin_Impl* pCW=nullptr;
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
    {
        pCW = aChildWins[n].get();
        if (pCW->nId == nId)
            break;
    }

    if ( n<nCount )
    {
        SfxChildWindow *pChildWin = pCW->pWin;
        if ( pChildWin )
        {
            if ( bVisible )
            {
                if ( pCW->pCli )
                {
                    pCW->pCli->bSetFocus = bSetFocus;
                    pCW->pCli->nVisible = SfxChildVisibility::VISIBLE;
                    pChildWin->Show( bSetFocus && pChildWin->WantsFocus() ? ShowFlags::NONE : ShowFlags::NoFocusChange | ShowFlags::NoActivate );
                }
                else
                    static_cast<SfxDockingWindow*>(pChildWin->GetWindow())->Reappear_Impl();

            }
            else
            {
                if ( pCW->pCli )
                {
                    pCW->pCli->nVisible = SfxChildVisibility::VISIBLE ^ SfxChildVisibility::NOT_HIDDEN;
                    pCW->pWin->Hide();
                }
                else
                    static_cast<SfxDockingWindow*>(pChildWin->GetWindow())->Disappear_Impl();

            }

            ArrangeChildren_Impl();
            ShowChildren_Impl();
        }
        else if ( bVisible )
        {
            SetChildWindow_Impl( nId, true, bSetFocus );
            pChildWin = pCW->pWin;
        }

        if ( pChildWin )
        {
            pChildWin->SetVisible_Impl( bVisible );
            SfxChildWindowFlags nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChildWin->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            if ( !pCW->bCreate )
                SaveStatus_Impl( pChildWin, pCW->aInfo );
        }

        return;
    }

#ifdef DBG_UTIL
    nCount = aChildWins.size();
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
            break;

    if ( n<nCount )
    {
        OSL_FAIL("The ChildWindow is not in context!");
    }
    else
    {
        OSL_FAIL("The ChildWindow is not registered");
    }
#endif
}


SfxChildWindow* SfxWorkWindow::GetChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
             break;

    if (n<nCount)
        return aChildWins[n]->pWin;
    return nullptr;
}


void SfxWorkWindow::ResetChildWindows_Impl()
{
    for (std::unique_ptr<SfxChildWin_Impl>& pChildWin : aChildWins)
    {
        pChildWin->nId = 0;
        pChildWin->bEnable = false;
    }
}

// returns the size of the area (client area) of the
// parent windows, in which the ChildWindow can be fitted.

tools::Rectangle SfxWorkWindow::GetTopRect_Impl()
{
    return pMasterFrame->GetTopOuterRectPixel_Impl();
}


// Virtual method to find out if there is room for a ChildWindow in the
// client area of the parent.

bool SfxWorkWindow::RequestTopToolSpacePixel_Impl( SvBorder aBorder )
{
    return !(!IsDockingAllowed() ||
            aClientArea.GetWidth() < aBorder.Left() + aBorder.Right() ||
            aClientArea.GetHeight() < aBorder.Top() + aBorder.Bottom());
}

void SfxWorkWindow::SaveStatus_Impl(SfxChildWindow *pChild, const SfxChildWinInfo &rInfo)
{
    // The Status of the Presentation mode is not saved
    if ( IsDockingAllowed() && bInternalDockingAllowed )
        pChild->SaveStatus(rInfo);
}

void SfxWorkWindow::InitializeChild_Impl(SfxChildWin_Impl *pCW)
{
    SfxDispatcher *pDisp = pBindings->GetDispatcher_Impl();
    SfxViewFrame *pViewFrame = pDisp ? pDisp->GetFrame() :nullptr;
    SfxModule *pMod = pViewFrame ? SfxModule::GetActiveModule(pViewFrame) :nullptr;

    OUString sModule;
    if (pViewFrame)
    {
        try
        {
            uno::Reference< frame::XModuleManager2 > xModuleManager(
                frame::ModuleManager::create(::comphelper::getProcessComponentContext()));
            sModule = xModuleManager->identify(pViewFrame->GetFrame().GetFrameInterface());
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::ClassifyFactoryByServiceName(sModule);
            sModule = SvtModuleOptions::GetFactoryShortName(eFac);
        }
        catch (...)
        {
        }
    }

    SfxChildWinFactory* pFact=nullptr;
    SfxApplication *pApp = SfxGetpApp();
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = &rFactories[nFactory];
            if ( pFact->nId == pCW->nSaveId )
            {
                pCW->aInfo   = pFact->aInfo;
                pCW->aInfo.aModule = sModule;
                SfxChildWindow::InitializeChildWinFactory_Impl(
                                            pCW->nSaveId, pCW->aInfo);
                pCW->bCreate = pCW->aInfo.bVisible;
                SfxChildWindowFlags nFlags = pFact->aInfo.nFlags;
                if ( nFlags & SfxChildWindowFlags::TASK )
                    pCW->aInfo.nFlags |= SfxChildWindowFlags::TASK;
                if ( nFlags & SfxChildWindowFlags::CANTGETFOCUS )
                    pCW->aInfo.nFlags |= SfxChildWindowFlags::CANTGETFOCUS;
                if ( nFlags & SfxChildWindowFlags::FORCEDOCK )
                    pCW->aInfo.nFlags |= SfxChildWindowFlags::FORCEDOCK;
                pFact->aInfo = pCW->aInfo;
                return;
            }
        }
    }

    if ( !pMod )
        return;

    SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
    if ( !pFactories )
        return;

    SfxChildWinFactArr_Impl &rFactories = *pFactories;
    for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
    {
        pFact = &rFactories[nFactory];
        if ( pFact->nId == pCW->nSaveId )
        {
            pCW->aInfo   = pFact->aInfo;
            pCW->aInfo.aModule = sModule;
            SfxChildWindow::InitializeChildWinFactory_Impl(
                                        pCW->nSaveId, pCW->aInfo);
            pCW->bCreate = pCW->aInfo.bVisible;
            SfxChildWindowFlags nFlags = pFact->aInfo.nFlags;
            if ( nFlags & SfxChildWindowFlags::TASK )
                pCW->aInfo.nFlags |= SfxChildWindowFlags::TASK;
            if ( nFlags & SfxChildWindowFlags::CANTGETFOCUS )
                pCW->aInfo.nFlags |= SfxChildWindowFlags::CANTGETFOCUS;
            if ( nFlags & SfxChildWindowFlags::FORCEDOCK )
                pCW->aInfo.nFlags |= SfxChildWindowFlags::FORCEDOCK;
            if ( nFlags & SfxChildWindowFlags::ALWAYSAVAILABLE )
                pCW->aInfo.nFlags |= SfxChildWindowFlags::ALWAYSAVAILABLE;
            pFact->aInfo = pCW->aInfo;
            return;
        }
    }
}

SfxSplitWindow* SfxWorkWindow::GetSplitWindow_Impl( SfxChildAlignment eAlign )
{
    switch ( eAlign )
    {
        case SfxChildAlignment::TOP:
            return pSplit[2];

        case SfxChildAlignment::BOTTOM:
            return pSplit[3];

        case SfxChildAlignment::LEFT:
            return pSplit[0];

        case SfxChildAlignment::RIGHT:
            return pSplit[1];

        default:
            return nullptr;
    }
}

void SfxWorkWindow::MakeChildrenVisible_Impl( bool bVis )
{
    bAllChildrenVisible = bVis;
    if ( bVis )
    {
        if ( !bSorted )
            Sort_Impl();
        for (sal_uInt16 n : aSortedList)
        {
            SfxChild_Impl* pCli = aChildren[n].get();
            if ( (pCli->eAlign == SfxChildAlignment::NOALIGNMENT) || (IsDockingAllowed() && bInternalDockingAllowed) )
                pCli->nVisible |= SfxChildVisibility::ACTIVE;
        }
    }
    else
    {
        if ( !bSorted )
            Sort_Impl();
        for (sal_uInt16 n : aSortedList)
        {
            SfxChild_Impl* pCli = aChildren[n].get();
            pCli->nVisible &= ~SfxChildVisibility::ACTIVE;
        }
    }
}

bool SfxWorkWindow::IsAutoHideMode( const SfxSplitWindow *pSplitWin )
{
    for (VclPtr<SfxSplitWindow> & pWin : pSplit)
    {
        if ( pWin.get() != pSplitWin && pWin->IsAutoHide( true ) )
            return true;
    }
    return false;
}


void SfxWorkWindow::EndAutoShow_Impl( Point aPos )
{
    for (VclPtr<SfxSplitWindow> & p : pSplit)
    {
        if ( p && p->IsAutoHide(false) )
        {
            Point aLocalPos = p->ScreenToOutputPixel( aPos );
            tools::Rectangle aRect( Point(), p->GetSizePixel() );
            if ( !aRect.IsInside( aLocalPos ) )
                p->FadeOut();
        }
    }
}

void SfxWorkWindow::ArrangeAutoHideWindows( SfxSplitWindow *pActSplitWin )
{
    if ( m_nLock )
        return;

    tools::Rectangle aArea( aUpperClientArea );
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Either dummy window or window in the auto-show-mode are processed
        // (not pinned, FadeIn).
        // Only the abandoned window may be invisible, because perhaps its
        // size is just being calculated before it is displayed.
        VclPtr<SfxSplitWindow> const & pSplitWin = pSplit[n];
        bool bDummyWindow = !pSplitWin->IsFadeIn();
        vcl::Window *pDummy = pSplitWin->GetSplitWindow();
        vcl::Window *pWin = bDummyWindow ? pDummy : pSplitWin;
        if ( (pSplitWin->IsPinned() && !bDummyWindow) || (!pWin->IsVisible() && pActSplitWin != pSplitWin) )
            continue;

        // Width and position of the dummy window as a starting point
        Size aSize = pDummy->GetSizePixel();
        Point aPos = pDummy->GetPosPixel();

        switch ( n )
        {
            case 0 :
            {
                // Left SplitWindow
                // Get the width of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.setWidth( pSplitWin->GetSizePixel().Width() );

                // If a Window is visible to the left, then the free region
                // starts to the right from it, for example at the Client area
                long nLeft = aPos.X() + aSize.Width();
                if ( nLeft > aArea.Left() )
                    aArea.SetLeft( nLeft );
                break;
            }
            case 1 :
            {
                // Right SplitWindow
                // Position to correct the difference of the widths
                aPos.AdjustX(aSize.Width() );

                // Get the width of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.setWidth( pSplitWin->GetSizePixel().Width() );

                aPos.AdjustX( -(aSize.Width()) );

                // If already a window is opened at the left side, then the
                // right is not allowed to overlap this one.
                if ( aPos.X() < aArea.Left() )
                {
                    aPos.setX( aArea.Left() );
                    aSize.setWidth( aArea.GetWidth() );
                }

                // If a Window is visible to the right, then the free region
                // starts to the left from it, for example at the Client area
                long nRight = aPos.X();
                if ( nRight < aArea.Right() )
                    aArea.SetRight( nRight );
                break;
            }
            case 2 :
            {
                // Top SplitWindow
                // Get the height of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.setHeight( pSplitWin->GetSizePixel().Height() );


                // Adjust width with regard to if a Window is already open
                // to the left or right
                aPos.setX( aArea.Left() );
                aSize.setWidth( aArea.GetWidth() );

                // If a Window is visible at the top, then the free region
                // starts beneath it, for example at the Client area
                long nTop = aPos.Y() + aSize.Height();
                if ( nTop > aArea.Top() )
                    aArea.SetTop( nTop );
                break;
            }
            case 3 :
            {
                // The bottom SplitWindow
                // Position to correct the difference of the heights
                aPos.AdjustY(aSize.Height() );

                // Get the height of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.setHeight( pSplitWin->GetSizePixel().Height() );

                aPos.AdjustY( -(aSize.Height()) );

                // Adjust width with regard to if a Window is already open
                // to the left or right.
                aPos.setX( aArea.Left() );
                aSize.setWidth( aArea.GetWidth() );

                // If already a window is opened at the top, then the
                // bottom one is not allowed to overlap this one.
                if ( aPos.Y() < aArea.Top() )
                {
                    aPos.setY( aArea.Top() );
                    aSize.setHeight( aArea.GetHeight() );
                }

                break;
            }
        }

        if ( !bDummyWindow )
            // the FadeIn-Window is a Floating window, which coordinates are
            // set in Screen coordinates.
            pSplitWin->SetPosSizePixel( pWorkWin->OutputToScreenPixel(aPos), aSize );
        else
            // the docked DummyWindow
            pDummy->SetPosSizePixel( aPos, aSize );
    }
}

tools::Rectangle SfxWorkWindow::GetFreeArea( bool bAutoHide ) const
{
    if ( bAutoHide )
    {
        tools::Rectangle aArea( aClientArea );
        for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
        {
            if ( pSplit[n]->IsPinned() || !pSplit[n]->IsVisible() )
                continue;

            Size aSize = pSplit[n]->GetSizePixel();
            switch ( n )
            {
                case 0 :
                    aArea.AdjustLeft(aSize.Width() );
                    break;
                case 1 :
                    aArea.AdjustRight( -(aSize.Width()) );
                    break;
                case 2 :
                    aArea.AdjustTop(aSize.Height() );
                    break;
                case 3 :
                    aArea.AdjustBottom( -(aSize.Height()) );
                    break;
            }
        }

        return aArea;
    }
    else
        return aClientArea;
}

void SfxWorkWindow::SetActiveChild_Impl( vcl::Window *pChild )
{
    pActiveChild = pChild;
}

void SfxWorkWindow::DataChanged_Impl()
{
    sal_uInt16 n;
    sal_uInt16 nCount = aChildWins.size();
    for (n=0; n<nCount; n++)
    {
        SfxChildWin_Impl*pCW = aChildWins[n].get();
        if (pCW && pCW->pWin)
        {
            // TODO does this really have any meaning ?
            if (pCW->pWin->GetWindow())
                pCW->pWin->GetWindow()->UpdateSettings(Application::GetSettings());
        }
    }

    ArrangeChildren_Impl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
