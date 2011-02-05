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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <stdio.h>
#include <boost/unordered_map.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include "workwin.hxx"
#include <sfx2/viewfrm.hxx>
#include "arrdecl.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include "splitwin.hxx"
#include <sfx2/msgpool.hxx>
#include "sfxresid.hxx"
#include <sfx2/objsh.hxx>
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
namespace css = ::com::sun::star;

struct ResIdToResName
{
    USHORT      nId;
    const char* pName;
};

static const ResIdToResName pToolBarResToName[] =
{
    { 558,      "fullscreenbar"        },
    { 560,      "standardbar",         },
    { 18001,    "formsnavigationbar"   },
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


SFX_IMPL_XINTERFACE_3( LayoutManagerListener, OWeakObject, ::com::sun::star::frame::XLayoutManagerListener, ::com::sun::star::lang::XEventListener, ::com::sun::star::lang::XComponent )
SFX_IMPL_XTYPEPROVIDER_3( LayoutManagerListener, ::com::sun::star::frame::XLayoutManagerListener, ::com::sun::star::lang::XEventListener, ::com::sun::star::lang::XComponent )

LayoutManagerListener::LayoutManagerListener(
    SfxWorkWindow* pWrkWin ) :
    m_bHasFrame( sal_False ),
    m_pWrkWin( pWrkWin ),
    m_aLayoutManagerPropName( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))
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
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LockCount" )) );
                        aValue >>= m_pWrkWin->m_nLock;
                    }
                }
                catch ( css::lang::DisposedException& )
                {
                }
                catch ( css::uno::RuntimeException& e )
                {
                    throw e;
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
            catch ( css::uno::RuntimeException& e )
            {
                throw e;
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
            m_pWrkWin->MakeVisible_Impl( TRUE );
            m_pWrkWin->ShowChilds_Impl();
            m_pWrkWin->ArrangeChilds_Impl( TRUE );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::INVISIBLE )
        {
            m_pWrkWin->MakeVisible_Impl( FALSE );
            m_pWrkWin->HideChilds_Impl();
            m_pWrkWin->ArrangeChilds_Impl( TRUE );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::LOCK )
        {
            m_pWrkWin->Lock_Impl( TRUE );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::UNLOCK )
        {
            m_pWrkWin->Lock_Impl( FALSE );
        }
    }
}

//====================================================================

typedef boost::unordered_map< sal_Int32, rtl::OUString > ToolBarResIdToResourceURLMap;

static sal_Bool bMapInitialized = sal_False;
static ToolBarResIdToResourceURLMap aResIdToResourceURLMap;

static rtl::OUString GetResourceURLFromResId( USHORT nResId )
{
    if ( !bMapInitialized )
    {
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        if ( !bMapInitialized )
        {
            sal_Int32 nIndex( 0 );
            while ( pToolBarResToName[nIndex].nId != 0 )
            {
                rtl::OUString aResourceURL( rtl::OUString::createFromAscii( pToolBarResToName[nIndex].pName ));
                aResIdToResourceURLMap.insert( ToolBarResIdToResourceURLMap::value_type(
                                                    sal_Int32( pToolBarResToName[nIndex].nId ), aResourceURL ));
                ++nIndex;
            }
            bMapInitialized = sal_True;
        }
    }

    ToolBarResIdToResourceURLMap::const_iterator pIter = aResIdToResourceURLMap.find( nResId );
    if ( pIter != aResIdToResourceURLMap.end() )
        return pIter->second;
    else
        return rtl::OUString();
}

BOOL IsAppWorkWinToolbox_Impl( USHORT nPos )
{
    switch ( nPos )
    {
        case SFX_OBJECTBAR_APPLICATION :
        case SFX_OBJECTBAR_MACRO:
        case SFX_OBJECTBAR_FULLSCREEN:
            return TRUE;
        default:
            return FALSE;
    }
}

USHORT TbxMatch( USHORT nPos )
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

USHORT ChildAlignValue(SfxChildAlignment eAlign)
{
    USHORT ret = 17;

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

USHORT ChildTravelValue( SfxChildAlignment eAlign )
{
    USHORT ret = 17;

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
    aSortedList.Remove(0, aSortedList.Count());
    for (USHORT i=0; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if (pCli)
        {
            USHORT k;
            for (k=0; k<aSortedList.Count(); k++)
//                              if ( (*pChilds)[aSortedList[k]]->eAlign > pCli->eAlign )
                if (ChildAlignValue((*pChilds)[aSortedList[k]]->eAlign) >
                    ChildAlignValue(pCli->eAlign))
                    break;
            aSortedList.Insert (i,k);
        }
    }

    bSorted = TRUE;
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
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
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

    //nOrigMode = SFX_VISIBILITY_CLIENT;
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
    nChilds( 0 ),
    nOrigMode( 0 ),
    bSorted( TRUE ),
    bDockingAllowed(TRUE),
    bInternalDockingAllowed(TRUE),
    bAllChildsVisible(TRUE),
    bIsFullScreen( FALSE ),
    bShowStatusBar( TRUE ),
    m_nLock( 0 ),
    m_aStatusBarResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" )),
    m_aLayoutManagerPropName( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )),
    m_aTbxTypeName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" )),
    m_aProgressBarResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/progressbar/progressbar" ))
{
    DBG_CTOR(SfxWorkWindow, 0);
    DBG_ASSERT (pBindings, "Keine Bindings!");

    pBindings->SetWorkWindow_Impl( this );

    pChildWins = new SfxChildWindows_Impl;
    pChilds = new SfxChildList_Impl;

    // For the ObjectBars a integral place in the Childlist is reserved,
    // so that they always come in a defined order.
    SfxChild_Impl* pChild=0;
    for (USHORT n=0; n < SFX_OBJECTBAR_MAX; ++n)
        pChilds->Insert(0,pChild);

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
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            ReleaseChild_Impl(*p);
        delete p;
    }

    // Delete help structure for Child-Windows
    DBG_ASSERT( pChilds->Count() == 0, "dangling childs" );
    delete pChilds;
    delete pChildWins;

    if ( m_xLayoutManagerListener.is() )
        m_xLayoutManagerListener->dispose();
}

