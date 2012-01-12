/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _VCL_GTKINST_HXX
#define _VCL_GTKINST_HXX

#include <unx/salinst.h>
#include <unx/salsys.h>

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
    std::list<sal_uLong> aYieldStack;
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

    virtual SalFrame*           CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual SalSystem*          CreateSalSystem();
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
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
