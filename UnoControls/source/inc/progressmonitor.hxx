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

#pragma once

#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <rtl/ref.hxx>

#include <vector>
#include <memory>

#include <basecontainercontrol.hxx>

namespace com::sun::star::awt { class XFixedText; }
namespace com::sun::star::awt { class XControlModel; }

namespace unocontrols {

class ProgressBar;

#define PROGRESSMONITOR_FREEBORDER                      10                                                      // border around and between the controls
#define PROGRESSMONITOR_DEFAULT_TOPIC                   ""
#define PROGRESSMONITOR_DEFAULT_TEXT                    ""
#define PROGRESSMONITOR_LINECOLOR_BRIGHT                sal_Int32(Color( 0xFF, 0xFF, 0xFF ))             // white
#define PROGRESSMONITOR_LINECOLOR_SHADOW                sal_Int32(Color( 0x00, 0x00, 0x00 ))             // black
#define PROGRESSMONITOR_DEFAULT_WIDTH                   350
#define PROGRESSMONITOR_DEFAULT_HEIGHT                  100

/// Item of TextList
struct IMPL_TextlistItem
{
    OUString sTopic;          /// Left site of textline in dialog
    OUString sText;          /// Right site of textline in dialog
};

class ProgressMonitor final : public css::awt::XLayoutConstrains
                        , public css::awt::XButton
                        , public css::awt::XProgressMonitor
                        , public BaseContainerControl
{
public:
    ProgressMonitor( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~ProgressMonitor() override;

    //  XInterface

    /**
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider

    /**
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    //  XAggregation

    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType ) override;

    //  XProgressMonitor

    /**
        @short      add topic to dialog
        @descr      Add a topic with a text in right textlist (used for FixedText-member).<BR>
                    ( "beforeProgress" fix the right list ). The dialog metric is recalculated.

        @seealso    removeText(), updateText()

        @param      sTopic              Name of topic<BR>
                                         [sTopic != "" && sTopic != NULL]
        @param      sText               Value of topic<BR>
                                         [sText != "" && sText != NULL]
        @param      bbeforeProgress     Position of topic<BR>
                                         [True => before progressbar / False => below progressbar]
        @onerror    DEBUG   = Assertion<BR>
                    RELEASE = nothing
    */

    virtual void SAL_CALL addText(
        const OUString& sTopic ,
        const OUString& sText ,
        sal_Bool bbeforeProgress
    ) override;

    virtual void SAL_CALL removeText(
        const OUString& sTopic ,
        sal_Bool bbeforeProgress
    ) override;

    virtual void SAL_CALL updateText(
        const OUString&  sTopic ,
        const OUString&  sText ,
        sal_Bool bbeforeProgress
    ) override;

    //  XProgressBar

    virtual void SAL_CALL setForegroundColor( sal_Int32 nColor ) override;

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor ) override;

    virtual void SAL_CALL setValue( sal_Int32 nValue ) override;

    virtual void SAL_CALL setRange( sal_Int32   nMin    ,
                                    sal_Int32   nMax    ) override;

    virtual sal_Int32 SAL_CALL getValue() override;

    //  XButton

    virtual void SAL_CALL addActionListener(
        const css::uno::Reference< css::awt::XActionListener >& xListener
    ) override;

    virtual void SAL_CALL removeActionListener(
        const css::uno::Reference< css::awt::XActionListener >& xListener
    ) override;

    virtual void SAL_CALL setLabel( const OUString& sLabel ) override;

    virtual void SAL_CALL setActionCommand( const OUString& sCommand ) override;

    //  XLayoutConstrains

    virtual css::awt::Size SAL_CALL getMinimumSize() override;

    virtual css::awt::Size SAL_CALL getPreferredSize() override;

    virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit     >& xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer  >& xParent
    ) override;

    virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& xModel ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    //  XWindow

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) override;

private:
    virtual void impl_paint( sal_Int32 nX ,
                             sal_Int32 nY ,
                             const css::uno::Reference< css::awt::XGraphics >& xGraphics ) override;

    using BaseControl::impl_recalcLayout;

    void impl_recalcLayout();

    void impl_rebuildFixedText();

    void impl_cleanMemory();

    IMPL_TextlistItem* impl_searchTopic( std::u16string_view sTopic , bool bbeforeProgress );

// debug methods

    static bool impl_debug_checkParameter( std::u16string_view sTopic, std::u16string_view sText );    // addText, updateText
    static bool impl_debug_checkParameter( std::u16string_view rTopic );                           // removeText

// private variables

private:
    ::std::vector < std::unique_ptr<IMPL_TextlistItem> > maTextlist_Top;         // Elements before progress
    css::uno::Reference< css::awt::XFixedText >   m_xTopic_Top;   // (used, if parameter "beforeProgress"=true in "addText, updateText, removeText")
    css::uno::Reference< css::awt::XFixedText >   m_xText_Top;

    ::std::vector < std::unique_ptr<IMPL_TextlistItem> > maTextlist_Bottom;      // Elements below of progress
    css::uno::Reference< css::awt::XFixedText >   m_xTopic_Bottom;   // (used, if parameter "beforeProgress"=false in "addText, updateText, removeText")
    css::uno::Reference< css::awt::XFixedText >   m_xText_Bottom;

    rtl::Reference<ProgressBar>                   m_xProgressBar;
    css::uno::Reference< css::awt::XButton >      m_xButton;
    css::awt::Rectangle                           m_a3DLine;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
