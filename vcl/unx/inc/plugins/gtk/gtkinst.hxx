/*************************************************************************
 *
 *  $RCSfile: gtkinst.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:55:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


#define GTK_YIELD_GRAB() GtkYieldMutex::GtkYieldGuard aLocalGtkYieldGuard( static_cast<GtkYieldMutex*>(GetSalData()->pInstance_->GetYieldMutex()) )

class GtkInstance : public X11SalInstance
{
public:
    GtkInstance( SalYieldMutex* pMutex )
            : X11SalInstance( pMutex )
    {}
    virtual ~GtkInstance();

    virtual SalFrame*           CreateFrame( SalFrame* pParent, ULONG nStyle );
    virtual SalObject*          CreateObject( SalFrame* pParent );
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
