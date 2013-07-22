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

#include <stdio.h>
#include <boost/unordered_map.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include "workwin.hxx"
#include <sfx2/viewfrm.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include "splitwin.hxx"
#include <sfx2/msgpool.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/request.hxx>      // SFX_ITEMSET_SET
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>
#include <tools/rcid.h>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

struct ResIdToResName
{
    sal_uInt16      nId;
    const char* pName;
};

static const ResIdToResName pToolBarResToName[] =
{
    // OMG! hardcoded numbers that have nice (?) symbolic names
    // elsewhere.
    { 558,      "fullscreenbar"        }, // This 558 for instance equals RID_FULLSCREENTOOLBOX (in
                                          // value, and presumably also in semantics) from app.hrc in
                                          // this very same directory, so why RID_FULLSCREENTOOLBOX
                                          // can't be used I have no idea.

    { 560,      "standardbar",         }, // 560 is called RID_ENVTOOLBOX in app.hrc, still the same?

    { 18001,    "formsnavigationbar"   }, // Probably the rest are defined in .hrc files that are higher
                                          // up in the dependency chain and/or later in the build order,
                                          // and that is the (bad) reason why their symbolic names are
                                          // not available? Would it really be so owful to move the .hrc
                                          // files in question out from the modules where they now are?

    { 18002,    "formsfilterbar"       },
    { 18003,    "formtextobjectbar"    },
    { 18004,    "formcontrols"         },
    { 18005,    "moreformcontrols"     },
    { 18006,    "formdesign"           },
    { 20050,    "toolbar"              },      //math
    { 30001,    "objectbar"            },      //chart
    { 30513,    "toolbar"              },      //chart
    { 25005,    "textobjectbar"        },      //calc
    { 25053,    "drawobjectbar"        },
    { 25054,    "graphicobjectbar"     },
    { 25001,    "formatobjectbar"      },
    { 25006,    "previewbar"           },
    { 25035,    "toolbar"              },      //calc
    { 23015,    "bezierobjectbar"      },      //draw/impress
    { 23019,    "gluepointsobjectbar"  },
    { 23030,    "graphicobjectbar"     },
    { 23013,    "drawingobjectbar"     },      //impress
    { 23016,    "textobjectbar"        },      //impress
    { 23028,    "textobjectbar"        },      //draw
    { 23011,    "toolbar"              },      //impress
    { 23020,    "optionsbar"           },
    { 23021,    "commontaskbar"        },
    { 23025,    "toolbar"              },      //draw
    { 23026,    "optionsbar"           },
    { 23027,    "drawingobjectbar"     },      //draw
    { 23017,    "outlinetoolbar"       },      //impress
    { 23012,    "slideviewtoolbar"     },
    { 23014,    "slideviewobjectbar"   },
    { 23283,    "bezierobjectbar"      },      //writer
    { 23269,    "drawingobjectbar"     },
    { 23270,    "drawtextobjectbar"    },
    { 23267,    "frameobjectbar"       },
    { 23268,    "graphicobjectbar"     },
    { 23271,    "numobjectbar"         },
    { 23272,    "oleobjectbar"         },
    { 23266,    "tableobjectbar"       },
    { 23265,    "textobjectbar"        },
    { 20631,    "previewobjectbar"     },      //writer
    { 20402,    "toolbar"              },      //web
    { 20403,    "textobjectbar"        },
    { 23273,    "toolbar"              },      //writer
    { 20408,    "frameobjectbar"       },      //web
    { 20410,    "graphicobjectbar"     },
    { 20411,    "oleobjectbar"         },
    { 14850,    "macrobar"             },
    { 10987,    "fontworkobjectbar"    },      //global
    { 10986,    "extrusionobjectbar"   },
    { 23022,    "formsobjectbar"       },
    { 23310,    "viewerbar"            },      //writer (plugin)
    { 25000,    "viewerbar"            },      //calc   (plugin)
    { 23023,    "viewerbar"            },      //impress(plugin)
    { 23024,    "viewerbar"            },      //draw   (plugin)
    { 23031,    "mediaobjectbar"       },      //draw/impress
    { 25060,    "mediaobjectbar"       },      //calc
    { 23311,    "mediaobjectbar"       },      //writer
    { 23313,    "navigationobjectbar"  },      //writer
    { 0,        ""                     }
};

DBG_NAME(SfxWorkWindow)

//SV_IMPL_OBJARR( SfxObjectBarArr_Impl, SfxObjectBar_Impl );

//====================================================================
// Sort the Children according their alignment
// The order corresponds to the enum SfxChildAlignment (->CHILDWIN.HXX).
//

// Help to make changes to the alignment compatible!


LayoutManagerListener::LayoutManagerListener(
    SfxWorkWindow* pWrkWin ) :
    m_bHasFrame( sal_False ),
    m_pWrkWin( pWrkWin ),
    m_aLayoutManagerPropName( "LayoutManager" )
{
}

LayoutManagerListener::~LayoutManagerListener()
{
}