SystemWindow* SfxWorkWindow::GetTopWindow() const
{
    Window* pRet = pWorkWin;
    while ( pRet && !pRet->IsSystemWindow() )
        pRet = pRet->GetParent();
    return (SystemWindow*) pRet;
}

void SfxWorkWindow::Lock_Impl( BOOL bLock )
{
    if ( bLock )
        m_nLock++;
    else
        --m_nLock;
    if ( m_nLock<0 )
    {
        DBG_ERROR("Lock count underflow!");
        m_nLock = 0;
    }

    if ( !m_nLock )
        ArrangeChilds_Impl();
}

void SfxWorkWindow::ChangeWindow_Impl( Window *pNew )
{
    Window *pOld = pWorkWin;
    pWorkWin = pNew;
    for ( USHORT nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        SfxChild_Impl *pCli = (*pChilds)[nPos];
        if ( pCli && pCli->pWin && pCli->pWin->GetParent() == pOld )
        {
            pCli->pWin->SetParent( pNew );
        }
    }
}

void SfxWorkWindow::SaveStatus_Impl()
{
    USHORT nCount = pChildWins->Count();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxChildWin_Impl* pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            USHORT nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
        }
    }
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
    USHORT n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
           if (p->GetWindowCount())
        p->Lock();
    }

    // Delete Child-Windows
    for ( n=0; n<pChildWins->Count(); )
    {
        SfxChildWin_Impl* pCW = (*pChildWins)[n];
        pChildWins->Remove(n);
           SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
/*
            USHORT nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
*/
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
        // See task 128307 (Windows)
        // Window::NotifyAllChilds() calls SfxWorkWindow::DataChanged_Impl for
        // 8-bit displays (WM_QUERYPALETTECHANGED message due to focus change)!!
        //(*pChildWins)[n] = 0;
    }

    //pChildWins->Remove((USHORT)0, nCount);

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

        // Delete ObjectBars (this is done last, so that pChilds does not
        // receive dead Pointers)
        for ( USHORT i = 0; i < aObjBarList.size(); i++ )
        {
            // Not every position must be occupied
            USHORT nId = aObjBarList[i].nId;
            if ( nId )
                aObjBarList[i].nId = 0;
        }
    }

    // ObjectBars are all released at once, since they occupy a
    // fixed contiguous area in the array pChild
    pChilds->Remove(0, SFX_OBJECTBAR_MAX);
    bSorted = FALSE;

    nChilds = 0;
}

//====================================================================
// Virtual method for placing the child window.

void SfxWorkWindow::ArrangeChilds_Impl( BOOL /*bForce*/)
{
    Arrange_Impl();
}

void SfxFrameWorkWin_Impl::ArrangeChilds_Impl( BOOL bForce )
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
    if ( nChilds )
    {
        if ( IsVisible_Impl() )
            aBorder = Arrange_Impl();
    }
    // If the current application document contains a IPClient, then the
    // object through SetTopToolFramePixel has to be assigned the available
    // space. The object will then point to its UITools and sets the app border
    // (-> SfxInPlaceEnv_Impl:: ArrangeChilds_Impl ()). Otherwise the
    // app border is set here directly to possibly overwrite the Border that
    // was set by an object from another document.  The object does not set
    // the SetAppBorder when it removes its UI tools so that no-dithering
    // ObjectBar arises.
    // (->SfxInPlaceEnv_Impl::ArrangeChilds_Impl())

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
    if ( !nChilds )
        return aBorder;

    if (!bSorted)
        Sort_Impl();

    Point aPos;
    Size aSize;
    Rectangle aTmp( aClientArea );

    for ( USHORT n=0; n<aSortedList.Count(); ++n )
    {
        SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
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
        BOOL bAllowHiding = TRUE;
        switch ( pCli->eAlign )
        {
            case SFX_ALIGN_HIGHESTTOP:
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_TOOLBOXTOP:
            case SFX_ALIGN_LOWESTTOP:
                aSize.Width() = aTmp.GetWidth();
                if ( pCli->pWin->GetType() == WINDOW_SPLITWINDOW )
                    aSize = ((SplitWindow *)(pCli->pWin))->CalcLayoutSizePixel( aSize );
                bAllowHiding = FALSE;
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
                bAllowHiding = FALSE;
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
                pCli->bResize = FALSE;
                continue;
        }

        pCli->pWin->SetPosSizePixel( aPos, aSize );
        pCli->bResize = FALSE;
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

//--------------------------------------------------------------------
// Close-Handler: The Configuration of the ChildWindows is saved.
//

void SfxWorkWindow::Close_Impl()
{
    for (USHORT n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            USHORT nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
        }
    }
}

