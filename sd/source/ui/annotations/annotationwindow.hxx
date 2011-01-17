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

#ifndef _ANNOTATIONWINDOW_HXX
#define _ANNOTATIONWINDOW_HXX

#include <com/sun/star/office/XAnnotation.hpp>

#include <tools/datetime.hxx>
#include <tools/date.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/floatwin.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

#include <svx/sdr/overlay/overlayobject.hxx>
#include <editeng/editstat.hxx>

class OutlinerView;
class Outliner;
class ScrollBar;
class Edit;
class MultiLineEdit;
class PopupMenu;
class SvxLanguageItem;
class OutlinerParaObject;
class SdDrawDocument;

namespace sd {

class AnnotationManagerImpl;
class AnnotationWindow;
class DrawDocShell;
class View;

class AnnotationTextWindow : public Control
{
private:
    OutlinerView*       mpOutlinerView;
    AnnotationWindow*   mpAnnotationWindow;

protected:
    virtual void    Paint( const Rectangle& rRect);
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    LoseFocus();

public:
    AnnotationTextWindow( AnnotationWindow* pParent, WinBits nBits );
    ~AnnotationTextWindow();

    void SetOutlinerView( OutlinerView* pOutlinerView ) { mpOutlinerView = pOutlinerView; }

    virtual XubString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    virtual void    GetFocus();
};


class AnnotationWindow : public FloatingWindow
{
    private:
        AnnotationManagerImpl&  mrManager;
        DrawDocShell*           mpDocShell;
        View*                   mpView;
        SdDrawDocument*         mpDoc;

        OutlinerView*           mpOutlinerView;
        Outliner*               mpOutliner;
        ScrollBar*              mpVScrollbar;
        ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > mxAnnotation;
        bool                    mbReadonly;
        bool                    mbProtected;
        bool                    mbMouseOverButton;
        AnnotationTextWindow*   mpTextWindow;
        MultiLineEdit*          mpMeta;
        Rectangle               maRectMetaButton;
        basegfx::B2DPolygon     maPopupTriangle;

    protected:
        void            SetSizePixel( const Size& rNewSize );

        DECL_LINK(ModifyHdl, void*);
        DECL_LINK(ScrollHdl, ScrollBar*);

    public:
        AnnotationWindow( AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, ::Window* pParent );
        virtual ~AnnotationWindow();

        void StartEdit();

        virtual SvxLanguageItem GetLanguage(void);

        void setAnnotation( const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation, bool bGrabFocus = false );
        const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& getAnnotation() const { return mxAnnotation; }

        void ExecuteSlot( sal_uInt16 nSID );

        ScrollBar*      Scrollbar()     { return mpVScrollbar;}

        DrawDocShell*           DocShell()      { return mpDocShell; }
        OutlinerView*           getView()       { return mpOutlinerView; }
        sd::View*               DocView()       { return mpView; }
        Outliner*               Engine()        { return mpOutliner; }
        SdDrawDocument*         Doc()           { return mpDoc; }

        long            GetPostItTextHeight();

        void            InitControls();
        void            HidePostIt();
        void            DoResize();
        void            ResizeIfNeccessary(long aOldHeight, long aNewHeight);
        void            SetScrollbar();

        void            Rescale();

        bool            IsReadOnly() { return mbReadonly;}

        bool            IsProtected() { return mbProtected; }

        void            SetLanguage(const SvxLanguageItem aNewItem);

        sal_Int32       GetScrollbarWidth();

        void            ToggleInsMode();

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo*);

        virtual void    Deactivate();
        virtual void    Paint( const Rectangle& rRect);
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    Command( const CommandEvent& rCEvt );
        virtual void    GetFocus();

        void            SetColor();

        Color           maColor;
        Color           maColorDark;
        Color           maColorLight;
};

} // namespace sd

#endif
