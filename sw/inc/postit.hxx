/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:34:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _POSTIT_HXX
#define _POSTIT_HXX

#ifndef _WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

//TODO: move to cxx
// does not work with forward declaration, why??
#ifndef _LINEINFO_HXX
#include <vcl/lineinfo.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

class SwPostItMgr;
class SwPostItField;
class SwFmtFld;
class OutlinerView;
class Outliner;
class ScrollBar;
class SwEditWin;
class SwView;
class SwPostIt;
class Edit;
class MultiLineEdit;
class SwRect;
class PopupMenu;

class SwPostItAnkor: public sdr::overlay::OverlayObjectWithBasePosition
{
    protected:
        /*                        6------------7
             1                   /
            /4\ ---------------5
           2 - 3
        */

        basegfx::B2DPoint                       maSecondPosition;
        basegfx::B2DPoint                       maThirdPosition;
        basegfx::B2DPoint                       maFourthPosition;
        basegfx::B2DPoint                       maFifthPosition;
        basegfx::B2DPoint                       maSixthPosition;
        basegfx::B2DPoint                       maSeventhPosition;

    private:
        // object's geometry
        basegfx::B2DPolygon                     maTriangle;
        basegfx::B2DPolygon                     maLine;
        basegfx::B2DPolygon                     maLineTop;
        LineInfo                                mLineInfo;
        unsigned long                           mHeight;
        bool                                    mbShadowedEffect;
    protected:
        // helpers to fill and reset geometry
        void implEnsureGeometry();
        void implResetGeometry();

        // helpers to paint geometry
        void implDrawGeometry(OutputDevice& rOutputDevice, Color aColor, double fOffX, double fOffY);
        Color implBlendColor(const Color aOriginal, sal_Int16 nChange);

        virtual void drawGeometry(OutputDevice& rOutputDevice);
        virtual void createBaseRange(OutputDevice& rOutputDevice);

    public:
        SwPostItAnkor(const basegfx::B2DPoint& rBasePos,
                        const basegfx::B2DPoint& rSecondPos,
                        const basegfx::B2DPoint& rThirdPos,
                        const basegfx::B2DPoint& rFourthPos,
                        const basegfx::B2DPoint& rFifthPos,
                        const basegfx::B2DPoint& rSixthPos,
                        const basegfx::B2DPoint& rSeventhPos,
                        Color aBaseColor,
                        const LineInfo &aLineInfo,
                        bool bShadowedEffect);
        virtual ~SwPostItAnkor();

        const basegfx::B2DPoint& GetSecondPosition() const { return maSecondPosition; }
        const basegfx::B2DPoint& GetThirdPosition() const { return maThirdPosition; }
        const basegfx::B2DPoint& GetFourthPosition() const { return maFourthPosition; }
        const basegfx::B2DPoint& GetFifthPosition() const { return maFifthPosition; }
        const basegfx::B2DPoint& GetSixthPosition() const { return maSixthPosition; }
        const basegfx::B2DPoint& GetSeventhPosition() const { return maSeventhPosition; }

        void SetAllPosition(const basegfx::B2DPoint& rPoint1, const basegfx::B2DPoint& rPoint2, const basegfx::B2DPoint& rPoint3,
            const basegfx::B2DPoint& rPoint4, const basegfx::B2DPoint& rPoint5, const basegfx::B2DPoint& rPoint6, const basegfx::B2DPoint& rPoint7);
        void SetColorLineInfo(Color aBaseColor,const LineInfo& aLineInfo);
        void SetSecondPosition(const basegfx::B2DPoint& rNew);
        void SetThirdPosition(const basegfx::B2DPoint& rNew);
        void SetFourthPosition(const basegfx::B2DPoint& rNew);
        void SetFifthPosition(const basegfx::B2DPoint& rNew);
        void SetSixthPosition(const basegfx::B2DPoint& rNew);
        void SetSeventhPosition(const basegfx::B2DPoint& rNew);

        void SetLineInfo(const LineInfo &aLineInfo);

        void SetHeight(const unsigned long aHeight) {mHeight = aHeight;};

        bool getShadowedEffect() const { return mbShadowedEffect; }
        void setShadowedEffect(bool bNew);


        //sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol) const;
        // transform object coordinates. Transforms maBasePosition
        // and invalidates on change
        virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
};

class PostItTxt : public Window
{
    private:
        OutlinerView*   mpOutlinerView;
        SwPostIt*       mpPostIt;

        Color           mColorDark;
        Color           mColorLight;
        bool            mMouseOver;
        BOOL            mbShowPopup;

