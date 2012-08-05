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

#ifndef _SD_ANNOTATIONTAG_HXX_
#define _SD_ANNOTATIONTAG_HXX_

#include <com/sun/star/office/XAnnotation.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "smarttag.hxx"

namespace css = ::com::sun::star;

namespace sd {

class View;
class AnnotationManagerImpl;
class AnnotationWindow;

class AnnotationTag : public SmartTag
{
public:
    AnnotationTag( AnnotationManagerImpl& rManager, ::sd::View& rView, const css::uno::Reference< css::office::XAnnotation >& xAnnotation, Color& rColor, int nIndex, const Font& rFont );
    virtual ~AnnotationTag();

    /// @return true if the SmartTag handled the event.
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /// @return true if the SmartTag consumes this event.
    virtual bool KeyInput( const KeyEvent& rKEvt );

    /// @return true if the SmartTag consumes this event.
    virtual bool RequestHelp( const HelpEvent& rHEvt );

    /// @return true if the SmartTag consumes this event.
    virtual bool Command( const CommandEvent& rCEvt );

    // callbacks from sdr view
    virtual sal_uLong GetMarkablePointCount() const;
    virtual sal_uLong GetMarkedPointCount() const;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False);
    virtual void CheckPossibilities();
    virtual sal_Bool MarkPoints(const Rectangle* pRect, sal_Bool bUnmark);

    void Move( int nDX, int nDY );
    bool OnMove( const KeyEvent& rKEvt );

    BitmapEx CreateAnnotationBitmap(bool);

    css::uno::Reference< css::office::XAnnotation > GetAnnotation() const { return mxAnnotation; }

    void OpenPopup( bool bEdit );
    void ClosePopup();

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual bool getContext( SdrViewContext& rContext );
    virtual void disposing();
    virtual void select();
    virtual void deselect();

    DECL_LINK( WindowEventHandler, VclWindowEvent* );
    DECL_LINK( ClosePopupHdl, void* );

private:
    AnnotationManagerImpl& mrManager;
    css::uno::Reference< css::office::XAnnotation > mxAnnotation;
    std::auto_ptr<AnnotationWindow>                 mpAnnotationWindow;
    Color                                           maColor;
    int                                             mnIndex;
    const Font&                                     mrFont;
    Size                                            maSize;
    sal_uLong                                           mnClosePopupEvent;
    ::Window*                                       mpListenWindow;
    Point                                           maMouseDownPos;
};

} // end of namespace sd

#endif      // _SD_ANNOTATIONTAG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
