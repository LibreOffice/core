/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gtkinst.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:39:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_GTKINST_HXX
#define _VCL_GTKINST_HXX

#include <salinst.h>
#include <salsys.h>

class GtkYieldMutex : public SalYieldMutex
{
public:
                                                GtkYieldMutex();
    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    virtual int Grab();
    virtual void Ungrab( int );

    class GtkYieldGuard
    {
        GtkYieldMutex*  m_pMutex;
        int             m_nGrab;
    public:
        GtkYieldGuard( GtkYieldMutex* pMutex )
                : m_pMutex( pMutex )
        {
            m_nGrab = m_pMutex->Grab();
        }
        ~GtkYieldGuard()
        {
            m_pMutex->Ungrab( m_nGrab );
        }
    };
};

class GtkHookedYieldMutex : public GtkYieldMutex
{
    virtual int      Grab()             { return 0; };
    virtual void     Ungrab(int )       {};
    std::list<ULONG> aYieldStack;
public:
    GtkHookedYieldMutex();
    virtual void      acquire();
    virtual void      release();
    virtual sal_Bool  tryToAcquire() { return SalYieldMutex::tryToAcquire(); }
    void ThreadsEnter();
    void ThreadsLeave();
};


#define GTK_YIELD_GRAB() GtkYieldMutex::GtkYieldGuard aLocalGtkYieldGuard( static_cast<GtkYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex()) )

class GtkInstance : public X11SalInstance
{
public:
    GtkInstance( SalYieldMutex* pMutex )
            : X11SalInstance( pMutex )
    {}
    virtual ~GtkInstance();

    virtual SalFrame*           CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE );
    virtual SalSystem*          CreateSalSystem();
};

class GtkSalSystem : public X11SalSystem
{
public:
    GtkSalSystem() : X11SalSystem() {}
    virtual ~GtkSalSystem();
    virtual int ShowNativeDialog( const String& rTitle,
                                  const String& rMessage,
                                  const std::list< String >& rButtons,
                                  int nDefButton );
};

#endif // _VCL_GTKINST_HXX
