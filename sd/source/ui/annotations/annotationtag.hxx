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

    /** returns true if the SmartTag handled the event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool RequestHelp( const HelpEvent& rHEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool Command( const CommandEvent& rCEvt );

    // callbacks from sdr view
    virtual sal_uLong GetMarkablePointCount() const;
    virtual sal_uLong GetMarkedPointCount() const;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False);
    virtual void CheckPossibilities();
    virtual sal_Bool MarkPoints(const Rectangle* pRect, sal_Bool bUnmark);

    void Move( int nDX, int nDY );
    bool OnMove( const KeyEvent& rKEvt );

    // ---

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

