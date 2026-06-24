/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basecontainercontrol.hxx>
#include <toolkit/controls/unocontrols.hxx>

namespace com::sun::star::awt { class XFixedText; }

namespace unocontrols {

class ProgressBar;

constexpr sal_Int32 PROGRESSMONITOR_FREEBORDER = 10;                                                     // border around and between the controls
inline constexpr OUString PROGRESSMONITOR_DEFAULT_TOPIC  = u""_ustr;
inline constexpr OUString PROGRESSMONITOR_DEFAULT_TEXT = u""_ustr;
constexpr sal_Int32 PROGRESSMONITOR_LINECOLOR_BRIGHT = 0xFFFFFF;             // white
constexpr sal_Int32 PROGRESSMONITOR_LINECOLOR_SHADOW = 0x000000;             // black
constexpr sal_Int32 PROGRESSMONITOR_DEFAULT_WIDTH = 350;
constexpr sal_Int32 PROGRESSMONITOR_DEFAULT_HEIGHT = 100;

/// Item of TextList
struct IMPL_TextlistItem
{
    OUString sTopic;          /// Left site of textline in dialog
    OUString sText;          /// Right site of textline in dialog
};

using ProgressMonitor_BASE = cppu::ImplInheritanceHelper<BaseContainerControl,
                                                         css::awt::XLayoutConstrains,
                                                         css::awt::XButton,
                                                         css::awt::XProgressMonitor>;
class ProgressMonitor final : public ProgressMonitor_BASE
{
public:
    ProgressMonitor( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~ProgressMonitor() override;

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

    virtual void addText(
        const OUString& sTopic ,
        const OUString& sText ,
        bool bbeforeProgress
    ) override;

    virtual void removeText(
        const OUString& sTopic ,
        bool bbeforeProgress
    ) override;

    virtual void updateText(
        const OUString&  sTopic ,
        const OUString&  sText ,
        bool bbeforeProgress
    ) override;

    //  XProgressBar

    virtual void setForegroundColor( sal_Int32 nColor ) override;

    virtual void setBackgroundColor( sal_Int32 nColor ) override;

    virtual void setValue( sal_Int32 nValue ) override;

    virtual void setRange( sal_Int32   nMin    ,
                                    sal_Int32   nMax    ) override;

    virtual sal_Int32 getValue() override;

    //  XButton

    virtual void addActionListener(
        const css::uno::Reference< css::awt::XActionListener >& xListener
    ) override;

    virtual void removeActionListener(
        const css::uno::Reference< css::awt::XActionListener >& xListener
    ) override;

    virtual void setLabel( const OUString& sLabel ) override;

    virtual void setActionCommand( const OUString& sCommand ) override;

    //  XLayoutConstrains

    virtual css::awt::Size getMinimumSize() override;

    virtual css::awt::Size getPreferredSize() override;

    virtual css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    //  XControl

    virtual void createPeer(
        const css::uno::Reference< css::awt::XToolkit     >& xToolkit ,
        const css::uno::Reference< css::awt::XWindowPeer  >& xParent
    ) override;

    virtual bool setModel( const css::uno::Reference< css::awt::XControlModel >& xModel ) override;

    virtual css::uno::Reference< css::awt::XControlModel > getModel() override;

    //  XComponent

    virtual void dispose() override;

    //  XWindow

    virtual void setPosSize(   sal_Int32   nX      ,
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
    ::std::vector < IMPL_TextlistItem > maTextlist_Top;         // Elements before progress
    rtl::Reference< UnoFixedTextControl >   m_xTopic_Top;   // (used, if parameter "beforeProgress"=true in "addText, updateText, removeText")
    rtl::Reference< UnoFixedTextControl >   m_xText_Top;

    ::std::vector < IMPL_TextlistItem > maTextlist_Bottom;      // Elements below of progress
    rtl::Reference< UnoFixedTextControl >   m_xTopic_Bottom;   // (used, if parameter "beforeProgress"=false in "addText, updateText, removeText")
    rtl::Reference< UnoFixedTextControl >   m_xText_Bottom;

    rtl::Reference<ProgressBar>                   m_xProgressBar;
    rtl::Reference< UnoButtonControl >            m_xButton;
    css::awt::Rectangle                           m_a3DLine;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