void LayoutManagerListener::setFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    SolarMutexGuard aGuard;
    if ( m_pWrkWin && !m_bHasFrame )
    {
        m_xFrame    = xFrame;
        m_bHasFrame = sal_True;

        if ( xFrame.is() )
        {
            css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            css::uno::Reference< css::frame::XLayoutManagerEventBroadcaster > xLayoutManager;
            if ( xPropSet.is() )
            {
                try
                {
                    Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
                    aValue >>= xLayoutManager;

                    if ( xLayoutManager.is() )
                        xLayoutManager->addLayoutManagerEventListener(
                            css::uno::Reference< css::frame::XLayoutManagerListener >(
                                static_cast< OWeakObject* >( this ), css::uno::UNO_QUERY ));

                    xPropSet = css::uno::Reference< css::beans::XPropertySet >( xLayoutManager, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        aValue = xPropSet->getPropertyValue(
                            OUString( "LockCount" ) );
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
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//  XComponent
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManagerListener::addEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing, only internal class
}

void SAL_CALL LayoutManagerListener::removeEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing, only internal class
}

void SAL_CALL LayoutManagerListener::dispose()
throw( css::uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    // reset member
    m_pWrkWin = 0;

    css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame.get(), css::uno::UNO_QUERY );
    if ( xFrame.is() )
    {
        m_xFrame = css::uno::Reference< css::frame::XFrame >();
        m_bHasFrame = sal_False;

        css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
        css::uno::Reference< css::frame::XLayoutManagerEventBroadcaster > xLayoutManager;
        if ( xPropSet.is() )
        {
            try
            {
                css::uno::Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
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
    }
}

//---------------------------------------------------------------------------------------------------------
//  XEventListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManagerListener::disposing(
    const css::lang::EventObject& )
throw( css::uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    m_pWrkWin = 0;
    m_bHasFrame = sal_False;
    m_xFrame = css::uno::Reference< css::frame::XFrame >();
}

//---------------------------------------------------------------------------------------------------------
// XLayoutManagerEventListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManagerListener::layoutEvent(
    const css::lang::EventObject&,
    ::sal_Int16                   eLayoutEvent,
    const css::uno::Any&                        )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( m_pWrkWin )
    {
        if ( eLayoutEvent == css::frame::LayoutManagerEvents::VISIBLE )
        {
            m_pWrkWin->MakeVisible_Impl( sal_True );
            m_pWrkWin->ShowChildren_Impl();
            m_pWrkWin->ArrangeChildren_Impl( sal_True );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::INVISIBLE )
        {
            m_pWrkWin->MakeVisible_Impl( sal_False );
            m_pWrkWin->HideChildren_Impl();
            m_pWrkWin->ArrangeChildren_Impl( sal_True );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::LOCK )
        {
            m_pWrkWin->Lock_Impl( sal_True );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::UNLOCK )
        {
            m_pWrkWin->Lock_Impl( sal_False );
        }
    }
}

namespace
{
    class FilledToolBarResIdToResourceURLMap
    {
    private:
        typedef boost::unordered_map< sal_Int32, OUString > ToolBarResIdToResourceURLMap;
        ToolBarResIdToResourceURLMap m_aResIdToResourceURLMap;
    public:
        FilledToolBarResIdToResourceURLMap()
        {
            sal_Int32 nIndex( 0 );
            while ( pToolBarResToName[nIndex].nId != 0 )
            {
                OUString aResourceURL( OUString::createFromAscii( pToolBarResToName[nIndex].pName ));
                m_aResIdToResourceURLMap.insert( ToolBarResIdToResourceURLMap::value_type(
                                                    sal_Int32( pToolBarResToName[nIndex].nId ), aResourceURL ));
                ++nIndex;
            }
        }

        OUString findURL(sal_uInt16 nResId) const
        {
            ToolBarResIdToResourceURLMap::const_iterator aIter = m_aResIdToResourceURLMap.find( nResId );
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

static OUString GetResourceURLFromResId( sal_uInt16 nResId )
{
    return theFilledToolBarResIdToResourceURLMap::get().findURL(nResId);
}

sal_Bool IsAppWorkWinToolbox_Impl( sal_uInt16 nPos )
{
    switch ( nPos )
    {
        case SFX_OBJECTBAR_APPLICATION :
        case SFX_OBJECTBAR_MACRO:
        case SFX_OBJECTBAR_FULLSCREEN:
            return sal_True;
        default:
            return sal_False;
    }
}

sal_uInt16 TbxMatch( sal_uInt16 nPos )
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

sal_uInt16 ChildAlignValue(SfxChildAlignment eAlign)
{
    sal_uInt16 ret = 17;

    switch (eAlign)
    {
        case SFX_ALIGN_HIGHESTTOP:
            ret = 1;
            break;
        case SFX_ALIGN_LOWESTBOTTOM:
            ret = 2;
            break;
        case SFX_ALIGN_FIRSTLEFT:
            ret = 3;
            break;
        case SFX_ALIGN_LASTRIGHT:
            ret = 4;
            break;
        case SFX_ALIGN_LEFT:
            ret = 5;
            break;
        case SFX_ALIGN_RIGHT:
            ret = 6;
            break;
        case SFX_ALIGN_FIRSTRIGHT:
            ret = 7;
            break;
        case SFX_ALIGN_LASTLEFT:
            ret = 8;
            break;
        case SFX_ALIGN_TOP:
            ret = 9;
            break;
        case SFX_ALIGN_BOTTOM:
            ret = 10;
            break;
        case SFX_ALIGN_TOOLBOXTOP:
            ret = 11;
            break;
        case SFX_ALIGN_TOOLBOXBOTTOM:
            ret = 12;
            break;
        case SFX_ALIGN_LOWESTTOP:
            ret = 13;
            break;
        case SFX_ALIGN_HIGHESTBOTTOM:
            ret = 14;
            break;
        case SFX_ALIGN_TOOLBOXLEFT:
            ret = 15;
            break;
        case SFX_ALIGN_TOOLBOXRIGHT:
            ret = 16;
            break;
        case SFX_ALIGN_NOALIGNMENT:
            break;  // -Wall not handled...
    }

    return ret;
}

sal_uInt16 ChildTravelValue( SfxChildAlignment eAlign )
{
    sal_uInt16 ret = 17;

    switch (eAlign)
    {
        case SFX_ALIGN_FIRSTLEFT:
            ret = 1;
            break;
        case SFX_ALIGN_LEFT:
            ret = 2;
            break;
        case SFX_ALIGN_LASTLEFT:
            ret = 3;
            break;
        case SFX_ALIGN_TOOLBOXLEFT:
            ret = 4;
            break;
        case SFX_ALIGN_HIGHESTTOP:
            ret = 5;
            break;
        case SFX_ALIGN_TOP:
            ret = 6;
            break;
        case SFX_ALIGN_TOOLBOXTOP:
            ret = 7;
            break;
        case SFX_ALIGN_LOWESTTOP:
            ret = 8;
            break;
        case SFX_ALIGN_HIGHESTBOTTOM:
            ret = 9;
            break;
        case SFX_ALIGN_TOOLBOXBOTTOM:
            ret = 10;
            break;
        case SFX_ALIGN_BOTTOM:
            ret = 11;
            break;
        case SFX_ALIGN_LOWESTBOTTOM:
            ret = 12;
            break;
        case SFX_ALIGN_TOOLBOXRIGHT:
            ret = 13;
            break;
        case SFX_ALIGN_FIRSTRIGHT:
            ret = 14;
            break;
        case SFX_ALIGN_RIGHT:
            ret = 15;
            break;
        case SFX_ALIGN_LASTRIGHT:
            ret = 16;
            break;
        case SFX_ALIGN_NOALIGNMENT:
            break;  // -Wall not handled.
    }

    return ret;
}

void SfxWorkWindow::Sort_Impl()
{
    aSortedList.clear();
    for (sal_uInt16 i = 0; i < aChildren.size(); ++i)
    {
        SfxChild_Impl *pCli = aChildren[i];
        if (pCli)
        {
            sal_uInt16 k;
            for (k=0; k<aSortedList.size(); k++)
                if (ChildAlignValue( aChildren[aSortedList[k]]->eAlign ) >
                    ChildAlignValue(pCli->eAlign))
                    break;
            aSortedList.insert( aSortedList.begin() + k, i );
        }
    }

    bSorted = sal_True;
}


//====================================================================
// constructor for workwin of a Frame

SfxFrameWorkWin_Impl::SfxFrameWorkWin_Impl( Window *pWin, SfxFrame *pFrm, SfxFrame* pMaster )
    : SfxWorkWindow(
        pWin,
        pFrm->GetCurrentViewFrame()->GetBindings(),
        pFrm->GetParentFrame() ? pFrm->GetParentFrame()->GetWorkWindow_Impl() : NULL )
    , pMasterFrame( pMaster )
    , pFrame( pFrm )
{
    pConfigShell = pFrm->GetCurrentViewFrame();
    if ( pConfigShell && pConfigShell->GetObjectShell() )
    {
        bShowStatusBar = ( !pConfigShell->GetObjectShell()->IsInPlaceActive() );
        bDockingAllowed = sal_True;
        bInternalDockingAllowed = sal_True;
    }

    // The required split windows (one for each side) can be created
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // The SplitWindows excludes direct ChildWindows of the WorkWindows
        // and receives the docked window.

        SfxChildAlignment eAlign =
                        ( n == SFX_SPLITWINDOWS_LEFT ? SFX_ALIGN_LEFT :
                            n == SFX_SPLITWINDOWS_RIGHT ? SFX_ALIGN_RIGHT :
                            n == SFX_SPLITWINDOWS_TOP ? SFX_ALIGN_TOP :
                                SFX_ALIGN_BOTTOM );
        SfxSplitWindow *pSplitWin = new SfxSplitWindow(pWorkWin, eAlign, this, pParent==0 );
        pSplit[n] = pSplitWin;
    }

    nOrigMode = SFX_VISIBILITY_STANDARD;
    nUpdateMode = SFX_VISIBILITY_STANDARD;
}

//====================================================================
// Constructor of the base class

SfxWorkWindow::SfxWorkWindow( Window *pWin, SfxBindings& rB, SfxWorkWindow* pParentWorkwin ) :
    pParent( pParentWorkwin ),
    pBindings(&rB),
    pWorkWin (pWin),
    pConfigShell( 0 ),
    pActiveChild( 0 ),
    nChildren( 0 ),
    nOrigMode( 0 ),
    bSorted( sal_True ),
    bDockingAllowed(sal_True),
    bInternalDockingAllowed(sal_True),
    bAllChildrenVisible(sal_True),
#if HAVE_FEATURE_DESKTOP_GUI_ELEMENTS
    bIsFullScreen( sal_False ),
    bShowStatusBar( sal_True ),
#else
    bIsFullScreen( sal_True ),
    bShowStatusBar( sal_False ),
#endif
    m_nLock( 0 ),
    m_aStatusBarResName( "private:resource/statusbar/statusbar" ),
    m_aLayoutManagerPropName( "LayoutManager" ),
    m_aTbxTypeName( "private:resource/toolbar/" ),
    m_aProgressBarResName( "private:resource/progressbar/progressbar" )
{
    DBG_CTOR(SfxWorkWindow, 0);
    DBG_ASSERT (pBindings, "No Bindings!");

    pBindings->SetWorkWindow_Impl( this );

    // For the ObjectBars a integral place in the Childlist is reserved,
    // so that they always come in a defined order.
    aChildren.insert( aChildren.begin(), SFX_OBJECTBAR_MAX, (SfxChild_Impl*)NULL );

    // create and initialize layout manager listener
    Reference< com::sun::star::frame::XFrame > xFrame = GetFrameInterface();
    LayoutManagerListener* pLayoutManagerListener = new LayoutManagerListener( this );
    m_xLayoutManagerListener = css::uno::Reference< css::lang::XComponent >(
                                    static_cast< cppu::OWeakObject* >( pLayoutManagerListener ),
                                        css::uno::UNO_QUERY );
    pLayoutManagerListener->setFrame( xFrame );
}

//====================================================================
// Destructor

SfxWorkWindow::~SfxWorkWindow()
{
    DBG_DTOR(SfxWorkWindow, 0);

    // Delete SplitWindows
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            ReleaseChild_Impl(*p);
        delete p;
    }

    // Delete help structure for Child-Windows
    DBG_ASSERT( aChildren.empty(), "dangling children" );

    if ( m_xLayoutManagerListener.is() )
        m_xLayoutManagerListener->dispose();
}

void SfxWorkWindow::Lock_Impl( sal_Bool bLock )
{
    if ( bLock )
        m_nLock++;
    else
        --m_nLock;
    if ( m_nLock<0 )
    {
        OSL_FAIL("Lock count underflow!");
        m_nLock = 0;
    }

    if ( !m_nLock )
        ArrangeChildren_Impl();
}

//--------------------------------------------------------------------
// Helper method to release the child lists. Should the destructor not be
// called after this, instead work continues, then space for the object bars
// and split windows has to be reserved in the same way as in the constructor
// of SfxWorkWindow.

void SfxWorkWindow::DeleteControllers_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    // Lock SplitWindows (which means supressing the Resize-Reaction of the
    // DockingWindows)
    sal_uInt16 n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
           if (p->GetWindowCount())
        p->Lock();
    }

    // Delete Child-Windows
    for ( n=0; n<aChildWins.size(); )
    {
        SfxChildWin_Impl* pCW = aChildWins[n];
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
            pCW->pWin = 0;
            pWorkWin->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( pChild->GetWindow() );
            pChild->Destroy();
        }

        delete pCW;

        // ATTENTION: The array itself is cleared after this loop!!
        // Therefore we have to set every array entry to zero as it could be
        // accessed by calling pChild->Destroy().
        // Window::NotifyAllChildren() calls SfxWorkWindow::DataChanged_Impl for
        // 8-bit displays (WM_QUERYPALETTECHANGED message due to focus change)!!
    }

    Reference< com::sun::star::frame::XFrame > xFrame = GetFrameInterface();
    Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
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
        for ( sal_uInt16 i = 0; i < aObjBarList.size(); i++ )
        {
            // Not every position must be occupied
            sal_uInt16 nId = aObjBarList[i].nId;
            if ( nId )
                aObjBarList[i].nId = 0;
        }
    }

    // ObjectBars are all released at once, since they occupy a
    // fixed contiguous area in the array pChild
    aChildren.clear();
    bSorted = sal_False;

    nChildren = 0;
}

