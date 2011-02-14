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

#ifndef GCC
#endif

#include <stdio.h>
#include <hash_map>

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
#include "sfx2/sfxresid.hxx"
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
#ifndef _SFXEITEM_HXX //autogen
#include <svl/eitem.hxx>
#endif
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
    sal_uInt16      nId;
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
    { 0,        ""                     }
};

DBG_NAME(SfxWorkWindow)

//SV_IMPL_OBJARR( SfxObjectBarArr_Impl, SfxObjectBar_Impl );

//====================================================================
// Sortiert die Children nach ihrem Alignment
// Reihenfolge entspricht der im enum SfxChildAlignment (->CHILDWIN.HXX).
//

// Hilfe, um die "Anderungen am Alignment kompatibal zu machen!


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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pWrkWin )
    {
        if ( eLayoutEvent == css::frame::LayoutManagerEvents::VISIBLE )
        {
            m_pWrkWin->MakeVisible_Impl( sal_True );
            m_pWrkWin->ShowChilds_Impl();
            m_pWrkWin->ArrangeChilds_Impl( sal_True );
        }
        else if ( eLayoutEvent == css::frame::LayoutManagerEvents::INVISIBLE )
        {
            m_pWrkWin->MakeVisible_Impl( sal_False );
            m_pWrkWin->HideChilds_Impl();
            m_pWrkWin->ArrangeChilds_Impl( sal_True );
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

//====================================================================

typedef std::hash_map< sal_Int32, rtl::OUString > ToolBarResIdToResourceURLMap;

static sal_Bool bMapInitialized = sal_False;
static ToolBarResIdToResourceURLMap aResIdToResourceURLMap;

static rtl::OUString GetResourceURLFromResId( sal_uInt16 nResId )
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
    aSortedList.Remove(0, aSortedList.Count());
    for (sal_uInt16 i=0; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if (pCli)
        {
            sal_uInt16 k;
            for (k=0; k<aSortedList.Count(); k++)
//              if ( (*pChilds)[aSortedList[k]]->eAlign > pCli->eAlign )
                if (ChildAlignValue((*pChilds)[aSortedList[k]]->eAlign) >
                    ChildAlignValue(pCli->eAlign))
                    break;
            aSortedList.Insert (i,k);
        }
    }

    bSorted = sal_True;
}


//====================================================================
// ctor f"ur workwin eines Frames

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

    // Die ben"otigten SplitWindows (je eins f"ur jede Seite) werden erzeugt
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Die SplitWindows sind direkte ChildWindows des WorkWindows und enthalten
        // die angedockten Fenster.

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
// ctor der Basisklasse

SfxWorkWindow::SfxWorkWindow( Window *pWin, SfxBindings& rB, SfxWorkWindow* pParentWorkwin ) :
    pParent( pParentWorkwin ),
    pBindings(&rB),
    pWorkWin (pWin),
    pConfigShell( 0 ),
    pActiveChild( 0 ),
    nChilds( 0 ),
    nOrigMode( 0 ),
    bSorted( sal_True ),
    bDockingAllowed(sal_True),
    bInternalDockingAllowed(sal_True),
    bAllChildsVisible(sal_True),
    bIsFullScreen( sal_False ),
    bShowStatusBar( sal_True ),
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

    // F"ur die ObjectBars wird ein fester Platz in der ChildList reserviert,
    // damit sie immer in einer definierten Reihenfolge kommen.
    SfxChild_Impl* pChild=0;
    for (sal_uInt16 n=0; n < SFX_OBJECTBAR_MAX; ++n)
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
// dtor

SfxWorkWindow::~SfxWorkWindow()
{
    DBG_DTOR(SfxWorkWindow, 0);

    // SplitWindows l"oschen
    for ( sal_uInt16 n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            ReleaseChild_Impl(*p);
        delete p;
    }

    // Hilfsstruktur f"ur Child-Windows l"oschen
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

void SfxWorkWindow::Lock_Impl( sal_Bool bLock )
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
    for ( sal_uInt16 nPos = 0; nPos < pChilds->Count(); ++nPos )
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
    sal_uInt16 nCount = pChildWins->Count();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxChildWin_Impl* pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            sal_uInt16 nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
        }
    }
}

//--------------------------------------------------------------------
// Hilfsmethode zum Freigeben der Childlisten. Wenn danach nicht der dtor
// aufgerufen wird, sondern weiter gearbeitet wird, mu\s wie im ctor von
// SfxWorkWindow noch Platz f"ur die Objectbars und SplitWindows reserviert
// werden.