    protected:
        virtual void    Paint( const Rectangle& rRect);
        virtual void    KeyInput( const KeyEvent& rKeyEvt );
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Command( const CommandEvent& rCEvt );
        virtual void    DataChanged( const DataChangedEvent& aData);
        virtual void    LoseFocus();
    public:
        virtual void    GetFocus();
        void            SetColor(Color &aColorDark,Color &aColorLight);

    public:
            PostItTxt(Window* pParent, WinBits nBits);
            ~PostItTxt();

            void    SetTextView( OutlinerView* aEditView ) {    mpOutlinerView = aEditView; }
            DECL_LINK( WindowEventListener, VclSimpleEvent* );
};

class SwPostIt : public Window
{
    private:
        SwView*         mpView;
        sdr::overlay::OverlayManager* pOverlayManager;
        OutlinerView*   mpOutlinerView;
        Outliner*       mpOutliner;
        PostItTxt*      mpPostItTxt;
        MultiLineEdit*  mpMeta;
        ScrollBar*      mpVScrollbar;
        SwFmtFld*       mpFmtFld;
        SwPostItField*  mpFld;
        SwPostItAnkor*  mpAnkor;
        SwPostItMgr*    mpMgr;
        bool            mbMeta;
        bool            mbReadonly;
        Color           mColorAnkor;
        Color           mColorDark;
        Color           mColorLight;
        basegfx::B2DPolygon aPopupTriangle;
        Rectangle       mRectMetaButton;
        PopupMenu*      mpButtonPopup;
        sal_Int32       mnEventId;
        bool            mbMarginSide;

    protected:

        virtual void    DataChanged( const DataChangedEvent& aEvent);
        virtual void    LoseFocus();
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    Paint( const Rectangle& rRect);
        virtual void    Resize();
        virtual void    GetFocus();

        DECL_LINK(ScrollHdl, ScrollBar*);
        void            InitControls();

    public:
        SwPostIt( Window* pParent, WinBits nBits,SwFmtFld* aField,SwPostItMgr* aMgr,bool bMarginSide);
        ~SwPostIt();

        void    SetSizePixel( const Size& rNewSize );
        void    SetPosSizePixelRect( long nX, long nY,long nWidth, long nHeight,const SwRect &aRect,const long PageBorder, USHORT nFlags = WINDOW_POSSIZE_ALL );
        void    TranslateTopPosition(const long aAmount);

        void    MetaInfo(const bool bMeta);
        void    SetPostItText();

        PostItTxt*      PostItText()    { return mpPostItTxt;}
        ScrollBar*      Scrollbar()     { return mpVScrollbar;}
        SwPostItAnkor*  Ankor()         { return mpAnkor;}
        OutlinerView*   View()          { return mpOutlinerView;}
        SwView*         DocView()       { return mpView;}
        Outliner*       Engine()        { return mpOutliner;}
        SwPostItMgr*    Mgr()           { return mpMgr; }
        SwEditWin*      EditWin();

        long            GetPostItTextHeight();
        void            UpdateData();

        void            SwitchToPostIt(USHORT aDirection);
        void            SwitchToFieldPos(bool bAfter = true);

        void            ExecuteCommand(USHORT aSlot);
        void            Delete(USHORT aType);
        void            Hide(USHORT aType);
        void            DoResize() { Resize(); }

        void            ShowAnkorOnly(const Point &aPoint);
        void            ShowNote();
        void            HideNote();

        void            ResetAttributes();

        void            SetReadonly(BOOL bSet);
        BOOL            IsReadOnly()        { return mbReadonly;}

        void            SetColor(Color &aColorDark,Color &aColorLight, Color &aColorAnkor);
        Color           ColorDark();
        Color           ColorLight();
        Color           ColorAnkor();

        void            Rescale();

        sal_Int32       GetMetaHeight();
        sal_Int32       GetMinimumSizeWithMeta();
        sal_Int32       GetMinimumSizeWithoutMeta();
        sal_Int32       GetMetaButtonAreaWidth();
        sal_Int32       GetScrollbarWidth();

        void            SetSpellChecking(bool bEnable);

        /*
        //void          ClearModifyFlag()   { mpOutliner->SetModified(); }
        //BOOL          IsModified() const  { return mpOutliner->IsModified();}

        void            SetStartLine(USHORT nLine){nStartLine = nLine;}

        virtual void    Command( const CommandEvent& rCEvt );
        void            HandleWheelCommand( const CommandEvent& rCEvt );

        void            SetTextEncoding(rtl_TextEncoding eEncoding);
        */
};


#endif