//====================================================================
// Virtual method for placing the child window.

void SfxWorkWindow::ArrangeChildren_Impl( sal_Bool /*bForce*/)
{
    Arrange_Impl();
}

void SfxFrameWorkWin_Impl::ArrangeChildren_Impl( sal_Bool bForce )
{
    if ( pFrame->IsClosing_Impl() || ( m_nLock && !bForce ))
        return;

    SfxInPlaceClient *pClient = 0;
    SfxViewFrame *pF = pFrame->GetCurrentViewFrame();
    if ( pF && pF->GetViewShell() )
        pClient = pF->GetViewShell()->GetIPClient();

    if ( pClient )
        return;

    aClientArea = GetTopRect_Impl();
    if ( aClientArea.IsEmpty() )
        return;

    SvBorder aBorder;
    if ( nChildren )
    {
        if ( IsVisible_Impl() )
            aBorder = Arrange_Impl();
    }
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

    ArrangeAutoHideWindows( NULL );
}

//--------------------------------------------------------------------

SvBorder SfxWorkWindow::Arrange_Impl()

/*  [Description]

    This method organizes all visible child windows so that the docked window
    sorted in order from the outside to the inside are placed after one
    another. If a visible window does not fit anymore into the free
    ClientArea, it is set to "not visible".
*/
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    aClientArea = GetTopRect_Impl();
    aUpperClientArea = aClientArea;

    SvBorder aBorder;
    if ( !nChildren )
        return aBorder;

    if (!bSorted)
        Sort_Impl();

    Point aPos;
    Size aSize;
    Rectangle aTmp( aClientArea );

    for ( sal_uInt16 n=0; n<aSortedList.size(); ++n )
    {
        SfxChild_Impl* pCli = aChildren[aSortedList[n]];
        if ( !pCli->pWin )
            continue;

        // First, we assume that there is room for the window.
        pCli->nVisible |= CHILD_FITS_IN;

        // Skip invisiable windows
        if (pCli->nVisible != CHILD_VISIBLE)
            continue;

        if ( pCli->bResize )
            aSize = pCli->aSize;
        else
            aSize = pCli->pWin->GetSizePixel();

        SvBorder aTemp = aBorder;
        sal_Bool bAllowHiding = sal_True;
        switch ( pCli->eAlign )
        {
            case SFX_ALIGN_HIGHESTTOP:
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_TOOLBOXTOP:
            case SFX_ALIGN_LOWESTTOP:
                aSize.Width() = aTmp.GetWidth();
                if ( pCli->pWin->GetType() == WINDOW_SPLITWINDOW )
                    aSize = ((SplitWindow *)(pCli->pWin))->CalcLayoutSizePixel( aSize );
                bAllowHiding = sal_False;
                aBorder.Top() += aSize.Height();
                aPos = aTmp.TopLeft();
                aTmp.Top() += aSize.Height();
                if ( pCli->eAlign == SFX_ALIGN_HIGHESTTOP )
                    aUpperClientArea.Top() += aSize.Height();
                break;

            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_TOOLBOXBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                aSize.Width() = aTmp.GetWidth();
                if ( pCli->pWin->GetType() == WINDOW_SPLITWINDOW )
                    aSize = ((SplitWindow *)(pCli->pWin))->CalcLayoutSizePixel( aSize );
                aBorder.Bottom() += aSize.Height();
                aPos = aTmp.BottomLeft();
                aPos.Y() -= (aSize.Height()-1);
                aTmp.Bottom() -= aSize.Height();
                if ( pCli->eAlign == SFX_ALIGN_LOWESTBOTTOM )
                    aUpperClientArea.Bottom() -= aSize.Height();
                break;

            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_LASTLEFT:
            case SFX_ALIGN_TOOLBOXLEFT:
                aSize.Height() = aTmp.GetHeight();
                if ( pCli->pWin->GetType() == WINDOW_SPLITWINDOW )
                    aSize = ((SplitWindow *)(pCli->pWin))->CalcLayoutSizePixel( aSize );
                bAllowHiding = sal_False;
                aBorder.Left() += aSize.Width();
                aPos = aTmp.TopLeft();
                aTmp.Left() += aSize.Width();
                if ( pCli->eAlign != SFX_ALIGN_TOOLBOXLEFT )
                    aUpperClientArea.Left() += aSize.Width();
                break;

            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_LASTRIGHT:
            case SFX_ALIGN_TOOLBOXRIGHT:
                aSize.Height() = aTmp.GetHeight();
                if ( pCli->pWin->GetType() == WINDOW_SPLITWINDOW )
                    aSize = ((SplitWindow *)(pCli->pWin))->CalcLayoutSizePixel( aSize );
                aBorder.Right() += aSize.Width();
                aPos = aTmp.TopRight();
                aPos.X() -= (aSize.Width()-1);
                aTmp.Right() -= aSize.Width();
                if ( pCli->eAlign != SFX_ALIGN_TOOLBOXRIGHT )
                    aUpperClientArea.Right() -= aSize.Width();
                break;

            default:
                pCli->aSize = pCli->pWin->GetSizePixel();
                pCli->bResize = sal_False;
                continue;
        }

        pCli->pWin->SetPosSizePixel( aPos, aSize );
        pCli->bResize = sal_False;
        pCli->aSize = aSize;
        if( bAllowHiding && !RequestTopToolSpacePixel_Impl( aBorder ) )
        {
            pCli->nVisible ^= CHILD_FITS_IN;
            aBorder = aTemp;
        }
    }

    if ( aClientArea.GetWidth() >= aBorder.Left() + aBorder.Right() )
    {
        aClientArea.Left() += aBorder.Left();
        aClientArea.Right() -= aBorder.Right();
    }
    else
    {
        aBorder.Left() = aClientArea.Left();
        aBorder.Right() = aClientArea.Right();
        aClientArea.Right() = aClientArea.Left() = aTmp.Left();
    }

    if ( aClientArea.GetHeight() >= aBorder.Top() + aBorder.Bottom() )
    {
        aClientArea.Top() += aBorder.Top();
        aClientArea.Bottom() -= aBorder.Bottom();
    }
    else
    {
        aBorder.Top() = aClientArea.Top();
        aBorder.Bottom() = aClientArea.Bottom();
        aClientArea.Top() = aClientArea.Bottom() = aTmp.Top();
    }

    return IsDockingAllowed() ? aBorder : SvBorder();
}

sal_Bool SfxWorkWindow::PrepareClose_Impl()
{
    for (sal_uInt16 n=0; n<aChildWins.size(); n++)
    {
        SfxChildWin_Impl *pCW  = aChildWins[n];
        SfxChildWindow *pChild = pCW->pWin;
        if ( pChild && !pChild->QueryClose() )
            return sal_False;
    }

    return sal_True;
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::RegisterChild_Impl( Window& rWindow,
                    SfxChildAlignment eAlign, sal_Bool bCanGetFocus )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
    DBG_ASSERT( aChildren.size() < 255, "too many children" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );
    DBG_ASSERT( !FindChild_Impl(rWindow), "child registered more than once" );


    if ( rWindow.GetParent() != pWorkWin )
        rWindow.SetParent( pWorkWin );

    SfxChild_Impl *pChild = new SfxChild_Impl(rWindow, rWindow.GetSizePixel(),
                                    eAlign, rWindow.IsVisible());
    pChild->bCanGetFocus = bCanGetFocus;

    aChildren.push_back(pChild);
    bSorted = sal_False;
    nChildren++;
    return aChildren.back();
}

//--------------------------------------------------------------------

