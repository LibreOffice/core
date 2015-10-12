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

#ifndef INCLUDED_SD_SOURCE_UI_ANNOTATIONS_ANNOTATIONTAG_HXX
#define INCLUDED_SD_SOURCE_UI_ANNOTATIONS_ANNOTATIONTAG_HXX

#include <com/sun/star/office/XAnnotation.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "smarttag.hxx"

namespace sd {

class View;
class AnnotationManagerImpl;
class AnnotationWindow;

class AnnotationTag : public SmartTag
{
public:
    AnnotationTag( AnnotationManagerImpl& rManager, ::sd::View& rView, const css::uno::Reference< css::office::XAnnotation >& xAnnotation, Color& rColor, int nIndex, const vcl::Font& rFont );
    virtual ~AnnotationTag();

    /// @return true if the SmartTag handled the event.
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& ) override;

    /// @return true if the SmartTag consumes this event.
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    /// @return true if the SmartTag consumes this event.
    virtual bool RequestHelp( const HelpEvent& rHEvt ) override;

    /// @return true if the SmartTag consumes this event.
    virtual bool Command( const CommandEvent& rCEvt ) override;

    // callbacks from sdr view
    virtual sal_uLong GetMarkablePointCount() const override;
    virtual sal_uLong GetMarkedPointCount() const override;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark=false) override;
    virtual void CheckPossibilities() override;
    virtual bool MarkPoints(const Rectangle* pRect, bool bUnmark) override;

    void Move( int nDX, int nDY );
    bool OnMove( const KeyEvent& rKEvt );

    BitmapEx CreateAnnotationBitmap(bool);

    css::uno::Reference< css::office::XAnnotation > GetAnnotation() const { return mxAnnotation; }

    void OpenPopup( bool bEdit );
    void ClosePopup();

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList ) override;
    virtual bool getContext( SdrViewContext& rContext ) override;
    virtual void disposing() override;
    virtual void select() override;
    virtual void deselect() override;

    DECL_LINK_TYPED( WindowEventHandler, VclWindowEvent&, void );
    DECL_LINK_TYPED( ClosePopupHdl, void*, void );

private:
    AnnotationManagerImpl& mrManager;
    css::uno::Reference< css::office::XAnnotation > mxAnnotation;
    VclPtr<AnnotationWindow>                        mpAnnotationWindow;
    Color                                           maColor;
    int                                             mnIndex;
    const vcl::Font&                              mrFont;
    Size                                            maSize;
    ImplSVEvent *                                   mnClosePopupEvent;
    VclPtr<vcl::Window>                             mpListenWindow;
    Point                                           maMouseDownPos;
};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_ANNOTATIONS_ANNOTATIONTAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