void SfxWorkWindow::DeleteControllers_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    // SplitWindows locken (d.h. Resize-Reaktion an den
    // DockingWindows unterdr"ucken)
    sal_uInt16 n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
           if (p->GetWindowCount())
        p->Lock();
    }

    // Child-Windows l"oschen
    for ( n=0; n<pChildWins->Count(); )
    {
        SfxChildWin_Impl* pCW = (*pChildWins)[n];
        pChildWins->Remove(n);
           SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
/*
            sal_uInt16 nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
*/
            pChild->Hide();

            // Wenn das ChildWindow ein direktes Childfenster ist und nicht
            // in einem SplitWindow liegt, am WorkWindow abmelden.
            // Nach TH ist eine Abmeldung am Splitwindow nicht erforderlich,
            // wenn dieses auch gleich mit zerst"ort wird (s.u.).
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

    //pChildWins->Remove((sal_uInt16)0, nCount);

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

        // StatusBar l"oschen
        ResetStatusBar_Impl();

        // ObjectBars l"oschen( zuletzt, damit pChilds nicht tote Pointer enh"alt )
        for ( sal_uInt16 i = 0; i < aObjBarList.size(); i++ )
        {
            // Nicht jede Position mu\s belegt sein
            sal_uInt16 nId = aObjBarList[i].nId;
            if ( nId )
                aObjBarList[i].nId = 0;
        }
    }

    // ObjectBars werden alle auf einmal released, da sie einen
    // festen zusammenh"angenden  Bereich im Array pChilds belegen
    pChilds->Remove(0, SFX_OBJECTBAR_MAX);
    bSorted = sal_False;

    nChilds = 0;
}

//====================================================================
// Virtuelle Methode zum Anordnen der Childfenster.

void SfxWorkWindow::ArrangeChilds_Impl( sal_Bool /*bForce*/)
{
    Arrange_Impl();
}

void SfxFrameWorkWin_Impl::ArrangeChilds_Impl( sal_Bool bForce )
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

    // Wenn das aktuelle Dokument der Applikation einen IPClient enth"alt, mu\s
    // dem dazugeh"origen Objekt durch SetTopToolFramePixel der zur Verf"ugung
    // stehende Platz zugeteilt werden. Das Objekt zeigt dann seine UITools an
    // und setzt den App-Border(->SfxInPlaceEnv_Impl::ArrangeChilds_Impl()).
    // Anderenfalls wird hier direkt der AppBorder gesetzt, um evtl. den Border
    // zu "uberschreiben, den bisher ein Objekt aus einem anderen Dokument
    // gesetzt hatte.
    // Das Objekt setzt, wenn es seine UI-Tools wegnimmt, den SetAppBorder nicht,
    // damit kein ObjectBar-Zappeln entsteht.
    // (->SfxInPlaceEnv_Impl::ArrangeChilds_Impl())

    pMasterFrame->SetToolSpaceBorderPixel_Impl( aBorder );

    ArrangeAutoHideWindows( NULL );
}

//--------------------------------------------------------------------

SvBorder SfxWorkWindow::Arrange_Impl()

