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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