void SfxWorkWindow::ReleaseChild_Impl( Window& rWindow )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    SfxChild_Impl *pChild = 0;
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < aChildren.size(); ++nPos )
    {
        pChild = aChildren[nPos];
        if ( pChild && pChild->pWin == &rWindow )
            break;
    }

    if ( nPos < aChildren.size() )
    {
        bSorted = sal_False;
        nChildren--;
        aChildren.erase(aChildren.begin() + nPos);
        delete pChild;
    }
    else {
        OSL_FAIL( "releasing unregistered child" );
    }
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::FindChild_Impl( const Window& rWindow ) const
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    SfxChild_Impl *pChild = 0;
    sal_uInt16 nCount = aChildren.size();
    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
    {
        pChild = aChildren[nPos];
        if ( pChild && pChild->pWin == &rWindow )
            return pChild;
    }

    return 0;
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChildren_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    bool bInvisible = ( !IsVisible_Impl() || ( !pWorkWin->IsReallyVisible() && !pWorkWin->IsReallyShown() ));

    for ( sal_uInt16 nPos = 0; nPos < aChildren.size(); ++nPos )
    {
        SfxChildWin_Impl* pCW = 0;
        SfxChild_Impl *pCli = aChildren[nPos];

        if ( pCli && pCli->pWin )
        {
            // We have to find the SfxChildWin_Impl to retrieve the
            // SFX_CHILDWIN flags that can influence visibility.
            for (sal_uInt16 n=0; n<aChildWins.size(); n++)
            {
                SfxChildWin_Impl* pCWin = aChildWins[n];
                SfxChild_Impl*    pChild  = pCWin->pCli;
                if ( pChild == pCli )
                {
                    pCW = pCWin;
                    break;
                }
            }

            bool bVisible( !bInvisible );
            if ( pCW )
            {
                // Check flag SFX_CHILDWIN_NEVERHIDE that forces us to show
                // the child window even in situations where no child window is
                // visible.
                sal_uInt16 nFlags = pCW->aInfo.nFlags;
                bVisible = !bInvisible || (( nFlags & SFX_CHILDWIN_NEVERHIDE ) != 0 );
            }

            if ( CHILD_VISIBLE == (pCli->nVisible & CHILD_VISIBLE) && bVisible )
            {
                sal_uInt16 nFlags = pCli->bSetFocus ? 0 : SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE;
                switch ( pCli->pWin->GetType() )
                {
                    case RSC_DOCKINGWINDOW :
                        ((DockingWindow*)pCli->pWin)->Show( sal_True, nFlags );
                        break;
                    case RSC_SPLITWINDOW :
                        ((SplitWindow*)pCli->pWin)->Show( sal_True, nFlags );
                        break;
                    default:
                        pCli->pWin->Show( sal_True, nFlags );
                        break;
                }

                pCli->bSetFocus = sal_False;
            }
            else
            {
                switch ( pCli->pWin->GetType() )
                {
                    case RSC_DOCKINGWINDOW :
                        ((DockingWindow*)pCli->pWin)->Hide();
                        break;
                    default:
                        pCli->pWin->Hide();
                        break;
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxWorkWindow::HideChildren_Impl()
{
    for ( sal_uInt16 nPos = aChildren.size(); nPos > 0; --nPos )
    {
        SfxChild_Impl *pChild = aChildren[nPos-1];
        if (pChild && pChild->pWin)
        {
            switch ( pChild->pWin->GetType() )
            {
                case RSC_DOCKINGWINDOW :
                    ((DockingWindow*)pChild->pWin)->Hide();
                    break;
                default:
                    pChild->pWin->Hide();
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------

void SfxWorkWindow::ResetObjectBars_Impl()
{
    sal_uInt16 n;
    for ( n = 0; n < aObjBarList.size(); n++ )
        aObjBarList[n].bDestroy = sal_True;

    for ( n = 0; n < aChildWins.size(); ++n )
        aChildWins[n]->nId = 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::SetObjectBar_Impl( sal_uInt16 nPos, sal_uInt32 nResId,
            SfxInterface* pIFace, const String *pName)
{
    DBG_ASSERT( (nPos & SFX_POSITION_MASK) < SFX_OBJECTBAR_MAX,
                "object bar position overflow" );

    sal_uInt16 nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
    {
        pParent->SetObjectBar_Impl( nPos, nResId, pIFace, pName );
        return;
    }

    SfxObjectBar_Impl aObjBar;
    aObjBar.pIFace = pIFace;
    aObjBar.nId = sal::static_int_cast<sal_uInt16>(nResId);
    aObjBar.nPos = nRealPos;
    aObjBar.nMode = (nPos & SFX_VISIBILITY_MASK);
    if (pName)
        aObjBar.aName = *pName;
    else
        aObjBar.aName.Erase();

    for ( sal_uInt16 n=0; n<aObjBarList.size(); n++ )
    {
        if ( aObjBarList[n].nId == aObjBar.nId )
        {
            aObjBarList[n] = aObjBar;
            return;
        }
    }

    aObjBarList.push_back( aObjBar );
}

//------------------------------------------------------------------------

bool SfxWorkWindow::KnowsObjectBar_Impl( sal_uInt16 nPos ) const

/*  [Description]

    Determines if a object list is available at the position in question.
    This is independent for the fact whether it is actually turned on or off.
*/

{
    sal_uInt16 nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
        return pParent->KnowsObjectBar_Impl( nPos );

    for ( sal_uInt16 n=0; n<aObjBarList.size(); n++ )
    {
        if ( aObjBarList[n].nPos == nRealPos )
            return true;
    }

    return false;
}

//------------------------------------------------------------------------

sal_Bool SfxWorkWindow::IsVisible_Impl( sal_uInt16 nMode ) const
{
    switch( nUpdateMode )
    {
        case SFX_VISIBILITY_STANDARD:
            return sal_True;
        case SFX_VISIBILITY_UNVISIBLE:
            return sal_False;
        case SFX_VISIBILITY_CLIENT:
        case SFX_VISIBILITY_SERVER:
            return !!(nMode & nUpdateMode);
        default:
            return !!(nMode & nOrigMode ) ||
                nOrigMode == SFX_VISIBILITY_STANDARD;
    }
}

void SfxFrameWorkWin_Impl::UpdateObjectBars_Impl()
{
    if ( pFrame->IsClosing_Impl() )
        return;

    SfxWorkWindow *pWork = pParent;
    while ( pWork )
    {
        pWork->SfxWorkWindow::UpdateObjectBars_Impl();
        pWork = pWork->GetParent_Impl();
    }

    SfxWorkWindow::UpdateObjectBars_Impl();

    {
        pWork = pParent;
        while ( pWork )
        {
            pWork->ArrangeChildren_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ArrangeChildren_Impl( sal_False );

        pWork = pParent;
        while ( pWork )
        {
            pWork->ShowChildren_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ShowChildren_Impl();
    }

    ShowChildren_Impl();
}

Reference< ::com::sun::star::task::XStatusIndicator > SfxWorkWindow::GetStatusIndicator()
{
    Reference< com::sun::star::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
    Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
        aValue >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            xLayoutManager->createElement( m_aProgressBarResName );
            xLayoutManager->showElement( m_aProgressBarResName );

            Reference< ::com::sun::star::ui::XUIElement > xProgressBar =
                xLayoutManager->getElement( m_aProgressBarResName );
            if ( xProgressBar.is() )
            {
                xStatusIndicator = Reference< ::com::sun::star::task::XStatusIndicator >(
                    xProgressBar->getRealInterface(), UNO_QUERY );
            }
        }
    }

    return xStatusIndicator;
}

//------------------------------------------------------------------------

sal_Bool SfxWorkWindow::IsPluginMode( SfxObjectShell* pObjShell )
{
    if ( pObjShell && pObjShell->GetMedium() )
    {
        SFX_ITEMSET_ARG( pObjShell->GetMedium()->GetItemSet(), pViewOnlyItem, SfxBoolItem, SID_VIEWONLY, sal_False );
        if ( pViewOnlyItem && pViewOnlyItem->GetValue() )
            return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SfxWorkWindow::GetFrameInterface()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;

    SfxDispatcher* pDispatcher( GetBindings().GetDispatcher() );
    if ( pDispatcher )
    {
        SfxViewFrame* pFrame = pDispatcher->GetFrame();
        if ( pFrame )
           xFrame = pFrame->GetFrame().GetFrameInterface();
    }

    return xFrame;
}

//------------------------------------------------------------------------

void SfxWorkWindow::UpdateObjectBars_Impl()
{
    // Lock SplitWindows (which means supressing the Resize-Reaction of the
    // DockingWindows)
    sal_uInt16 n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    // you realize what is needed often (saves Code and execution time)
    SFX_APP();

    Reference< com::sun::star::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
        aValue >>= xLayoutManager;
    }

    if ( !xLayoutManager.is() )
        return;

    sal_Bool       bPluginMode( sal_False );
    SfxDispatcher* pDispatcher( GetBindings().GetDispatcher() );

    if ( pDispatcher )
    {
        SfxViewFrame* pFrame = pDispatcher->GetFrame();
        if ( pFrame )
           bPluginMode = IsPluginMode( pFrame->GetObjectShell() );
    }

    // Iterate over all Toolboxes
    xLayoutManager->lock();
    for ( n = 0; n < aObjBarList.size(); ++n )
    {
        sal_uInt16      nId      = aObjBarList[n].nId;
        sal_Bool    bDestroy = aObjBarList[n].bDestroy;

        // Determine the vaild mode for the ToolBox
        sal_uInt16 nTbxMode = aObjBarList[n].nMode;
        bool bFullScreenTbx = SFX_VISIBILITY_FULLSCREEN ==
                                  ( nTbxMode & SFX_VISIBILITY_FULLSCREEN );
        nTbxMode &= ~SFX_VISIBILITY_FULLSCREEN;
        nTbxMode &= ~SFX_VISIBILITY_VIEWER;

        // Is a ToolBox required in this context ?
        bool bModesMatching = ( nUpdateMode && ( nTbxMode & nUpdateMode) == nUpdateMode );
        if ( bDestroy )
        {
            OUString aTbxId( m_aTbxTypeName );
            aTbxId += GetResourceURLFromResId( aObjBarList[n].nId );
            xLayoutManager->destroyElement( aTbxId );
        }
        else if ( nId != 0 && ( ( bModesMatching && !bIsFullScreen ) ||
                                ( bIsFullScreen && bFullScreenTbx ) ) )
        {
            OUString aTbxId( m_aTbxTypeName );
            aTbxId += GetResourceURLFromResId( aObjBarList[n].nId );
            if ( !IsDockingAllowed() && !xLayoutManager->isElementFloating( aTbxId ))
                xLayoutManager->destroyElement( aTbxId );
            else
            {
                xLayoutManager->requestElement( aTbxId );
                if ( bPluginMode )
                    xLayoutManager->lockWindow( aTbxId );
            }
        }
        else if ( nId != 0 )
        {
            // Delete the Toolbox at this Position if possible
            OUString aTbxId( m_aTbxTypeName );
            aTbxId += GetResourceURLFromResId( aObjBarList[n].nId );
            xLayoutManager->destroyElement( aTbxId );
        }
    }

    UpdateStatusBar_Impl();

    // unlocking automatically forces Layout
    xLayoutManager->unlock();

    UpdateChildWindows_Impl();

    // Unlock the SplitWindows again
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock(sal_False);
    }
}

bool SfxWorkWindow::AllowChildWindowCreation_Impl( const SfxChildWin_Impl& i_rCW ) const
{
    // or checking the availability of child windows, we need access to the module
    const SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    const SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : NULL;
    const SfxModule* pModule = pShell ? pShell->GetModule() : NULL;
    ENSURE_OR_RETURN( pModule, "SfxWorkWindow::UpdateChildWindows_Impl: did not find an SfxModule to ask for the child win availability!", true );
    return pModule->IsChildWindowAvailable( i_rCW.nId, pViewFrame );
}

void SfxWorkWindow::UpdateChildWindows_Impl()
{
    // any current or in the context available Childwindows
    for ( sal_uInt16 n=0; n<aChildWins.size(); n++ )
    {
        SfxChildWin_Impl *pCW = aChildWins[n];
        SfxChildWindow *pChildWin = pCW->pWin;
        sal_Bool bCreate = sal_False;
        if ( pCW->nId && !pCW->bDisabled  && (pCW->aInfo.nFlags & SFX_CHILDWIN_ALWAYSAVAILABLE || IsVisible_Impl( pCW->nVisibility ) ) )
        {
            // In the context is an appropriate ChildWindow allowed;
            // it is also turned on?
            if ( pChildWin == NULL && pCW->bCreate )
            {
                // Internal docking is only used for embedding into another
                // container. We force the floating state of all floatable
                // child windows.
                if ( !bInternalDockingAllowed )
                {
                    // Special case for all non-floatable child windows. We have
                    // to prevent the creation here!
                    bCreate = !( pCW->aInfo.nFlags & SFX_CHILDWIN_FORCEDOCK );
                }
                else if ( !IsDockingAllowed() || bIsFullScreen ) // || !bInternalDocking )
                {
                    // In Presentation mode or FullScreen only FloatingWindows
                    SfxChildAlignment eAlign;
                    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
                        bCreate = ( eAlign == SFX_ALIGN_NOALIGNMENT );
                }
                else
                    bCreate = sal_True;

                if ( bCreate )
                    bCreate = AllowChildWindowCreation_Impl( *pCW );

                // Currently, no window here, but it is enabled; windows
                // Create window and if possible theContext
                if ( bCreate )
                    CreateChildWin_Impl( pCW, sal_False );

                if ( !bAllChildrenVisible )
                {
                    if ( pCW->pCli )
                        pCW->pCli->nVisible &= ~CHILD_ACTIVE;
                }
            }
            else if ( pChildWin )
            {
                // Window already exists, it should also be visible?
                if ( ( !bIsFullScreen || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT ) && bAllChildrenVisible )
                {
                    // Update Mode is compatible; definitely enable it
                    bCreate = AllowChildWindowCreation_Impl( *pCW );
                    if ( bCreate )
                    {
                        if ( pCW->pCli )
                        {
                            // The window is a direct Child
                            if ( bAllChildrenVisible && ( (IsDockingAllowed() && bInternalDockingAllowed) || pCW->pCli->eAlign == SFX_ALIGN_NOALIGNMENT ) )
                                pCW->pCli->nVisible |= CHILD_NOT_HIDDEN;
                        }
                        else
                        {
                            if ( pCW->bCreate && IsDockingAllowed() && bInternalDockingAllowed )
                                // The window ia within a SplitWindow
                                ((SfxDockingWindow*)pChildWin->GetWindow())->Reappear_Impl();
                        }

                        if ( pCW->nInterfaceId != pChildWin->GetContextId() )
                            pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );
                    }
                }
            }
        }

        if ( pChildWin && !bCreate )
        {
            if ( !pChildWin->QueryClose() || pChildWin->IsHideNotDelete() || Application::IsUICaptured() )
            {
                if ( pCW->pCli )
                {
                    if ( pCW->pCli->nVisible & CHILD_NOT_HIDDEN )
                        pCW->pCli->nVisible ^= CHILD_NOT_HIDDEN;
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Disappear_Impl();
            }
            else
                RemoveChildWin_Impl( pCW );
        }
    }
}

void SfxWorkWindow::CreateChildWin_Impl( SfxChildWin_Impl *pCW, sal_Bool bSetFocus )
{
    if ( pCW->aInfo.bVisible != 42 )
        pCW->aInfo.bVisible = sal_True;

    SfxChildWindow *pChildWin = SfxChildWindow::CreateChildWindow( pCW->nId, pWorkWin, &GetBindings(), pCW->aInfo);
    if (pChildWin)
    {
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
                aChildren[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
            }
        }

        // make childwin keyboard accessible
        pWorkWin->GetSystemWindow()->GetTaskPaneList()->AddWindow( pChildWin->GetWindow() );

        pCW->pWin = pChildWin;

        if ( pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT || pChildWin->GetWindow()->GetParent() == pWorkWin)
        {
            // The window is not docked or docked outside of one split windows
            // and must therefore be registered explicitly as a Child
            pCW->pCli = RegisterChild_Impl(*(pChildWin->GetWindow()), pChildWin->GetAlignment(), pChildWin->CanGetFocus());
            pCW->pCli->nVisible = CHILD_VISIBLE;
            if ( pChildWin->GetAlignment() != SFX_ALIGN_NOALIGNMENT && bIsFullScreen )
                pCW->pCli->nVisible ^= CHILD_ACTIVE;
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
}

void SfxWorkWindow::RemoveChildWin_Impl( SfxChildWin_Impl *pCW )
{
    sal_uInt16 nId = pCW->nSaveId;
    SfxChildWindow *pChildWin = pCW->pWin;

    // Save the information in the INI file
    sal_uInt16 nFlags = pCW->aInfo.nFlags;
    pCW->aInfo = pChildWin->GetInfo();
    pCW->aInfo.nFlags |= nFlags;
    SaveStatus_Impl(pChildWin, pCW->aInfo);

    pChildWin->Hide();

    if ( pCW->pCli )
    {
        // Child window is a direct child window and must therefore unregister
        // itself from the  WorkWindow
        pCW->pCli = 0;
        ReleaseChild_Impl(*pChildWin->GetWindow());
    }
    else
    {
        // ChildWindow is within a SplitWindow and unregister itself in
        // the destructor.
    }

    pWorkWin->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( pChildWin->GetWindow() );
    pCW->pWin = 0;
    pChildWin->Destroy();

    GetBindings().Invalidate( nId );
}

void SfxWorkWindow::ResetStatusBar_Impl()
{
    aStatBar.nId = 0;
}

//--------------------------------------------------------------------
void SfxWorkWindow::SetStatusBar_Impl( sal_uInt32 nResId, SfxShell*, SfxBindings& )
{
    if ( nResId && bShowStatusBar && IsVisible_Impl() )
        aStatBar.nId = sal::static_int_cast<sal_uInt16>(nResId);
}

void SfxWorkWindow::UpdateStatusBar_Impl()
{
    Reference< ::com::sun::star::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

    Any aValue = xPropSet->getPropertyValue( m_aLayoutManagerPropName );
    aValue >>= xLayoutManager;

    // No status bar, if no ID is required or when in FullScreenView or
    // if disabled
    if ( aStatBar.nId && IsDockingAllowed() && bInternalDockingAllowed && bShowStatusBar &&
         ( (aStatBar.bOn && !bIsFullScreen) || aStatBar.bTemp ) )
    {
        // Id has changed, thus create a suitable Statusbarmanager, this takes
        // over the  current status bar;
        if ( xLayoutManager.is() )
            xLayoutManager->requestElement( m_aStatusBarResName );
    }
    else
    {
        // Destroy the current StatusBar
        // The Manager only creates the Status bar, does not destroy it.
        if ( xLayoutManager.is() )
            xLayoutManager->destroyElement( m_aStatusBarResName );
    }
}

void SfxWorkWindow::MakeVisible_Impl( sal_Bool bVis )
{
    if ( bVis )
        nOrigMode = SFX_VISIBILITY_STANDARD;
    else
        nOrigMode = SFX_VISIBILITY_UNVISIBLE;

    if ( nOrigMode != nUpdateMode)
        nUpdateMode = nOrigMode;
}

sal_Bool SfxWorkWindow::IsVisible_Impl()
{
    return nOrigMode != SFX_VISIBILITY_UNVISIBLE;
}

//------------------------------------------------------------------------
void SfxWorkWindow::HidePopups_Impl(sal_Bool bHide, sal_Bool bParent, sal_uInt16 nId )
{
    for ( sal_uInt16 n = 0; n < aChildWins.size(); ++n )
    {
        SfxChildWindow *pCW = aChildWins[n]->pWin;
        if (pCW && pCW->GetAlignment() == SFX_ALIGN_NOALIGNMENT && pCW->GetType() != nId)
        {
            Window *pWin = pCW->GetWindow();
            SfxChild_Impl *pChild = FindChild_Impl(*pWin);
            if (bHide)
            {
                pChild->nVisible &= ~CHILD_ACTIVE;
                pCW->Hide();
            }
            else
            {
                pChild->nVisible |= CHILD_ACTIVE;
                if ( CHILD_VISIBLE == (pChild->nVisible & CHILD_VISIBLE) )
                    pCW->Show( SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }
    }

    if ( bParent && pParent )
        pParent->HidePopups_Impl( bHide, bParent, nId );
}

//------------------------------------------------------------------------

void SfxWorkWindow::ConfigChild_Impl(SfxChildIdentifier eChild,
            SfxDockingConfig eConfig, sal_uInt16 nId)
{
    SfxDockingWindow* pDockWin=0;
    sal_uInt16 nPos = USHRT_MAX;
    Window *pWin=0;
    SfxChildWin_Impl *pCW = 0;

    if ( eChild == SFX_CHILDWIN_OBJECTBAR )
    {
        return;
    }
    else
    {
        // configure direct childwindow
        for (sal_uInt16 n=0; n<aChildWins.size(); n++)
        {
            pCW = aChildWins[n];
            SfxChildWindow *pChild = pCW->pWin;
            if ( pChild )
            {
                if ( pChild->GetType() == nId )
                {
                    if ( pChild->GetWindow()->GetType() == RSC_DOCKINGWINDOW )
                        // it's a DockingWindow
                        pDockWin = (SfxDockingWindow*) pChild->GetWindow();
                    else
                        // FloatingWindow or ModelessDialog
                        pWin = pChild->GetWindow();
                    break;
                }
            }
        }

        if ( pDockWin )
        {
            if ( eChild == SFX_CHILDWIN_DOCKINGWINDOW || pDockWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT )
            {
                if ( eChild == SFX_CHILDWIN_SPLITWINDOW && eConfig == SFX_TOGGLEFLOATMODE)
                {
                    // DockingWindow was dragged out of a SplitWindow
                    pCW->pCli = RegisterChild_Impl(*pDockWin, pDockWin->GetAlignment(), pCW->pWin->CanGetFocus());
                    pCW->pCli->nVisible = CHILD_VISIBLE;
                }

                pWin = pDockWin;
            }
            else
            {
                SfxSplitWindow *pSplitWin = GetSplitWindow_Impl(pDockWin->GetAlignment());

                // configure DockingWindow inside a SplitWindow
                if ( eConfig == SFX_TOGGLEFLOATMODE)
                {
                    // DockingWindow was dragged into a SplitWindow
                    pCW->pCli = 0;
                    ReleaseChild_Impl(*pDockWin);
                }

                pWin = pSplitWin->GetSplitWindow();
                if ( pSplitWin->GetWindowCount() == 1 )
                    ((SplitWindow*)pWin)->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }

        DBG_ASSERT( pCW, "Unknown window!" );
        if ( !pCW && pParent )
        {
            pParent->ConfigChild_Impl( eChild, eConfig, nId );
            return;
        }
    }

    if ( !bSorted )
        // windows may have been registered and released without an update until now
        Sort_Impl();

    SfxChild_Impl *pChild = 0;
    sal_uInt16 n;
    for ( n=0; n<aSortedList.size(); ++n )
    {
        pChild = aChildren[aSortedList[n]];
        if ( pChild )
            if ( pChild->pWin == pWin )
            break;
    }

    if ( n < aSortedList.size() )
        // sometimes called while toggeling float mode
        nPos = aSortedList[n];

    switch ( eConfig )
    {
        case SFX_SETDOCKINGRECTS :
        {
            if ( nPos == USHRT_MAX )
                return;

            Rectangle aOuterRect( GetTopRect_Impl() );
            aOuterRect.SetPos( pWorkWin->OutputToScreenPixel( aOuterRect.TopLeft() ));
            Rectangle aInnerRect( aOuterRect );
            sal_Bool bTbx = (eChild == SFX_CHILDWIN_OBJECTBAR);

            // The current affected window is included in the calculation of
            // the inner rectangle!
            for ( sal_uInt16 m=0; m<aSortedList.size(); ++m )
            {
                sal_uInt16 i=aSortedList[m];
                SfxChild_Impl* pCli = aChildren[i];

                if ( pCli && pCli->nVisible == CHILD_VISIBLE && pCli->pWin )
                {
                    switch ( pCli->eAlign )
                    {
                        case SFX_ALIGN_TOP:
                            // Objekt-Toolboxes come always last
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXTOP:
                            // Toolbox has priority, if no higher Position
                            if ( bTbx && i <= nPos)
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTTOP:
                            // Always performed first
                            aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTTOP:
                            // Is only counted if it is the current window
                            if ( i == nPos )
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_BOTTOM:
                            // Objekt-Toolboxes come always last
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXBOTTOM:
                            // Toolbox has priority, if no higher Position
                            if ( bTbx && i <= nPos)
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTBOTTOM:
                            // Always performed first
                            aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTBOTTOM:
                            // Is only counted if it is the current window
                            if ( i == nPos )
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LEFT:
                            // Toolboxes come always last
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXLEFT:
                            // Toolboxes come always last
                            if (bTbx && i <= nPos)
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTLEFT:
                            // Always performed first
                            aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTLEFT:
                            // Is only counted if it is the current window
                            if (i == nPos)
                                aInnerRect.Left() += pCli->aSize.Width();

                        case SFX_ALIGN_RIGHT:
                            // Toolboxes come always last
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXRIGHT:
                            // Toolboxes come always last
                            if (bTbx && i <= nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTRIGHT:
                            // Is only counted if it is the current window
                            if (i == nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTRIGHT:
                            // Always performed first
                            aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        default:
                            break;
                    }
                }
            }

            pDockWin->SetDockingRects(aOuterRect, aInnerRect);
            break;
        }

        case SFX_MOVEDOCKINGWINDOW :
        case SFX_ALIGNDOCKINGWINDOW :
        case SFX_TOGGLEFLOATMODE:
        {
            if ( nPos == USHRT_MAX && !pCW )
                return;

            SfxChildAlignment eAlign = SFX_ALIGN_NOALIGNMENT;
            SfxChild_Impl *pCli = ( nPos != USHRT_MAX ) ? aChildren[nPos] : 0;
            if ( pCli && pDockWin )
            {
                eAlign = pDockWin->GetAlignment();
                if ( eChild == SFX_CHILDWIN_DOCKINGWINDOW || eAlign == SFX_ALIGN_NOALIGNMENT)
                {
                    // configuration inside the SplitWindow, no change for the SplitWindows' configuration
                    pCli->bResize = sal_True;
                    pCli->aSize = pDockWin->GetSizePixel();
                }
            }

            if ( pCli )
            {
                if( pCli->eAlign != eAlign )
                {
                    bSorted = sal_False;
                    pCli->eAlign = eAlign;
                }

                ArrangeChildren_Impl();
                ShowChildren_Impl();
            }

            if ( pCW && pCW->pWin )
            {
                // store changed configuration
                sal_uInt16 nFlags = pCW->aInfo.nFlags;
                pCW->aInfo = pCW->pWin->GetInfo();
                pCW->aInfo.nFlags |= nFlags;
                if ( eConfig != SFX_MOVEDOCKINGWINDOW )
                    SaveStatus_Impl( pCW->pWin, pCW->aInfo);
            }

            break;
        }
    }
}

//--------------------------------------------------------------------

void SfxWorkWindow::SetChildWindowVisible_Impl( sal_uInt32 lId, sal_Bool bEnabled, sal_uInt16 nMode )
{
    sal_uInt16 nInter = (sal_uInt16) ( lId >> 16 );
    sal_uInt16 nId = (sal_uInt16) ( lId & 0xFFFF );

    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are always registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        sal_uInt16 nCount = pWork->aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (pWork->aChildWins[n]->nSaveId == nId)
            {
                pCW = pWork->aChildWins[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n];
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
        if ( pWork && !( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) )
            pWork->aChildWins.push_back( pCW );
        else
            aChildWins.push_back( pCW );
    }

    pCW->nId = nId;
    if ( nInter )
        pCW->nInterfaceId = nInter;
    pCW->nVisibility = nMode;
    pCW->bEnable = bEnabled;
    pCW->nVisibility = nMode;
}

//--------------------------------------------------------------------
// The on/of-Status of a ChildWindows is switched

void SfxWorkWindow::ToggleChildWindow_Impl(sal_uInt16 nId, sal_Bool bSetFocus)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nId == nId)
            break;

    if ( n<nCount )
    {
        // The Window is aleady known
        SfxChildWin_Impl *pCW = aChildWins[n];
        SfxChildWindow *pChild = pCW->pWin;

        bool bCreationAllowed( true );
        if ( !bInternalDockingAllowed )
        {
            // Special case for all non-floatable child windows. We have
            // to prevent the creation here!
            bCreationAllowed = !( pCW->aInfo.nFlags & SFX_CHILDWIN_FORCEDOCK );
        }

        if ( bCreationAllowed )
        {
            if ( pCW->bCreate )
            {
                if ( pChild )
                {
                    if ( pChild->QueryClose() )
                    {
                        pCW->bCreate = sal_False;
                        if ( pChild->IsHideAtToggle() )
                        {
                            ShowChildWindow_Impl( nId, sal_False, bSetFocus );
                        }
                        else
                        {
                            // The Window should be switched off
                            pChild->SetVisible_Impl( sal_False );
                            RemoveChildWin_Impl( pCW );
                        }
                    }
                }
                else
                {
                    // no actual Window exists, yet => just remember the "switched off" state
                    pCW->bCreate = sal_False;
                }
            }
            else
            {
                pCW->bCreate = AllowChildWindowCreation_Impl( *pCW );
                if ( pCW->bCreate )
                {
                    if ( pChild )
                    {
                        ShowChildWindow_Impl( nId, sal_True, bSetFocus );
                    }
                    else
                    {
                        // create actual Window
                        CreateChildWin_Impl( pCW, bSetFocus );
                        if ( !pCW->pWin )
                            // no success
                            pCW->bCreate = sal_False;
                    }
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
                    (SfxDockingWindow*) pCW->pWin->GetWindow();
                if ( pDock->IsAutoHide_Impl() )
                    pDock->AutoShow_Impl();
            }
        }

        return;
    }
    else if ( pParent )
    {
        pParent->ToggleChildWindow_Impl( nId, bSetFocus );
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

//--------------------------------------------------------------------

sal_Bool SfxWorkWindow::HasChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
            break;

    if (n<nCount)
    {
        SfxChildWin_Impl *pCW = aChildWins[n];
        SfxChildWindow *pChild = pCW->pWin;
        return ( pChild && pCW->bCreate );
    }

    if ( pParent )
        return pParent->HasChildWindow_Impl( nId );

    return sal_False;
}

sal_Bool SfxWorkWindow::IsFloating( sal_uInt16 nId )
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are always registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        sal_uInt16 nCount = pWork->aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (pWork->aChildWins[n]->nSaveId == nId)
            {
                pCW = pWork->aChildWins[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If new, then initialize, add this here depending on the flag or
        // the Parent
        pCW = new SfxChildWin_Impl( nId );
        pCW->bEnable = sal_False;
        pCW->nId = 0;
        pCW->nVisibility = 0;
        InitializeChild_Impl( pCW );
        if ( pWork && !( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) )
            pWork->aChildWins.push_back( pCW );
        else
            aChildWins.push_back( pCW );
    }

    SfxChildAlignment eAlign;
    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
        return( eAlign == SFX_ALIGN_NOALIGNMENT );
    else
        return sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxWorkWindow::KnowsChildWindow_Impl(sal_uInt16 nId)
{
    SfxChildWin_Impl *pCW=0;
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
    {
        pCW = aChildWins[n];
        if ( pCW->nSaveId == nId)
             break;
    }

    if (n<nCount)
    {
        if ( !(pCW->aInfo.nFlags & SFX_CHILDWIN_ALWAYSAVAILABLE) && !IsVisible_Impl(  pCW->nVisibility ) )
            return sal_False;
        return pCW->bEnable;
    }
    else if ( pParent )
        return pParent->KnowsChildWindow_Impl( nId );
    else
        return sal_False;
}

//--------------------------------------------------------------------

void SfxWorkWindow::SetChildWindow_Impl(sal_uInt16 nId, sal_Bool bOn, sal_Bool bSetFocus)
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are always registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        sal_uInt16 nCount = pWork->aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (pWork->aChildWins[n]->nSaveId == nId)
            {
                pCW = pWork->aChildWins[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        sal_uInt16 nCount = aChildWins.size();
        for (sal_uInt16 n=0; n<nCount; n++)
            if (aChildWins[n]->nSaveId == nId)
            {
                pCW = aChildWins[n];
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
        if ( !pWork || pCW->aInfo.nFlags & SFX_CHILDWIN_TASK )
            pWork = this;
        pWork->aChildWins.push_back( pCW );
    }

    if ( pCW->bCreate != bOn )
        pWork->ToggleChildWindow_Impl(nId,bSetFocus);
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChildWindow_Impl(sal_uInt16 nId, sal_Bool bVisible, sal_Bool bSetFocus)
{
    sal_uInt16 nCount = aChildWins.size();
    SfxChildWin_Impl* pCW=0;
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
    {
        pCW = aChildWins[n];
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
                    pCW->pCli->nVisible = CHILD_VISIBLE;
                    pChildWin->Show( bSetFocus && pChildWin->WantsFocus() ? 0 : SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Reappear_Impl();

            }
            else
            {
                if ( pCW->pCli )
                {
                    pCW->pCli->nVisible = CHILD_VISIBLE ^ CHILD_NOT_HIDDEN;
                    pCW->pWin->Hide();
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Disappear_Impl();

            }

            ArrangeChildren_Impl();
            ShowChildren_Impl();
        }
        else if ( bVisible )
        {
            SetChildWindow_Impl( nId, sal_True, bSetFocus );
            pChildWin = pCW->pWin;
        }

        if ( pChildWin )
        {
            pChildWin->SetVisible_Impl( bVisible );
            sal_uInt16 nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChildWin->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            if ( !pCW->bCreate )
                SaveStatus_Impl( pChildWin, pCW->aInfo );
        }

        return;
    }

    if ( pParent )
    {
        pParent->ShowChildWindow_Impl( nId, bVisible, bSetFocus );
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

//--------------------------------------------------------------------

SfxChildWindow* SfxWorkWindow::GetChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = aChildWins.size();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if (aChildWins[n]->nSaveId == nId)
             break;

    if (n<nCount)
        return aChildWins[n]->pWin;
    else if ( pParent )
        return pParent->GetChildWindow_Impl( nId );
    return 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::ResetChildWindows_Impl()
{
    for ( sal_uInt16 n = 0; n < aChildWins.size(); ++n )
    {
        aChildWins[n]->nId = 0;
        aChildWins[n]->bEnable = sal_False;
    }
}

//------------------------------------------------------------------------
// Virtual method that returns the size of the area (client area) of the
// parent windows, in which the ChildWindow can be fitted.

Rectangle SfxWorkWindow::GetTopRect_Impl()
{
    return Rectangle (Point(), pWorkWin->GetOutputSizePixel() );
}

//------------------------------------------------------------------------
// Virtual method that returns the size of the area (client area) of the
// parent windows, in which the ChildWindow can be fitted.

Rectangle SfxFrameWorkWin_Impl::GetTopRect_Impl()
{
    return pMasterFrame->GetTopOuterRectPixel_Impl();
}

//------------------------------------------------------------------------
// Virtual method to find out if there is room for a ChildWindow in the
// client area of the parent.

sal_Bool SfxWorkWindow::RequestTopToolSpacePixel_Impl( SvBorder aBorder )
{
    if ( !IsDockingAllowed() ||
            aClientArea.GetWidth() < aBorder.Left() + aBorder.Right() ||
            aClientArea.GetHeight() < aBorder.Top() + aBorder.Bottom() )
        return sal_False;
    else
        return sal_True;;
}

void SfxWorkWindow::SaveStatus_Impl(SfxChildWindow *pChild, const SfxChildWinInfo &rInfo)
{
    // The Status of the Presentation mode is not saved
    if ( IsDockingAllowed() && bInternalDockingAllowed )
        pChild->SaveStatus(rInfo);
}

void SfxWorkWindow::InitializeChild_Impl(SfxChildWin_Impl *pCW)
{
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SFX_APP();
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = rFactories[nFactory];
            if ( pFact->nId == pCW->nSaveId )
            {
                pCW->aInfo   = pFact->aInfo;
                SfxChildWindow::InitializeChildWinFactory_Impl(
                                            pCW->nSaveId, pCW->aInfo);
                pCW->bCreate = pCW->aInfo.bVisible;
                sal_uInt16 nFlags = pFact->aInfo.nFlags;
                if ( nFlags & SFX_CHILDWIN_TASK )
                    pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
                if ( nFlags & SFX_CHILDWIN_CANTGETFOCUS )
                    pCW->aInfo.nFlags |= SFX_CHILDWIN_CANTGETFOCUS;
                if ( nFlags & SFX_CHILDWIN_FORCEDOCK )
                    pCW->aInfo.nFlags |= SFX_CHILDWIN_FORCEDOCK;
                pFact->aInfo = pCW->aInfo;
                return;
            }
        }
    }

    SfxDispatcher *pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
    if ( pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == pCW->nSaveId )
                {
                    pCW->aInfo   = pFact->aInfo;
                    SfxChildWindow::InitializeChildWinFactory_Impl(
                                                pCW->nSaveId, pCW->aInfo);
                    pCW->bCreate = pCW->aInfo.bVisible;
                    sal_uInt16 nFlags = pFact->aInfo.nFlags;
                    if ( nFlags & SFX_CHILDWIN_TASK )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
                    if ( nFlags & SFX_CHILDWIN_CANTGETFOCUS )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_CANTGETFOCUS;
                    if ( nFlags & SFX_CHILDWIN_FORCEDOCK )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_FORCEDOCK;
                    if ( nFlags & SFX_CHILDWIN_ALWAYSAVAILABLE )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_ALWAYSAVAILABLE;
                    pFact->aInfo = pCW->aInfo;
                    return;
                }
            }
        }
    }
}

SfxSplitWindow* SfxWorkWindow::GetSplitWindow_Impl( SfxChildAlignment eAlign )
{
    switch ( eAlign )
    {
        case SFX_ALIGN_TOP:
            return pSplit[2];

        case SFX_ALIGN_BOTTOM:
            return pSplit[3];

        case SFX_ALIGN_LEFT:
            return pSplit[0];

        case SFX_ALIGN_RIGHT:
            return pSplit[1];

        default:
            return 0;
    }
}

void SfxWorkWindow::MakeChildrenVisible_Impl( sal_Bool bVis )
{
    if ( pParent )
        pParent->MakeChildrenVisible_Impl( bVis );

    bAllChildrenVisible = bVis;
    if ( bVis )
    {
        if ( !bSorted )
            Sort_Impl();
        for ( sal_uInt16 n=0; n<aSortedList.size(); ++n )
        {
            SfxChild_Impl* pCli = aChildren[aSortedList[n]];
            if ( (pCli->eAlign == SFX_ALIGN_NOALIGNMENT) || (IsDockingAllowed() && bInternalDockingAllowed) )
                pCli->nVisible |= CHILD_ACTIVE;
        }
    }
    else
    {
        if ( !bSorted )
            Sort_Impl();
        for ( sal_uInt16 n=0; n<aSortedList.size(); ++n )
        {
            SfxChild_Impl* pCli = aChildren[aSortedList[n]];
            pCli->nVisible &= ~CHILD_ACTIVE;
        }
    }
}

sal_Bool SfxWorkWindow::IsAutoHideMode( const SfxSplitWindow *pSplitWin )
{
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        if ( pSplit[n] != pSplitWin && pSplit[n]->IsAutoHide( sal_True ) )
            return sal_True;
    }
    return sal_False;
}


void SfxWorkWindow::EndAutoShow_Impl( Point aPos )
{
    if ( pParent )
        pParent->EndAutoShow_Impl( aPos );

    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if ( p && p->IsAutoHide() )
        {
            Point aLocalPos = p->ScreenToOutputPixel( aPos );
        Point aEmptyPoint = Point();
            Rectangle aRect( aEmptyPoint, p->GetSizePixel() );
            if ( !aRect.IsInside( aLocalPos ) )
                p->FadeOut();
        }
    }
}

void SfxWorkWindow::ArrangeAutoHideWindows( SfxSplitWindow *pActSplitWin )
{
    if ( m_nLock )
        return;

    if ( pParent )
        pParent->ArrangeAutoHideWindows( pActSplitWin );

    Rectangle aArea( aUpperClientArea );
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Either dummy window or window in the auto-show-mode are processed
        // (not pinned, FadeIn).
        // Only the abandoned window may be invisible, because perhaps its
        // size is just beeing calculated before it is displayed.
        SfxSplitWindow* pSplitWin = pSplit[n];
        sal_Bool bDummyWindow = !pSplitWin->IsFadeIn();
        Window *pDummy = pSplitWin->GetSplitWindow();
        Window *pWin = bDummyWindow ? pDummy : pSplitWin;
        if ( (pSplitWin->IsPinned() && !bDummyWindow) || (!pWin->IsVisible() && pActSplitWin != pSplitWin) )
            continue;

        // Width and position of the dummy window as a starting point
        Size aSize = pDummy->GetSizePixel();
        Point aPos = pDummy->GetPosPixel();

        switch ( n )
        {
            case ( 0 ) :
            {
                // Left SplitWindow
                // Get the width of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                // If a Window is visable to the left, then the free region
                // starts to the right from it, for example at the Client area
                long nLeft = aPos.X() + aSize.Width();
                if ( nLeft > aArea.Left() )
                    aArea.Left() = nLeft;
                break;
            }
            case ( 1 ) :
            {
                // Right SplitWindow
                // Position to correct the difference of the widths
                aPos.X() += aSize.Width();

                // Get the width of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                aPos.X() -= aSize.Width();

                // If already a window is opened at the left side, then the
                // right is not allowed to overlap this one.
                if ( aPos.X() < aArea.Left() )
                {
                    aPos.X() = aArea.Left();
                    aSize.Width() = aArea.GetWidth();
                }

                // If a Window is visable to the right, then the free region
                // starts to the left from it, for example at the Client area
                long nRight = aPos.X();
                if ( nRight < aArea.Right() )
                    aArea.Right() = nRight;
                break;
            }
            case ( 2 ) :
            {
                // Top SplitWindow
                // Get the height of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();


                // Adjust width with regard to if a Window is already open
                // to the left or right
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // If a Window is visable at the top, then the free region
                // starts beneath it, for example at the Client area
                long nTop = aPos.Y() + aSize.Height();
                if ( nTop > aArea.Top() )
                    aArea.Top() = nTop;
                break;
            }
            case ( 3 ) :
            {
                // The bottom SplitWindow
                // Position to correct the difference of the heights
                aPos.Y() += aSize.Height();

                // Get the height of the Window yourself, if no DummyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();

                aPos.Y() -= aSize.Height();

                // Adjust width with regard to if a Window is already open
                // to the left or right.
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // If already a window is opened at the top, then the
                // bottom one is not allowed to overlap this one.
                if ( aPos.Y() < aArea.Top() )
                {
                    aPos.Y() = aArea.Top();
                    aSize.Height() = aArea.GetHeight();
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

Rectangle SfxWorkWindow::GetFreeArea( sal_Bool bAutoHide ) const
{
    if ( bAutoHide )
    {
        Rectangle aArea( aClientArea );
        for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
        {
            if ( pSplit[n]->IsPinned() || !pSplit[n]->IsVisible() )
                continue;

            Size aSize = pSplit[n]->GetSizePixel();
            switch ( n )
            {
                case ( 0 ) :
                    aArea.Left() += aSize.Width();
                    break;
                case ( 1 ) :
                    aArea.Right() -= aSize.Width();
                    break;
                case ( 2 ) :
                    aArea.Top() += aSize.Height();
                    break;
                case ( 3 ) :
                    aArea.Bottom() -= aSize.Height();
                    break;
            }
        }

        return aArea;
    }
    else
        return aClientArea;
}

void SfxWorkWindow::SetActiveChild_Impl( Window *pChild )
{
    pActiveChild = pChild;
}

sal_Bool SfxWorkWindow::ActivateNextChild_Impl( sal_Bool bForward )
{
    // Sort all children under list
    std::vector<sal_uInt16> aList;
    for ( sal_uInt16 i=SFX_OBJECTBAR_MAX; i<aChildren.size(); i++)
    {
        SfxChild_Impl *pCli = aChildren[i];
        if ( pCli && pCli->bCanGetFocus && pCli->pWin )
        {
            sal_uInt16 k;
            for (k=0; k<aList.size(); k++)
                if ( ChildTravelValue( aChildren[aList[k]]->eAlign) > ChildTravelValue(pCli->eAlign) )
                    break;
            aList.insert( aList.begin() + k, i );
        }
    }

    if ( aList.empty() )
        return sal_False;

    sal_uInt16 nTopValue  = ChildTravelValue( SFX_ALIGN_LOWESTTOP );
    for ( sal_uInt16 i=0; i<aList.size(); i++ )
    {
        SfxChild_Impl* pCli = aChildren[aList[i]];
        if ( pCli->pWin && ChildTravelValue( pCli->eAlign ) > nTopValue )
            break;
    }

    sal_uInt16 n = bForward ? 0 : aList.size()-1;
    SfxChild_Impl *pAct=NULL;
    if ( pActiveChild )
    {
        // Look for the active window
        for ( n=0; n<aList.size(); n++ )
        {
            SfxChild_Impl* pCli = aChildren[aList[n]];
            if ( pCli && pCli->pWin && ( pCli->pWin == pActiveChild || !pActiveChild ) )
            {
                pAct = pCli;
                break;
            }
        }
    }

    // dummy entries for the container window
    aList.insert( aList.begin(), 0xFFFF );
    aList.push_back( 0xFFFF );
    n = n + 1;
    if ( pAct )
    {
        for ( sal_uInt16 i=0; i<SFX_SPLITWINDOWS_MAX; i++ )
        {
            // Maybe the pNext is a Splitwindow
            SfxSplitWindow *p = pSplit[i];
            if ( pAct->pWin == p )
            {
                if( p->ActivateNextChild_Impl( bForward ) )
                    return sal_True;
                break;
            }
        }

        // pAct is a direct ChildWindow
        // continue with the successor or predecessor of the active window
        if ( bForward )
            n = n+1;
        else
            n = n-1;

        if ( n == 0 || n == aList.size()-1 )
            return sal_False;
    }

    for( ;; )
    {
        SfxChild_Impl* pCli = aChildren[aList[n]];
        if ( pCli->pWin )
        {
            SfxChild_Impl* pNext = pCli;
            for ( sal_uInt16 i=0; n<SFX_SPLITWINDOWS_MAX; n++ )
            {
                // Maybe the pNext is a Splitwindow
                SfxSplitWindow *p = pSplit[i];
                if ( pNext->pWin == p )
                {
                    // Activate the first/last window
                    p->SetActiveWindow_Impl( NULL );
                    pNext = NULL;
                    if( p->ActivateNextChild_Impl( bForward ) )
                        return sal_True;
                    break;
                }
            }

            if ( pNext )
            {
                pNext->pWin->GrabFocus();
                pActiveChild = pNext->pWin;
                return sal_True;
            }
        }

        if ( bForward )
            n = n+1;
        else
            n = n-1;

        if ( n == 0 || n == aList.size()-1 )
            break;
    }

    return sal_False;
}

void SfxWorkWindow::DataChanged_Impl( const DataChangedEvent& )
{
    sal_uInt16 n;
    sal_uInt16 nCount = aChildWins.size();
    for (n=0; n<nCount; n++)
    {
        SfxChildWin_Impl*pCW = aChildWins[n];
        if ( pCW && pCW->pWin )
            pCW->pWin->GetWindow()->UpdateSettings( Application::GetSettings() );
    }

    ArrangeChildren_Impl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