/*  [Beschreibung]

    Diese Methode ordnet alle sichtbaren ChildFenster so an, da\s die angedockten
    Fenster nach der Sorierreihenfolge von au\sen nach innen aneinander
    gesetzt werden. Wenn ein an sich sichtbares Fenster nicht mehr in die
    noch freie ClientArea pa\st, wird es auf "nicht sichtbar" gesetzt.

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

    for ( sal_uInt16 n=0; n<aSortedList.Count(); ++n )
    {
        SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
        if ( !pCli->pWin )
            continue;

        // Zun"achst nehmen wir an, da\s das Fenster Platz hat
        pCli->nVisible |= CHILD_FITS_IN;

        // Nicht sichtbare Fenster "uberspringen
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

//--------------------------------------------------------------------
// Close-Handler: die Konfiguration der ChildWindows wird gespeichert.
//

void SfxWorkWindow::Close_Impl()
{
    for (sal_uInt16 n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            sal_uInt16 nFlags = pCW->aInfo.nFlags;
            pCW->aInfo = pChild->GetInfo();
            pCW->aInfo.nFlags |= nFlags;
            SaveStatus_Impl(pChild, pCW->aInfo);
        }
    }
}

sal_Bool SfxWorkWindow::PrepareClose_Impl()
{
    for (sal_uInt16 n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
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
    DBG_ASSERT( pChilds->Count() < 255, "too many childs" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );
    DBG_ASSERT( !FindChild_Impl(rWindow), "child registered more than once" );


    if ( rWindow.GetParent() != pWorkWin )
        rWindow.SetParent( pWorkWin );

    SfxChild_Impl *pChild = new SfxChild_Impl(rWindow, rWindow.GetSizePixel(),
                                    eAlign, rWindow.IsVisible());
    pChild->bCanGetFocus = bCanGetFocus;

    pChilds->Insert(pChilds->Count(), pChild);
    bSorted = sal_False;
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
            bSorted = sal_False;

        pChild->eAlign = eAlign;
        pChild->aSize = rNewSize;
        pChild->bResize = sal_True;
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
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        pChild = (*pChilds)[nPos];
        if ( pChild )
          if ( pChild->pWin == &rWindow )
            break;
    }

    if ( nPos < pChilds->Count() )
    {
        bSorted = sal_False;
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
    sal_uInt16 nCount = pChilds->Count();
    for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
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
    for ( sal_uInt16 nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        SfxChildWin_Impl* pCW = 0;
        pCli = (*pChilds)[nPos];

        if ( pCli && pCli->pWin )
        {
            // We have to find the SfxChildWin_Impl to retrieve the
            // SFX_CHILDWIN flags that can influence visibility.
            for (sal_uInt16 n=0; n<pChildWins->Count(); n++)
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

void SfxWorkWindow::HideChilds_Impl()
{
    SfxChild_Impl *pChild = 0;
    for ( sal_uInt16 nPos = pChilds->Count(); nPos > 0; --nPos )
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
    sal_uInt16 n;
    for ( n = 0; n < aObjBarList.size(); n++ )
        aObjBarList[n].bDestroy = sal_True;

    for ( n = 0; n < pChildWins->Count(); ++n )
        (*pChildWins)[n]->nId = 0;
}

void SfxWorkWindow::NextObjectBar_Impl( sal_uInt16 )
{
}

sal_uInt16 SfxWorkWindow::HasNextObjectBar_Impl( sal_uInt16, String* )
{
    return 0;
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

/*  [Beschreibung]

    Stellt fest, ob an der betreffenden Position "uberhaupt eine
    Objektleiste zur Verf"ugung stehen w"urde. Ist unabh"agig davon,
    ob diese tats"achlich ein- oder ausgeschaltet ist.
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

Window* SfxWorkWindow::GetObjectBar_Impl( sal_uInt16, sal_uInt32 )
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

        ArrangeChilds_Impl( sal_False );

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
    // SplitWindows locken (d.h. Resize-Reaktion an den
    // DockingWindows unterdr"ucken)
    sal_uInt16 n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    // was man so "ofters braucht, merkt man sich (spart Code und Laufzeit)
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

    // "uber alle Toolboxen iterieren
    xLayoutManager->lock();
    for ( n = 0; n < aObjBarList.size(); ++n )
    {
        sal_uInt16      nId      = aObjBarList[n].nId;
        sal_Bool    bDestroy = aObjBarList[n].bDestroy;

        // die Modi bestimmen, f"ur die die ToolBox gilt
        sal_uInt16 nTbxMode = aObjBarList[n].nMode;
        bool bFullScreenTbx = SFX_VISIBILITY_FULLSCREEN ==
                                  ( nTbxMode & SFX_VISIBILITY_FULLSCREEN );
        nTbxMode &= ~SFX_VISIBILITY_FULLSCREEN;
        nTbxMode &= ~SFX_VISIBILITY_VIEWER;

        // wird in diesem Kontext eine ToolBox gefordert?
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
            // ggf. Toolbox an dieser Position l"oschen
            rtl::OUString aTbxId( m_aTbxTypeName );
            aTbxId += GetResourceURLFromResId( aObjBarList[n].nId );
            xLayoutManager->destroyElement( aTbxId );
        }
    }

    UpdateStatusBar_Impl();

    // unlocking automatically forces Layout
    xLayoutManager->unlock();

    UpdateChildWindows_Impl();

    // SplitWindows wieder ent-locken
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
    // alle vorhandenen oder in den Kontext gekommenen ChildWindows
    for ( sal_uInt16 n=0; n<pChildWins->Count(); n++ )
    {
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChildWin = pCW->pWin;
        sal_Bool bCreate = sal_False;
        if ( pCW->nId && !pCW->bDisabled  && (pCW->aInfo.nFlags & SFX_CHILDWIN_ALWAYSAVAILABLE || IsVisible_Impl( pCW->nVisibility ) ) )
        {
            // Im Kontext ist ein geeignetes ChildWindow erlaubt;
            // ist es auch eingeschaltet ?
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
                    // im PresentationMode oder FullScreen nur FloatingWindows
                    SfxChildAlignment eAlign;
                    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
                        bCreate = ( eAlign == SFX_ALIGN_NOALIGNMENT );
                }
                else
                    bCreate = sal_True;

                if ( bCreate )
                    bCreate = AllowChildWindowCreation_Impl( *pCW );

                // Momentan kein Fenster da, aber es ist eingeschaltet; Fenster
                // und ggf. Context erzeugen
                if ( bCreate )
                    CreateChildWin_Impl( pCW, sal_False );

                if ( !bAllChildsVisible )
                {
                    if ( pCW->pCli )
                        pCW->pCli->nVisible &= ~CHILD_ACTIVE;
                }
            }
            else if ( pChildWin )
            {
                // Fenster existiert schon; soll es auch sichtbar sein ?
                if ( ( !bIsFullScreen || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT ) && bAllChildsVisible )
                {
                    // Updatemode ist kompatibel; auf jeden Fall wieder einschalten
                    bCreate = AllowChildWindowCreation_Impl( *pCW );
                    if ( bCreate )
                    {
                        if ( pCW->pCli )
                        {
                            // Fenster ist direktes Child
                            if ( bAllChildsVisible && ( (IsDockingAllowed() && bInternalDockingAllowed) || pCW->pCli->eAlign == SFX_ALIGN_NOALIGNMENT ) )
                                pCW->pCli->nVisible |= CHILD_NOT_HIDDEN;
                        }
                        else
                        {
                            if ( pCW->bCreate && IsDockingAllowed() && bInternalDockingAllowed )
                                // Fenster liegt in einem SplitWindow
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
#if 0
        // Enable-Status richtig setzen
        pChildWin->GetWindow()->EnableInput( pCW->bEnable &&
            ( pWorkWin->IsInputEnabled() /* || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT */ ) );
