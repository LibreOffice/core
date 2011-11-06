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


#ifndef _HEXPLWND_HXX
#define _HEXPLWND_HXX

// includes --------------------------------------------------------------

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>

// forward ---------------------------------------------------------------

class SfxBeamerUnoFrame_Impl;

// class SfxExplorerHorizChildWnd_Impl -----------------------------------

class SfxExplorerHorizChildWnd_Impl : public SfxChildWindow
{
public:
    SfxExplorerHorizChildWnd_Impl( Window* pParent, sal_uInt16 nId,
                                   SfxBindings* pBindings,
                                   SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SfxExplorerHorizChildWnd_Impl);


    virtual sal_Bool        QueryClose();
};

// class SfxExplorerDockWnd_Impl -----------------------------------------

class SfxExplorerHorizDockWnd_Impl : public SfxDockingWindow
{
private:
    SfxBeamerUnoFrame_Impl* _pFrame;

protected:
    virtual void            GetFocus();
    virtual long            Notify( NotifyEvent& rNEvt );

public:
    SfxExplorerHorizDockWnd_Impl( SfxBindings* pBindings,
                                  SfxChildWindow* pChildWin,
                                  Window* pParent, const ResId& rResId );
    ~SfxExplorerHorizDockWnd_Impl();

    virtual void            FillInfo( SfxChildWinInfo& ) const;
    virtual void            FadeIn( sal_Bool );
    sal_Bool                    QueryClose();
    void                    SetURL( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );
    const ::com::sun::star::util::URL&              GetURL() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             GetFrameInterface();
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    GetArgs() const;
};


#endif