BOOL SfxWorkWindow::PrepareClose_Impl()
{
    for (USHORT n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if ( pChild && !pChild->QueryClose() )
            return FALSE;
    }

    return TRUE;
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::RegisterChild_Impl( Window& rWindow,
                    SfxChildAlignment eAlign, BOOL bCanGetFocus )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
    DBG_ASSERT( pChilds->Count() < 255, "too many childs" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );
    DBG_ASSERT( !FindChild_Impl(rWindow), "child registered more than once" );


    if ( rWindow.GetParent() != pWorkWin )
        rWindow.SetParent( pWorkWin );

    SfxChild_Impl *pChild = new SfxChild_Impl(rWindow, rWindow.GetSizePixel(),
                                    eAlign, rWindow.IsVisible());
    pChild->bCanGetFocus = bCanGetFocus;

    pChilds->Insert(pChilds->Count(), pChild);
    bSorted = FALSE;
    nChilds++;
    return (*pChilds)[pChilds->Count()-1];
}

//--------------------------------------------------------------------

void SfxWorkWindow::AlignChild_Impl( Window& rWindow,
                                            const Size& rNewSize,
                                            SfxChildAlignment eAlign )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
//  DBG_ASSERT( pChilds, "aligning unregistered child" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );

    SfxChild_Impl *pChild = FindChild_Impl(rWindow);
    if ( pChild )
    {
        if (pChild->eAlign != eAlign)
            bSorted = FALSE;

        pChild->eAlign = eAlign;
        pChild->aSize = rNewSize;
        pChild->bResize = TRUE;
    }
    else {
        DBG_ERROR( "aligning unregistered child" );
    }
}

//--------------------------------------------------------------------