#endif
        // Zumindest der ExtraString wird beim Auswerten ver"andert, also neu holen
        SfxChildWinInfo aInfo = pChildWin->GetInfo();
        pCW->aInfo.aExtraString = aInfo.aExtraString;
        pCW->aInfo.bVisible = aInfo.bVisible;
        pCW->aInfo.nFlags |= aInfo.nFlags;

        // Nein !! Sonst kann man keine Fenster defaultmaessig ausschalten ( Partwindow! )
//      pCW->aInfo.bVisible = sal_True;

        // Erzeugung war erfolgreich
        GetBindings().Invalidate(pCW->nId);

        sal_uInt16 nPos = pChildWin->GetPosition();
        if (nPos != CHILDWIN_NOPOS)
        {
            DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
            if ((*pChilds)[TbxMatch(nPos)])// &&
//                            pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
            {
                // ChildWindow ersetzt ObjectBar
                (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
            }
        }

        // make childwin keyboard accessible
        pWorkWin->GetSystemWindow()->GetTaskPaneList()->AddWindow( pChildWin->GetWindow() );

        pCW->pWin = pChildWin;

        if ( pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT || pChildWin->GetWindow()->GetParent() == pWorkWin)
        {
            // Das Fenster ist entweder nicht angedockt oder au\serhalb
            // eines SplitWindows angedockt und mu\s daher explizit als
            // Child registriert werden
            pCW->pCli = RegisterChild_Impl(*(pChildWin->GetWindow()), pChildWin->GetAlignment(), pChildWin->CanGetFocus());
            pCW->pCli->nVisible = CHILD_VISIBLE;
            if ( pChildWin->GetAlignment() != SFX_ALIGN_NOALIGNMENT && bIsFullScreen )
                pCW->pCli->nVisible ^= CHILD_ACTIVE;
            pCW->pCli->bSetFocus = bSetFocus;
        }
        else
        {
            // Ein angedocktes Fenster, dessen Parent nicht das WorkWindow ist,
            // mu\s in einem SplitWindow liegen und daher nicht explizit
            // registriert werden.
            // Das passiert aber schon bei der Initialisierung des
            // SfxDockingWindows!
        }

        if ( pCW->nInterfaceId != pChildWin->GetContextId() )
            pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );

        // Information in der INI-Datei sichern
        SaveStatus_Impl(pChildWin, pCW->aInfo);
    }
}

void SfxWorkWindow::RemoveChildWin_Impl( SfxChildWin_Impl *pCW )
{
    sal_uInt16 nId = pCW->nSaveId;
    SfxChildWindow *pChildWin = pCW->pWin;

    // vorhandenes Fenster geht aus dem Kontext und wird daher entfernt
    sal_uInt16 nPos = pChildWin->GetPosition();
    if (nPos != CHILDWIN_NOPOS)
    {
/*
        // ChildWindow "uberlagert einen ObjectBar
        DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
        if ((*pChilds)[TbxMatch(nPos)] &&
            (aObjBars[nPos].nMode & nUpdateMode) ) //&&
//                         pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
        {
            // ObjectBar war "uberlagert; jetzt wieder anzeigen
            (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
        }
*/
    }

    // Information in der INI-Datei sichern
    sal_uInt16 nFlags = pCW->aInfo.nFlags;
    pCW->aInfo = pChildWin->GetInfo();
    pCW->aInfo.nFlags |= nFlags;
    SaveStatus_Impl(pChildWin, pCW->aInfo);

    pChildWin->Hide();

    if ( pCW->pCli )
    {
        // ChildWindow ist ein direktes ChildWindow und mu\s sich daher
        // beim WorkWindow abmelden
        pCW->pCli = 0;
        ReleaseChild_Impl(*pChildWin->GetWindow());
    }
    else
    {
        // ChildWindow liegt in einem SplitWindow und meldet sich
        // selbst im dtor dort ab
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

#define SFX_ITEMTYPE_STATBAR 4

void SfxWorkWindow::SetTempStatusBar_Impl( sal_Bool bSet )
{
    if ( aStatBar.bTemp != bSet && bShowStatusBar && IsVisible_Impl() )
    {
        sal_Bool bOn = sal_False;
        sal_Bool bReset = sal_False;
        if ( bSet && !aStatBar.nId )
        {
            bReset = sal_True;
            SetStatusBar_Impl( SFX_ITEMTYPE_STATBAR, SFX_APP(), GetBindings() );
        }

        if ( aStatBar.nId && aStatBar.bOn && !bIsFullScreen )
            bOn = sal_True;

        aStatBar.bTemp = bSet;
        if ( !bOn || bReset || (!bSet && aStatBar.nId ) )
        {
            // Nur was tun, wenn die Temp-Einstellung wirklich was bewirkt
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

    // keine Statusleiste, wenn keine Id gew"unscht oder bei FullScreenView
    // oder wenn ausgeschaltet
    if ( aStatBar.nId && IsDockingAllowed() && bInternalDockingAllowed && bShowStatusBar &&
         ( (aStatBar.bOn && !bIsFullScreen) || aStatBar.bTemp ) )
    {
        // Id hat sich ge"andert, also passenden Statusbarmanager erzeugen,
        // dieser "ubernimmt die aktuelle Statusleiste;
        if ( xLayoutManager.is() )
            xLayoutManager->requestElement( m_aStatusBarResName );
    }
    else
    {
        // Aktuelle StatusBar vernichten
        // Der Manager erzeugt die Statusleiste nur, er zerst"ort sie
        // nicht !
        if ( xLayoutManager.is() )
            xLayoutManager->destroyElement( m_aStatusBarResName );
    }
}

//------------------------------------------------------------------------
/*
void SfxWorkWindow::SetObjectBarVisibility_Impl( sal_uInt16 nMask )
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
    for ( sal_uInt16 n = 0; n < pChildWins->Count(); ++n )
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
        for (sal_uInt16 n=0; n<pChildWins->Count(); n++)
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

//          SfxChild_Impl *pChild = (*pChilds)[nPos];
            Rectangle aOuterRect( GetTopRect_Impl() );
            aOuterRect.SetPos( pWorkWin->OutputToScreenPixel( aOuterRect.TopLeft() ));
            Rectangle aInnerRect( aOuterRect );
            sal_Bool bTbx = (eChild == SFX_CHILDWIN_OBJECTBAR);

            // Das gerade betroffene Fenster wird bei der Berechnung des
            // inneren Rechtecks mit eingeschlossen!
            for ( sal_uInt16 m=0; m<aSortedList.Count(); ++m )
            {
                sal_uInt16 i=aSortedList[m];
                SfxChild_Impl* pCli = (*pChilds)[i];

                if ( pCli && pCli->nVisible == CHILD_VISIBLE && pCli->pWin )
                {
                    switch ( pCli->eAlign )
                    {
                        case SFX_ALIGN_TOP:
                            // Objekt-Toolboxen kommen immer zuletzt
                            //if ( bTbx || i <= nPos)
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXTOP:
                            // Toolbox geht nur vor, wenn nicht h"ohere Position
                            if ( bTbx && i <= nPos)
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTTOP:
                            // Geht immer vor
                            aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTTOP:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if ( i == nPos )
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_BOTTOM:
                            // Objekt-Toolboxen kommen immer zuletzt
                            //if ( bTbx || i <= nPos)
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXBOTTOM:
                            // Toolbox geht nur vor, wenn nicht h"ohere Position
                            if ( bTbx && i <= nPos)
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTBOTTOM:
                            // Geht immer vor
                            aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTBOTTOM:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if ( i == nPos )
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LEFT:
                            // Toolboxen kommen immer zuletzt
                            //if (bTbx || i <= nPos)
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXLEFT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx && i <= nPos)
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTLEFT:
                            // Geht immer vor
                            aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTLEFT:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if (i == nPos)
                                aInnerRect.Left() += pCli->aSize.Width();

                        case SFX_ALIGN_RIGHT:
                            // Toolboxen kommen immer zuletzt
                            //if (bTbx || i <= nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXRIGHT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx && i <= nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTRIGHT:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if (i == nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTRIGHT:
                            // Geht immer vor
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

                ArrangeChilds_Impl();
                ShowChilds_Impl();
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

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        sal_uInt16 nCount = pWork->pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        sal_uInt16 nCount = pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
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
#if 0
    if ( pCW->pWin )
        pCW->pWin->GetWindow()->EnableInput( bEnabled &&
            ( pWorkWin->IsInputEnabled() /* || pCW->pWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT */ ) );
#endif
    pCW->nVisibility = nMode;
}

//--------------------------------------------------------------------
// Der An/Aus-Status eines ChildWindows wird umgeschaltet.

void SfxWorkWindow::ToggleChildWindow_Impl(sal_uInt16 nId, sal_Bool bSetFocus)
{
    sal_uInt16 nCount = pChildWins->Count();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nId == nId)
            break;

    if ( n<nCount )
    {
        // Das Fenster ist schon bekannt
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
                        pCW->bCreate = sal_False;
                        if ( pChild->IsHideAtToggle() )
                        {
                            ShowChildWindow_Impl( nId, sal_False, bSetFocus );
                        }
                        else
                        {
                            // Fenster soll ausgeschaltet werdem
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
        DBG_ERROR("ChildWindow ist nicht im Kontext!");
    }
    else
    {
        DBG_ERROR("ChildWindow ist nicht registriert!");
    }
#endif
}

//--------------------------------------------------------------------

sal_Bool SfxWorkWindow::HasChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = pChildWins->Count();
    sal_uInt16 n;
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

    return sal_False;
}