void SfxWorkWindow::ReleaseChild_Impl( Window& rWindow )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
//  DBG_ASSERT( pChilds, "releasing unregistered child" );

    SfxChild_Impl *pChild = 0;
    USHORT nPos;
    for ( nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        pChild = (*pChilds)[nPos];
        if ( pChild )
          if ( pChild->pWin == &rWindow )
            break;
    }

    if ( nPos < pChilds->Count() )
    {
        bSorted = FALSE;
        nChilds--;
        pChilds->Remove(nPos);
        delete pChild;
    }
    else {
        DBG_ERROR( "releasing unregistered child" );
    }
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::FindChild_Impl( const Window& rWindow ) const
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    SfxChild_Impl *pChild = 0;
    USHORT nCount = pChilds->Count();
    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
    {
        pChild = (*pChilds)[nPos];
        if ( pChild )
          if ( pChild->pWin == &rWindow )
            return pChild;
    }

    return 0;
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChilds_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    bool bInvisible = ( !IsVisible_Impl() || ( !pWorkWin->IsReallyVisible() && !pWorkWin->IsReallyShown() ));

    SfxChild_Impl *pCli = 0;
    for ( USHORT nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        SfxChildWin_Impl* pCW = 0;
        pCli = (*pChilds)[nPos];

        if ( pCli && pCli->pWin )
        {
            // We have to find the SfxChildWin_Impl to retrieve the
            // SFX_CHILDWIN flags that can influence visibility.
            for (USHORT n=0; n<pChildWins->Count(); n++)
            {
                SfxChildWin_Impl* pCWin = (*pChildWins)[n];
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
                bVisible = !bInvisible || ( bInvisible & (( nFlags & SFX_CHILDWIN_NEVERHIDE ) != 0 ));
            }

            if ( CHILD_VISIBLE == (pCli->nVisible & CHILD_VISIBLE) && bVisible )
            {
                USHORT nFlags = pCli->bSetFocus ? 0 : SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE;
                switch ( pCli->pWin->GetType() )
                {
                    case RSC_DOCKINGWINDOW :
                        ((DockingWindow*)pCli->pWin)->Show( TRUE, nFlags );
                        break;
                    case RSC_SPLITWINDOW :
                        ((SplitWindow*)pCli->pWin)->Show( TRUE, nFlags );
                        break;
                    default:
                        pCli->pWin->Show( TRUE, nFlags );
                        break;
                }

                pCli->bSetFocus = FALSE;
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

void SfxWorkWindow::HideChilds_Impl()
{
    SfxChild_Impl *pChild = 0;
    for ( USHORT nPos = pChilds->Count(); nPos > 0; --nPos )
    {
        pChild = (*pChilds)[nPos-1];
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
    USHORT n;
    for ( n = 0; n < aObjBarList.size(); n++ )
        aObjBarList[n].bDestroy = sal_True;

    for ( n = 0; n < pChildWins->Count(); ++n )
        (*pChildWins)[n]->nId = 0;
}

void SfxWorkWindow::NextObjectBar_Impl( USHORT )
{
}

USHORT SfxWorkWindow::HasNextObjectBar_Impl( USHORT, String* )
{
    return 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::SetObjectBar_Impl( USHORT nPos, sal_uInt32 nResId,
            SfxInterface* pIFace, const String *pName)
{
    DBG_ASSERT( (nPos & SFX_POSITION_MASK) < SFX_OBJECTBAR_MAX,
                "object bar position overflow" );

    USHORT nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
    {
        pParent->SetObjectBar_Impl( nPos, nResId, pIFace, pName );
        return;
    }

    SfxObjectBar_Impl aObjBar;
    aObjBar.pIFace = pIFace;
    aObjBar.nId = sal::static_int_cast<USHORT>(nResId);
    aObjBar.nPos = nRealPos;
    aObjBar.nMode = (nPos & SFX_VISIBILITY_MASK);
    if (pName)
        aObjBar.aName = *pName;
    else
        aObjBar.aName.Erase();

    for ( USHORT n=0; n<aObjBarList.size(); n++ )
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

bool SfxWorkWindow::KnowsObjectBar_Impl( USHORT nPos ) const

/*  [Description]

    Determines if a object list is available at the position in question.
    This is independent for the fact whether it is actually turned on or off.
*/

{
    USHORT nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
        return pParent->KnowsObjectBar_Impl( nPos );

    for ( USHORT n=0; n<aObjBarList.size(); n++ )
    {
        if ( aObjBarList[n].nPos == nRealPos )
            return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------

BOOL SfxWorkWindow::IsVisible_Impl( USHORT nMode ) const
{
    switch( nUpdateMode )
    {
        case SFX_VISIBILITY_STANDARD:
            return TRUE;
        case SFX_VISIBILITY_UNVISIBLE:
            return FALSE;
        case SFX_VISIBILITY_PLUGSERVER:
        case SFX_VISIBILITY_PLUGCLIENT:
        case SFX_VISIBILITY_CLIENT:
        case SFX_VISIBILITY_SERVER:
            return !!(nMode & nUpdateMode);
        default:
            return !!(nMode & nOrigMode ) ||
                nOrigMode == SFX_VISIBILITY_STANDARD;
    }
}

Window* SfxWorkWindow::GetObjectBar_Impl( USHORT, sal_uInt32 )
{
    return NULL;
}

//------------------------------------------------------------------------
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

//  if ( pTask->IsActive() )
    {
        pWork = pParent;
        while ( pWork )
        {
            pWork->ArrangeChilds_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ArrangeChilds_Impl( FALSE );

        pWork = pParent;
        while ( pWork )
        {
            pWork->ShowChilds_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ShowChilds_Impl();
    }

    ShowChilds_Impl();
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
    USHORT n;
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
        USHORT      nId      = aObjBarList[n].nId;
        sal_Bool    bDestroy = aObjBarList[n].bDestroy;

        // Determine the vaild mode for the ToolBox
        USHORT nTbxMode = aObjBarList[n].nMode;
        bool bFullScreenTbx = SFX_VISIBILITY_FULLSCREEN ==
                                  ( nTbxMode & SFX_VISIBILITY_FULLSCREEN );
        nTbxMode &= ~SFX_VISIBILITY_FULLSCREEN;
        nTbxMode &= ~SFX_VISIBILITY_VIEWER;

        // Is a ToolBox required in this context ?
        bool bModesMatching = ( nUpdateMode && ( nTbxMode & nUpdateMode) == nUpdateMode );
        if ( bDestroy )
        {
            rtl::OUString aTbxId( m_aTbxTypeName );
            aTbxId += GetResourceURLFromResId( aObjBarList[n].nId );
            xLayoutManager->destroyElement( aTbxId );
        }
        else if ( nId != 0 && ( ( bModesMatching && !bIsFullScreen ) ||
                                ( bIsFullScreen && bFullScreenTbx ) ) )
        {
            rtl::OUString aTbxId( m_aTbxTypeName );
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
            rtl::OUString aTbxId( m_aTbxTypeName );
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
            p->Lock(FALSE);
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
   for ( USHORT n=0; n<pChildWins->Count(); n++ )
    {
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChildWin = pCW->pWin;
        BOOL bCreate = FALSE;
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
                    bCreate = TRUE;

                if ( bCreate )
                    bCreate = AllowChildWindowCreation_Impl( *pCW );

                // Currently, no window here, but it is enabled; windows
                // Create window and if possible theContext
                if ( bCreate )
                    CreateChildWin_Impl( pCW, FALSE );

                if ( !bAllChildsVisible )
                {
                    if ( pCW->pCli )
                        pCW->pCli->nVisible &= ~CHILD_ACTIVE;
                }
            }
            else if ( pChildWin )
            {
                // Window already exists, it should also be visible?
                if ( ( !bIsFullScreen || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT ) && bAllChildsVisible )
                {
                    // Update Mode is compatible; definitely enable it
                    bCreate = AllowChildWindowCreation_Impl( *pCW );
                    if ( bCreate )
                    {
                        if ( pCW->pCli )
                        {
                            // The window is a direct Child
                            if ( bAllChildsVisible && ( (IsDockingAllowed() && bInternalDockingAllowed) || pCW->pCli->eAlign == SFX_ALIGN_NOALIGNMENT ) )
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

void SfxWorkWindow::CreateChildWin_Impl( SfxChildWin_Impl *pCW, BOOL bSetFocus )
{
    if ( pCW->aInfo.bVisible != 42 )
        pCW->aInfo.bVisible = TRUE;

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

        // No! Otherwise, you could have disable any window stored (Partwindow!)
//              pCW->aInfo.bVisible = TRUE;

        // The creation was successful
        GetBindings().Invalidate(pCW->nId);

        USHORT nPos = pChildWin->GetPosition();
        if (nPos != CHILDWIN_NOPOS)
        {
            DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
            if ((*pChilds)[TbxMatch(nPos)])// &&
//       pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
            {
                // ChildWindow replaces ObjectBar
                (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
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
    USHORT nId = pCW->nSaveId;
    SfxChildWindow *pChildWin = pCW->pWin;

    // existing window goes out of the context and is therefore removed
    USHORT nPos = pChildWin->GetPosition();
    if (nPos != CHILDWIN_NOPOS)
    {
/*
        // ChildWindow overloads a ObjectBar
        DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
        if ((*pChilds)[TbxMatch(nPos)] &&
            (aObjBars[nPos].nMode & nUpdateMode) ) //&&
//                                                 pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
        {
            // ObjectBar was overloaded; now display it again
            (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
        }
*/
    }

    // Save the information in the INI file
    USHORT nFlags = pCW->aInfo.nFlags;
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
        aStatBar.nId = sal::static_int_cast<USHORT>(nResId);
}

#define SFX_ITEMTYPE_STATBAR 4

void SfxWorkWindow::SetTempStatusBar_Impl( BOOL bSet )
{
    if ( aStatBar.bTemp != bSet && bShowStatusBar && IsVisible_Impl() )
    {
        BOOL bOn = FALSE;
        BOOL bReset = FALSE;
        if ( bSet && !aStatBar.nId )
        {
            bReset = TRUE;
            SetStatusBar_Impl( SFX_ITEMTYPE_STATBAR, SFX_APP(), GetBindings() );
        }

        if ( aStatBar.nId && aStatBar.bOn && !bIsFullScreen )
            bOn = TRUE;

        aStatBar.bTemp = bSet;
        if ( !bOn || bReset || (!bSet && aStatBar.nId ) )
        {
            // Just do something if the temp settings really are causing
            // anything
            UpdateStatusBar_Impl();
            ArrangeChilds_Impl();
            ShowChilds_Impl();
        }

        if ( bReset )
            ResetStatusBar_Impl();
    }
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

//------------------------------------------------------------------------
/*
void SfxWorkWindow::SetObjectBarVisibility_Impl( USHORT nMask )
{
    switch( nMask )
    {
        case SFX_VISIBILITY_UNVISIBLE:
        case SFX_VISIBILITY_STANDARD:
        case SFX_VISIBILITY_CLIENT:
        case SFX_VISIBILITY_SERVER:
            nOrigMode = nMask;
    }
    if (nMask != nUpdateMode)
        nUpdateMode = nMask;
}*/

void SfxWorkWindow::MakeVisible_Impl( BOOL bVis )
{
    if ( bVis )
        nOrigMode = SFX_VISIBILITY_STANDARD;
    else
        nOrigMode = SFX_VISIBILITY_UNVISIBLE;

    if ( nOrigMode != nUpdateMode)
        nUpdateMode = nOrigMode;
}

BOOL SfxWorkWindow::IsVisible_Impl()
{
    return nOrigMode != SFX_VISIBILITY_UNVISIBLE;
}

//------------------------------------------------------------------------
void SfxWorkWindow::HidePopups_Impl(BOOL bHide, BOOL bParent, USHORT nId )
{
    for ( USHORT n = 0; n < pChildWins->Count(); ++n )
    {
        SfxChildWindow *pCW = (*pChildWins)[n]->pWin;
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
            SfxDockingConfig eConfig, USHORT nId)
{
    SfxDockingWindow* pDockWin=0;
    USHORT nPos = USHRT_MAX;
    Window *pWin=0;
    SfxChildWin_Impl *pCW = 0;

    if ( eChild == SFX_CHILDWIN_OBJECTBAR )
    {
        return;
    }
    else
    {
        // configure direct childwindow
        for (USHORT n=0; n<pChildWins->Count(); n++)
        {
            pCW = (*pChildWins)[n];
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
                    ((SplitWindow*)pWin)->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
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
    USHORT n;
    for ( n=0; n<aSortedList.Count(); ++n )
    {
        pChild = (*pChilds)[aSortedList[n]];
        if ( pChild )
            if ( pChild->pWin == pWin )
            break;
    }

    if ( n < aSortedList.Count() )
        // sometimes called while toggeling float mode
        nPos = aSortedList[n];

    switch ( eConfig )
    {
        case SFX_SETDOCKINGRECTS :
        {
            if ( nPos == USHRT_MAX )
                return;

//                      SfxChild_Impl *pChild = (*pChilds)[nPos];
            Rectangle aOuterRect( GetTopRect_Impl() );
            aOuterRect.SetPos( pWorkWin->OutputToScreenPixel( aOuterRect.TopLeft() ));
            Rectangle aInnerRect( aOuterRect );
            BOOL bTbx = (eChild == SFX_CHILDWIN_OBJECTBAR);

            // The current affected window is included in the calculation of
            // the inner rectangle!
            for ( USHORT m=0; m<aSortedList.Count(); ++m )
            {
                USHORT i=aSortedList[m];
                SfxChild_Impl* pCli = (*pChilds)[i];

                if ( pCli && pCli->nVisible == CHILD_VISIBLE && pCli->pWin )
                {
                    switch ( pCli->eAlign )
                    {
                        case SFX_ALIGN_TOP:
                            // Objekt-Toolboxes come always last
                            //if ( bTbx || i <= nPos)
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
                            //if ( bTbx || i <= nPos)
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
                            //if (bTbx || i <= nPos)
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
                            //if (bTbx || i <= nPos)
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
            SfxChild_Impl *pCli = ( nPos != USHRT_MAX ) ? (*pChilds)[nPos] : 0;
            if ( pCli && pDockWin )
            {
                eAlign = pDockWin->GetAlignment();
                if ( eChild == SFX_CHILDWIN_DOCKINGWINDOW || eAlign == SFX_ALIGN_NOALIGNMENT)
                {
                    // configuration inside the SplitWindow, no change for the SplitWindows' configuration
                    pCli->bResize = TRUE;
                    pCli->aSize = pDockWin->GetSizePixel();
                }
            }

            if ( pCli )
            {
                if( pCli->eAlign != eAlign )
                {
                    bSorted = FALSE;
                    pCli->eAlign = eAlign;
                }

                ArrangeChilds_Impl();
                ShowChilds_Impl();
            }

            if ( pCW && pCW->pWin )
            {
                // store changed configuration
                USHORT nFlags = pCW->aInfo.nFlags;
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

void SfxWorkWindow::SetChildWindowVisible_Impl( sal_uInt32 lId, BOOL bEnabled, USHORT nMode )
{
    USHORT nInter = (USHORT) ( lId >> 16 );
    USHORT nId = (USHORT) ( lId & 0xFFFF );

    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are alwas registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
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
            pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
        else
            pChildWins->Insert( pChildWins->Count(), pCW );
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

void SfxWorkWindow::ToggleChildWindow_Impl(USHORT nId, BOOL bSetFocus)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nId == nId)
            break;

    if ( n<nCount )
    {
        // The Window is aleady known
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
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
                        pCW->bCreate = FALSE;
                        if ( pChild->IsHideAtToggle() )
                        {
                            ShowChildWindow_Impl( nId, FALSE, bSetFocus );
                        }
                        else
                        {
                            // The Window should be switched off
                            pChild->SetVisible_Impl( FALSE );
                            RemoveChildWin_Impl( pCW );
                        }
                    }
                }
                else
                {
                    // no actual Window exists, yet => just remember the "switched off" state
                    pCW->bCreate = FALSE;
                }
            }
            else
            {
                pCW->bCreate = AllowChildWindowCreation_Impl( *pCW );
                if ( pCW->bCreate )
                {
                    if ( pChild )
                    {
                        ShowChildWindow_Impl( nId, TRUE, bSetFocus );
                    }
                    else
                    {
                        // create actual Window
                        CreateChildWin_Impl( pCW, bSetFocus );
                        if ( !pCW->pWin )
                            // no success
                            pCW->bCreate = FALSE;
                    }
                }
            }
        }

        ArrangeChilds_Impl();
        ShowChilds_Impl();

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
    nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if ( n < nCount )
    {
        DBG_ERROR("The ChildWindow is not in context!");
    }
    else
    {
        DBG_ERROR("The ChildWindow is not registered!");
    }
#endif
}

//--------------------------------------------------------------------

BOOL SfxWorkWindow::HasChildWindow_Impl(USHORT nId)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if (n<nCount)
    {
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        return ( pChild && pCW->bCreate );
    }

    if ( pParent )
        return pParent->HasChildWindow_Impl( nId );

    return FALSE;
}

BOOL SfxWorkWindow::IsFloating( USHORT nId )
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are alwas registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If new, then initialize, add this here depending on the flag or
        // the Parent
        pCW = new SfxChildWin_Impl( nId );
        pCW->bEnable = FALSE;
        pCW->nId = 0;
        pCW->nVisibility = 0;
        InitializeChild_Impl( pCW );
        if ( pWork && !( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) )
            pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
        else
            pChildWins->Insert( pChildWins->Count(), pCW );
    }

    SfxChildAlignment eAlign;
    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
        return( eAlign == SFX_ALIGN_NOALIGNMENT );
    else
        return TRUE;
}

//--------------------------------------------------------------------

BOOL SfxWorkWindow::KnowsChildWindow_Impl(USHORT nId)
{
    SfxChildWin_Impl *pCW=0;
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
    {
        pCW = (*pChildWins)[n];
        if ( pCW->nSaveId == nId)
             break;
    }

    if (n<nCount)
    {
        if ( !(pCW->aInfo.nFlags & SFX_CHILDWIN_ALWAYSAVAILABLE) && !IsVisible_Impl(  pCW->nVisibility ) )
            return FALSE;
        return pCW->bEnable;
    }
    else if ( pParent )
        return pParent->KnowsChildWindow_Impl( nId );
    else
        return FALSE;
}

//--------------------------------------------------------------------

void SfxWorkWindow::SetChildWindow_Impl(USHORT nId, BOOL bOn, BOOL bSetFocus)
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Get the top parent, child windows are alwas registered at the
    // task of the WorkWindow for example the frame or on AppWorkWindow
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // The Parent already known?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // If no Parent or the Parent us still unknown, then search here
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
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
        pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
    }

    if ( pCW->bCreate != bOn )
        pWork->ToggleChildWindow_Impl(nId,bSetFocus);
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChildWindow_Impl(USHORT nId, BOOL bVisible, BOOL bSetFocus)
{
    USHORT nCount = pChildWins->Count();
    SfxChildWin_Impl* pCW=0;
    USHORT n;
    for (n=0; n<nCount; n++)
    {
        pCW = (*pChildWins)[n];
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

            ArrangeChilds_Impl();
            ShowChilds_Impl();
        }
        else if ( bVisible )
        {
            SetChildWindow_Impl( nId, TRUE, bSetFocus );
            pChildWin = pCW->pWin;
        }

        if ( pChildWin )
        {
            pChildWin->SetVisible_Impl( bVisible );
            USHORT nFlags = pCW->aInfo.nFlags;
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
    nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if ( n<nCount )
    {
        DBG_ERROR("The ChildWindow is not in context!");
    }
    else
    {
        DBG_ERROR("The ChildWindow is not registered");
    }
#endif
}

//--------------------------------------------------------------------

SfxChildWindow* SfxWorkWindow::GetChildWindow_Impl(USHORT nId)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
             break;

    if (n<nCount)
        return (*pChildWins)[n]->pWin;
    else if ( pParent )
        return pParent->GetChildWindow_Impl( nId );
    return 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::ResetChildWindows_Impl()
{
//      if ( pParent )
//              pParent->ResetChildWindows_Impl();

    for ( USHORT n = 0; n < pChildWins->Count(); ++n )
    {
        (*pChildWins)[n]->nId = 0;
        (*pChildWins)[n]->bEnable = FALSE;
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

BOOL SfxWorkWindow::RequestTopToolSpacePixel_Impl( SvBorder aBorder )
{
    if ( !IsDockingAllowed() ||
            aClientArea.GetWidth() < aBorder.Left() + aBorder.Right() ||
            aClientArea.GetHeight() < aBorder.Top() + aBorder.Bottom() )
        return FALSE;
    else
        return TRUE;;
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
        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
        {
            pFact = rFactories[nFactory];
            if ( pFact->nId == pCW->nSaveId )
            {
                pCW->aInfo   = pFact->aInfo;
                SfxChildWindow::InitializeChildWinFactory_Impl(
                                            pCW->nSaveId, pCW->aInfo);
                pCW->bCreate = pCW->aInfo.bVisible;
                USHORT nFlags = pFact->aInfo.nFlags;
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
            for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == pCW->nSaveId )
                {
                    pCW->aInfo   = pFact->aInfo;
                    SfxChildWindow::InitializeChildWinFactory_Impl(
                                                pCW->nSaveId, pCW->aInfo);
                    pCW->bCreate = pCW->aInfo.bVisible;
                    USHORT nFlags = pFact->aInfo.nFlags;
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
/*
SfxStatBar_Impl* SfxWorkWindow::GetStatusBar_Impl()
{
    return &aStatBar;
} */

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

void SfxWorkWindow::MakeChildsVisible_Impl( BOOL bVis )
{
    if ( pParent )
        pParent->MakeChildsVisible_Impl( bVis );

    bAllChildsVisible = bVis;
    if ( bVis )
    {
        if ( !bSorted )
            Sort_Impl();
        for ( USHORT n=0; n<aSortedList.Count(); ++n )
        {
            SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
            if ( (pCli->eAlign == SFX_ALIGN_NOALIGNMENT) || (IsDockingAllowed() && bInternalDockingAllowed) )
                pCli->nVisible |= CHILD_ACTIVE;
        }
    }
    else
    {
        if ( !bSorted )
            Sort_Impl();
        for ( USHORT n=0; n<aSortedList.Count(); ++n )
        {
            SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
            pCli->nVisible &= ~CHILD_ACTIVE;
        }
    }
}

BOOL SfxWorkWindow::IsAutoHideMode( const SfxSplitWindow *pSplitWin )
{
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        if ( pSplit[n] != pSplitWin && pSplit[n]->IsAutoHide( TRUE ) )
            return TRUE;
    }
    return FALSE;
}


void SfxWorkWindow::EndAutoShow_Impl( Point aPos )
{
    if ( pParent )
        pParent->EndAutoShow_Impl( aPos );

    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
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
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Either dummy window or window in the auto-show-mode are processed
        // (not pinned, FadeIn).
        // Only the abandoned window may be invisible, because perhaps its
        // size is just beeing calculated before it is displayed.
        SfxSplitWindow* pSplitWin = pSplit[n];
        BOOL bDummyWindow = !pSplitWin->IsFadeIn();
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

Rectangle SfxWorkWindow::GetFreeArea( BOOL bAutoHide ) const
{
    if ( bAutoHide )
    {
        Rectangle aArea( aClientArea );
        for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
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

SfxChildWinController_Impl::SfxChildWinController_Impl( USHORT nID, SfxWorkWindow *pWork )
    : SfxControllerItem( nID, pWork->GetBindings() )
    , pWorkwin( pWork )
{}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SfxWorkWindow::CreateDispatch( const String& )
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >();
}

void SfxChildWinController_Impl::StateChanged(
    USHORT nSID, SfxItemState eState, const SfxPoolItem* )
{
    pWorkwin->DisableChildWindow_Impl( nSID, eState == SFX_ITEM_DISABLED );
}

void SfxWorkWindow::DisableChildWindow_Impl( USHORT nId, BOOL bDisable )
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
             break;
    if ( n<nCount && (*pChildWins)[n]->bDisabled != bDisable )
    {
        (*pChildWins)[n]->bDisabled = bDisable;
        UpdateChildWindows_Impl();
        ArrangeChilds_Impl();
        ShowChilds_Impl();
    }
}

void SfxWorkWindow::SetActiveChild_Impl( Window *pChild )
{
    pActiveChild = pChild;
}

Window* SfxWorkWindow::GetActiveChild_Impl()
{
    return pActiveChild;
}

BOOL SfxWorkWindow::ActivateNextChild_Impl( BOOL bForward )
{
    // Sort all children under list
    SvUShorts aList;
    for ( USHORT i=SFX_OBJECTBAR_MAX; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if ( pCli && pCli->bCanGetFocus && pCli->pWin )
        {
            USHORT k;
            for (k=0; k<aList.Count(); k++)
                if ( ChildTravelValue((*pChilds)[aList[k]]->eAlign) > ChildTravelValue(pCli->eAlign) )
                    break;
            aList.Insert(i,k);
        }
    }

    if ( aList.Count() == 0 )
        return FALSE;

    USHORT nTopValue  = ChildTravelValue( SFX_ALIGN_LOWESTTOP );
    for ( USHORT i=0; i<aList.Count(); i++ )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[i]];
        if ( pCli->pWin && ChildTravelValue( pCli->eAlign ) > nTopValue )
            break;
    }

    USHORT n = bForward ? 0 : aList.Count()-1;
    SfxChild_Impl *pAct=NULL;
    if ( pActiveChild )
    {
        // Look for the active window
        for ( n=0; n<aList.Count(); n++ )
        {
            SfxChild_Impl* pCli = (*pChilds)[aList[n]];
            if ( pCli && pCli->pWin && ( pCli->pWin == pActiveChild || !pActiveChild ) )
            {
                pAct = pCli;
                break;
            }
        }
    }

    // dummy entries for the container window
    aList.Insert( 0xFFFF, 0 );
    aList.Insert( 0xFFFF, aList.Count() );
    n = n + 1;
    if ( pAct )
    {
        for ( USHORT i=0; i<SFX_SPLITWINDOWS_MAX; i++ )
        {
            // Maybe the pNext is a Splitwindow
            SfxSplitWindow *p = pSplit[i];
            if ( pAct->pWin == p )
            {
                if( p->ActivateNextChild_Impl( bForward ) )
                    return TRUE;
                break;
            }
        }

        // pAct is a direct ChildWindow
        // continue with the successor or predecessor of the active window
        if ( bForward )
            n = n+1;
        else
            n = n-1;

        if ( n == 0 || n == aList.Count()-1 )
            return FALSE;
    }

    for( ;; )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[n]];
        if ( pCli->pWin )
        {
            SfxChild_Impl* pNext = pCli;
            for ( USHORT i=0; n<SFX_SPLITWINDOWS_MAX; n++ )
            {
                // Maybe the pNext is a Splitwindow
                SfxSplitWindow *p = pSplit[i];
                if ( pNext->pWin == p )
                {
                    // Activate the first/last window
                    p->SetActiveWindow_Impl( NULL );
                    pNext = NULL;
                    if( p->ActivateNextChild_Impl( bForward ) )
                        return TRUE;
                    break;
                }
            }

            if ( pNext )
            {
                pNext->pWin->GrabFocus();
                pActiveChild = pNext->pWin;
                return TRUE;
            }
        }

        if ( bForward )
            n = n+1;
        else
            n = n-1;

        if ( n == 0 || n == aList.Count()-1 )
            break;
    }

    return FALSE;
}

void SfxWorkWindow::SetObjectBarCustomizeMode_Impl( BOOL )
{
}

void SfxWorkWindow::DataChanged_Impl( const DataChangedEvent& )
{
    USHORT n;
    USHORT nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
    {
        SfxChildWin_Impl*pCW = (*pChildWins)[n];
        if ( pCW && pCW->pWin )
            pCW->pWin->GetWindow()->UpdateSettings( Application::GetSettings() );
    }

    ArrangeChilds_Impl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