sal_Bool SfxWorkWindow::IsFloating( sal_uInt16 nId )
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        sal_uInt16 nCount = pWork->pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        sal_uInt16 nCount = pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
        pCW = new SfxChildWin_Impl( nId );
        pCW->bEnable = sal_False;
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
        return sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxWorkWindow::KnowsChildWindow_Impl(sal_uInt16 nId)
{
    SfxChildWin_Impl *pCW=0;
    sal_uInt16 nCount = pChildWins->Count();
    sal_uInt16 n;
    for (n=0; n<nCount; n++)
    {
        pCW = (*pChildWins)[n];
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

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        sal_uInt16 nCount = pWork->pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        sal_uInt16 nCount = pChildWins->Count();
        for (sal_uInt16 n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                pWork = this;
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
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

void SfxWorkWindow::ShowChildWindow_Impl(sal_uInt16 nId, sal_Bool bVisible, sal_Bool bSetFocus)
{
    sal_uInt16 nCount = pChildWins->Count();
    SfxChildWin_Impl* pCW=0;
    sal_uInt16 n;
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
    nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if ( n<nCount )
    {
        DBG_ERROR("ChildWindow ist nicht im Kontext!");
    }
    else
    {
        DBG_ERROR("ChildWindow ist nicht registriert!");
    }
#endif
}

//--------------------------------------------------------------------

SfxChildWindow* SfxWorkWindow::GetChildWindow_Impl(sal_uInt16 nId)
{
    sal_uInt16 nCount = pChildWins->Count();
    sal_uInt16 n;
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
//  if ( pParent )
//      pParent->ResetChildWindows_Impl();

    for ( sal_uInt16 n = 0; n < pChildWins->Count(); ++n )
    {
        (*pChildWins)[n]->nId = 0;
        (*pChildWins)[n]->bEnable = sal_False;
    }
}

//------------------------------------------------------------------------
// Virtuelle Methode, die die Gr"o\se der Fl"ache (client area) des parent
// windows liefert, in der Child-Fenster angeordnet werden k"onnen.
// in der ClientArea des parent findet.

Rectangle SfxWorkWindow::GetTopRect_Impl()
{
    return Rectangle (Point(), pWorkWin->GetOutputSizePixel() );
}

//------------------------------------------------------------------------
// Virtuelle Methode, die die Gr"o\se der Fl"ache (client area) des parent
// windows liefert, in der Child-Fenster angeordnet werden k"onnen.
// in der ClientArea des parent findet.

Rectangle SfxFrameWorkWin_Impl::GetTopRect_Impl()
{
    return pMasterFrame->GetTopOuterRectPixel_Impl();
}

//------------------------------------------------------------------------
// Virtuelle Methode, um herauszufinden, ob ein Child-Fenster noch Platz
// in der ClientArea des parent findet.

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
    // Den Status vom Presentation mode wollen wir nicht sichern
    if ( IsDockingAllowed() && bInternalDockingAllowed )
        pChild->SaveStatus(rInfo);
}

void SfxWorkWindow::InitializeChild_Impl(SfxChildWin_Impl *pCW)
{
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SFX_APP();
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
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
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
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

void SfxWorkWindow::MakeChildsVisible_Impl( sal_Bool bVis )
{
    if ( pParent )
        pParent->MakeChildsVisible_Impl( bVis );

    bAllChildsVisible = bVis;
    if ( bVis )
    {
        if ( !bSorted )
            Sort_Impl();
        for ( sal_uInt16 n=0; n<aSortedList.Count(); ++n )
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
        for ( sal_uInt16 n=0; n<aSortedList.Count(); ++n )
        {
            SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
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
        // Es werden entweder Dummyfenster oder Fenster im AutoShow-Modus
        // ( nicht gepinned, FadeIn ) behandelt.
        // Nur das "ubergebene Fenster darf unsichtbar sein, denn vielleicht
        // soll daf"ur gerade die Gr"o\se berechnet werden, bevor es angezeigt
        // wird.
        SfxSplitWindow* pSplitWin = pSplit[n];
        sal_Bool bDummyWindow = !pSplitWin->IsFadeIn();
        Window *pDummy = pSplitWin->GetSplitWindow();
        Window *pWin = bDummyWindow ? pDummy : pSplitWin;
        if ( (pSplitWin->IsPinned() && !bDummyWindow) || (!pWin->IsVisible() && pActSplitWin != pSplitWin) )
            continue;

        // Breite und Position des Dummy-Fensters als Ausgangspunkt
        Size aSize = pDummy->GetSizePixel();
        Point aPos = pDummy->GetPosPixel();

        switch ( n )
        {
            case ( 0 ) :
            {
                // Linkes SplitWindow
                // Breite vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                // Wenn links ein Window sichtbar ist, beginnt der freie
                // Bereich rechts davon bzw. bei der Client area
                long nLeft = aPos.X() + aSize.Width();
                if ( nLeft > aArea.Left() )
                    aArea.Left() = nLeft;
                break;
            }
            case ( 1 ) :
            {
                // Rechtes SplitWindow
                // Position um Differenz der Breiten korrigieren
                aPos.X() += aSize.Width();

                // Breite vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                aPos.X() -= aSize.Width();

                // Wenn links schon ein Fenster aufgeklappt ist, darf
                // das rechte nicht dar"uber gehen
                if ( aPos.X() < aArea.Left() )
                {
                    aPos.X() = aArea.Left();
                    aSize.Width() = aArea.GetWidth();
                }

                // Wenn rechts ein Window sichtbar ist, endet der freie
                // Bereich links davon bzw. bei der Client area
                long nRight = aPos.X();
                if ( nRight < aArea.Right() )
                    aArea.Right() = nRight;
                break;
            }
            case ( 2 ) :
            {
                // Oberes SplitWindow
                // H"ohe vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();

                // Breite anpassen, je nachdem ob links oder rechts
                // schon ein Fenster aufgeklappt ist
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // Wenn oben ein Window sichtbar ist, beginnt der freie
                // Bereich darunter bzw. bei der Client Area
                long nTop = aPos.Y() + aSize.Height();
                if ( nTop > aArea.Top() )
                    aArea.Top() = nTop;
                break;
            }
            case ( 3 ) :
            {
                // Das untere SplitWindow
                // Position um Differenz der H"ohen korrigieren
                aPos.Y() += aSize.Height();

                // H"ohe vom Fenster selbst holen, wenn nicht das DummmyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();

                aPos.Y() -= aSize.Height();

                // Breite anpassen, je nachdem ob links oder rechts
                // schon ein Fenster aufgeklappt ist
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // Wenn oben schon ein Fenster aufgeklappt ist, darf
                // das untere nicht dar"uber gehen
                if ( aPos.Y() < aArea.Top() )
                {
                    aPos.Y() = aArea.Top();
                    aSize.Height() = aArea.GetHeight();
                }

                break;
            }
        }

        if ( !bDummyWindow )
            // Das FadeIn-Window ist ein Float, dessen Koordinaten in
            // Screenkoordinaten gesetzt werden
            pSplitWin->SetPosSizePixel( pWorkWin->OutputToScreenPixel(aPos), aSize );
        else
            // Das angedockte DummyWindow
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

SfxChildWinController_Impl::SfxChildWinController_Impl( sal_uInt16 nID, SfxWorkWindow *pWork )
    : SfxControllerItem( nID, pWork->GetBindings() )
    , pWorkwin( pWork )
{}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SfxWorkWindow::CreateDispatch( const String& )
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >();
}

void SfxChildWinController_Impl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* )
{
    pWorkwin->DisableChildWindow_Impl( nSID, eState == SFX_ITEM_DISABLED );
}

void SfxWorkWindow::DisableChildWindow_Impl( sal_uInt16 nId, sal_Bool bDisable )
{
    sal_uInt16 nCount = pChildWins->Count();
    sal_uInt16 n;
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

sal_Bool SfxWorkWindow::ActivateNextChild_Impl( sal_Bool bForward )
{
    // Alle Kinder gem"a\s Liste sortieren
    SvUShorts aList;
    for ( sal_uInt16 i=SFX_OBJECTBAR_MAX; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if ( pCli && pCli->bCanGetFocus && pCli->pWin )
        {
            sal_uInt16 k;
            for (k=0; k<aList.Count(); k++)
                if ( ChildTravelValue((*pChilds)[aList[k]]->eAlign) > ChildTravelValue(pCli->eAlign) )
                    break;
            aList.Insert(i,k);
        }
    }

    if ( aList.Count() == 0 )
        return sal_False;

    sal_uInt16 nTopValue  = ChildTravelValue( SFX_ALIGN_LOWESTTOP );
    for ( sal_uInt16 i=0; i<aList.Count(); i++ )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[i]];
        if ( pCli->pWin && ChildTravelValue( pCli->eAlign ) > nTopValue )
            break;
    }

    sal_uInt16 n = bForward ? 0 : aList.Count()-1;
    SfxChild_Impl *pAct=NULL;
    if ( pActiveChild )
    {
        // Das aktive Fenster suchen
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
        for ( sal_uInt16 i=0; i<SFX_SPLITWINDOWS_MAX; i++ )
        {
            // Eventuell ist pAct ein Splitwindow
            SfxSplitWindow *p = pSplit[i];
            if ( pAct->pWin == p )
            {
                if( p->ActivateNextChild_Impl( bForward ) )
                    return sal_True;
                break;
            }
        }

        // pAct ist ein direktes ChildWindow
        // mit dem Nachfolger bzw. Vorg"anger des aktiven Fensters weitermachen
        if ( bForward )
            n = n+1;
        else
            n = n-1;

        if ( n == 0 || n == aList.Count()-1 )
            return sal_False;
    }

    for( ;; )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[n]];
        if ( pCli->pWin )
        {
            SfxChild_Impl* pNext = pCli;
            for ( sal_uInt16 i=0; n<SFX_SPLITWINDOWS_MAX; n++ )
            {
                // Eventuell ist pNext ein Splitwindow
                SfxSplitWindow *p = pSplit[i];
                if ( pNext->pWin == p )
                {
                    // Das erste/letzte Fenster dort aktivieren
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

        if ( n == 0 || n == aList.Count()-1 )
            break;
    }

    return sal_False;
}

void SfxWorkWindow::SetObjectBarCustomizeMode_Impl( sal_Bool )
{
}

void SfxWorkWindow::DataChanged_Impl( const DataChangedEvent& )
{
    sal_uInt16 n;
    sal_uInt16 nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
    {
        SfxChildWin_Impl*pCW = (*pChildWins)[n];
        if ( pCW && pCW->pWin )
            pCW->pWin->GetWindow()->UpdateSettings( Application::GetSettings() );
    }

    ArrangeChilds_Impl();
}

