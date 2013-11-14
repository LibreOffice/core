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


#ifndef _SVX_DLG_CTRL_HXX
#define _SVX_DLG_CTRL_HXX

// include ---------------------------------------------------------------

#include <svtools/ctrlbox.hxx>
#include <sfx2/tabdlg.hxx>
#include "svx/svxdllapi.h"
#include <svx/rectenum.hxx>
#include <vcl/graph.hxx>

#ifndef _XTABLE_HXX
class XBitmapEntry;
class XColorEntry;
class XDash;
class XDashEntry;
class XGradient;
class XGradientEntry;
class XHatch;
class XHatchEntry;
class XLineEndEntry;
class XFillAttrSetItem;
#endif

class XOBitmap;
class XOutdevItemPool;

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
} } } }

/*************************************************************************
|*
|* Von SfxTabPage abgeleitet, um vom Control ueber virtuelle Methode
|* benachrichtigt werden zu koennen.
|*
\************************************************************************/
class SvxTabPage : public SfxTabPage
{

public:
    SvxTabPage( Window* pParent, ResId Id, const SfxItemSet& rInAttrs  ) :
        SfxTabPage( pParent, Id, rInAttrs ) {}

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) = 0;
};

/*************************************************************************
|*
|*  Control zur Darstellung und Auswahl der Eckpunkte (und Mittelpunkt)
|*  eines Objekts
|*
\************************************************************************/
typedef sal_uInt16 CTL_STATE;
#define CS_NOHORZ   1       // no horizontal input information is used
#define CS_NOVERT   2       // no vertikal input information is used

class SvxRectCtlAccessibleContext;
class SvxPixelCtlAccessible; // IAccessibility2 implementation 2009

class SVX_DLLPUBLIC SvxRectCtl : public Control
{
private:
    SVX_DLLPRIVATE void             InitSettings( sal_Bool bForeground, sal_Bool bBackground );
    SVX_DLLPRIVATE void             InitRectBitmap( void );
    SVX_DLLPRIVATE Bitmap&          GetRectBitmap( void );
    SVX_DLLPRIVATE void             Resize_Impl();

protected:
    SvxRectCtlAccessibleContext*    pAccContext;
    sal_uInt16                          nBorderWidth;
    sal_uInt16                          nRadius;
    Size                            aSize;
    Point                           aPtLT, aPtMT, aPtRT;
    Point                           aPtLM, aPtMM, aPtRM;
    Point                           aPtLB, aPtMB, aPtRB;
    Point                           aPtNew;
    RECT_POINT                      eRP, eDefRP;
    CTL_STYLE                       eCS;
    Bitmap*                         pBitmap;
    CTL_STATE                       m_nState;

    // #103516# Added a possibility to completely disable this control
    sal_Bool                        mbCompleteDisable;

    RECT_POINT          GetRPFromPoint( Point ) const;
    Point               GetPointFromRP( RECT_POINT ) const;
    void                SetFocusRect( const Rectangle* pRect = NULL );      // pRect == NULL -> calculate rectangle in method
    Point               SetActualRPWithoutInvalidate( RECT_POINT eNewRP );  // returns the last point

    virtual void        GetFocus();
    virtual void        LoseFocus();

    Point               GetApproxLogPtFromPixPt( const Point& rRoughPixelPoint ) const;
public:
    SvxRectCtl( Window* pParent, const ResId& rResId, RECT_POINT eRpt = RP_MM,
                sal_uInt16 nBorder = 200, sal_uInt16 nCircle = 80, CTL_STYLE eStyle = CS_RECT );
    virtual ~SvxRectCtl();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKeyEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        Resize();

    void                Reset();
    RECT_POINT          GetActualRP() const;
    void                SetActualRP( RECT_POINT eNewRP );

    void                SetState( CTL_STATE nState );

    sal_uInt8               GetNumOfChilds( void ) const;   // returns number of usable radio buttons

    Rectangle           CalculateFocusRectangle( void ) const;
    Rectangle           CalculateFocusRectangle( RECT_POINT eRectPoint ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    RECT_POINT          GetApproxRPFromPixPt( const ::com::sun::star::awt::Point& rPixelPoint ) const;

    // #103516# Added a possibility to completely disable this control
    sal_Bool IsCompletelyDisabled() const { return mbCompleteDisable; }
    void DoCompletelyDisable(sal_Bool bNew);
};

/*************************************************************************
|*
|*  Control zur Darstellung und Auswahl des Winkels der Eckpunkte
|*  eines Objekts
|*
\************************************************************************/
class SvxAngleCtl : public SvxRectCtl
{
private:
    void    Initialize();

protected:
    Font    aFont;
    Size    aFontSize;
    sal_Bool    bPositive;

public:
            SvxAngleCtl( Window* pParent, const ResId& rResId );
            SvxAngleCtl( Window* pParent, const ResId& rResId, Size aSize );

    void    ChangeMetric()
                { bPositive = !bPositive; }
    virtual void Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|*  Preview-Control zur Darstellung von Bitmaps
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxBitmapCtl
{
protected:
    Size            aSize;
    sal_uInt16          nLines;
    Color           aPixelColor, aBackgroundColor;
    const sal_uInt16*   pBmpArray;

public:
            SvxBitmapCtl( Window* pParent, const Size& rSize );
            ~SvxBitmapCtl();

    BitmapEx GetBitmapEx();

    void    SetBmpArray( const sal_uInt16* pPixel ) { pBmpArray = pPixel; }
    void    SetLines( sal_uInt16 nLns ) { nLines = nLns; }
    void    SetPixelColor( Color aColor ) { aPixelColor = aColor; }
    void    SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

/*************************************************************************
|*
|*  Control zum Editieren von Bitmaps
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxPixelCtl : public Control
{
private:
    using OutputDevice::SetLineColor;

protected:
    sal_uInt16      nLines, nSquares;
    Color       aPixelColor;
    Color       aBackgroundColor;
    Color       aLineColor;
    Size        aRectSize;
    sal_uInt16*     pPixel;
    sal_Bool        bPaintable;
    // IAccessibility2 implementation 2009. ------
    //Solution:Add member identifying position
    Point       aFocusPosition;
    Rectangle   implCalFocusRect( const Point& aPosition );
    // ------ IAccessibility2 implementation 2009.
    void    ChangePixel( sal_uInt16 nPixel );

public:
            SvxPixelCtl( Window* pParent, const ResId& rResId,
                        sal_uInt16 nNumber = 8 );
            ~SvxPixelCtl();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void    SetXBitmap( const BitmapEx& rBitmapEx );

    void    SetPixelColor( const Color& rCol ) { aPixelColor = rCol; }
    void    SetBackgroundColor( const Color& rCol ) { aBackgroundColor = rCol; }
    void    SetLineColor( const Color& rCol ) { aLineColor = rCol; }

    sal_uInt16  GetLineCount() const { return nLines; }
    Color   GetPixelColor() const { return aPixelColor; }
    Color   GetBackgroundColor() const { return aBackgroundColor; }

    sal_uInt16  GetBitmapPixel( const sal_uInt16 nPixelNumber );
    sal_uInt16* GetBitmapPixelPtr() { return pPixel; }

    void    SetPaintable( sal_Bool bTmp ) { bPaintable = bTmp; }
    void    Reset();
    // IAccessibility2 implementation 2009. ------
    SvxPixelCtlAccessible*  m_pAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >        m_xAccess;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    long GetSquares() const { return nSquares ; }
    long GetWidth() const { return aRectSize.getWidth() ; }
    long GetHeight() const { return aRectSize.getHeight() ; }

    //Device Pixel .
    long ShowPosition( const Point &pt);

    long PointToIndex(const Point &pt) const;
    Point IndexToPoint(long nIndex) const ;
    long GetFoucsPosIndex() const ;
    //Solution:Keyboard fucntion for key input and focus handling function
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();
    // ------ IAccessibility2 implementation 2009.
};

/*************************************************************************
|*
|* ColorLB kann mit Farben und Namen gefuellt werden
|*
\************************************************************************/

class XColorList;
typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;

class SVX_DLLPUBLIC ColorLB : public ColorListBox
{

public:
         ColorLB( Window* pParent, ResId Id ) : ColorListBox( pParent, Id ) {}
         ColorLB( Window* pParent, WinBits aWB ) : ColorListBox( pParent, aWB ) {}

    virtual void Fill( const XColorListSharedPtr aTab );

    void Append( const XColorEntry& rEntry );
    void Modify( const XColorEntry& rEntry, sal_uInt16 nPos );
};

/*************************************************************************
|*
|* HatchingLB
|*
\************************************************************************/

class XHatchList;
typedef ::boost::shared_ptr< XHatchList > XHatchListSharedPtr;

class SVX_DLLPUBLIC HatchingLB : public ListBox
{
public:
     explicit HatchingLB( Window* pParent, ResId Id);
     explicit HatchingLB( Window* pParent, WinBits aWB);

    virtual void Fill( const XHatchListSharedPtr aList );

    void    Append( const XHatchEntry& rEntry, const Bitmap& rBitmap );
    void    Modify( const XHatchEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
    void    SelectEntryByList( const XHatchListSharedPtr aList, const String& rStr, const XHatch& rXHatch, sal_uInt16 nDist = 0 );
};

/*************************************************************************
|*
|* GradientLB
|*
\************************************************************************/

class XGradientList;
typedef ::boost::shared_ptr< XGradientList > XGradientListSharedPtr;

class SVX_DLLPUBLIC GradientLB : public ListBox
{
public:
    explicit GradientLB( Window* pParent, ResId Id);
    explicit GradientLB( Window* pParent, WinBits aWB);

    virtual void Fill( const XGradientListSharedPtr aList );

    void    Append( const XGradientEntry& rEntry, const Bitmap& rBitmap );
    void    Modify( const XGradientEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
    void    SelectEntryByList( const XGradientListSharedPtr aList, const String& rStr, const XGradient& rXGradient, sal_uInt16 nDist = 0 );

private:
    XGradientListSharedPtr maList;
};

/*************************************************************************
|*
|* BitmapLB
|*
\************************************************************************/

class XBitmapList;
typedef ::boost::shared_ptr< XBitmapList > XBitmapListSharedPtr;

class SVX_DLLPUBLIC BitmapLB : public ListBox
{
public:
    explicit BitmapLB(Window* pParent, ResId Id);

    virtual void Fill(const XBitmapListSharedPtr aList);

    void Append(const Size& rSize, const XBitmapEntry& rEntry);
    void Modify(const Size& rSize, const XBitmapEntry& rEntry, sal_uInt16 nPos);
    void SelectEntryByList(const XBitmapListSharedPtr aList, const String& rStr);

private:
    BitmapEx        maBitmapEx;
};

/*************************************************************************
|*
|* FillAttrLB vereint alle Fuellattribute in einer ListBox
|*
\************************************************************************/
class FillAttrLB : public ColorListBox
{
private:
    BitmapEx        maBitmapEx;

public:
    FillAttrLB( Window* pParent, ResId Id );
    FillAttrLB( Window* pParent, WinBits aWB );

    virtual void Fill( const XColorListSharedPtr aTab );
    virtual void Fill( const XHatchListSharedPtr aList );
    virtual void Fill( const XGradientListSharedPtr aList );
    virtual void Fill( const XBitmapListSharedPtr aList );

    void SelectEntryByList(const XBitmapListSharedPtr aList, const String& rStr);
};

/*************************************************************************
|*
|* FillTypeLB
|*
\************************************************************************/
class FillTypeLB : public ListBox
{

public:
         FillTypeLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
         FillTypeLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    virtual void Fill();
};

/*************************************************************************
|*
|* LineLB
|*
\************************************************************************/

class XDashList;
typedef ::boost::shared_ptr< XDashList > XDashListSharedPtr;

class SVX_DLLPUBLIC LineLB : public ListBox
{
private:
    /// bitfield
    /// defines if standard fields (none, solid) are added, default is true
    bool        mbAddStandardFields : 1;

public:
    LineLB(Window* pParent, ResId Id);
    LineLB(Window* pParent, WinBits aWB);
    virtual ~LineLB();

    virtual void Fill(const XDashListSharedPtr aList);

    bool getAddStandardFields() const { return mbAddStandardFields; }
    void setAddStandardFields(bool bNew);

    void Append(const XDashEntry& rEntry, const Bitmap& rBitmap );
    void Modify(const XDashEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
    void SelectEntryByList(const XDashListSharedPtr aList, const String& rStr, const XDash& rDash, sal_uInt16 nDist = 0);
};

/*************************************************************************
|*
|* LineEndsLB
|*
\************************************************************************/

class XLineEndList;
typedef ::boost::shared_ptr< XLineEndList > XLineEndListSharedPtr;

class SVX_DLLPUBLIC LineEndLB : public ListBox
{

public:
                          LineEndLB( Window* pParent, ResId Id );
                          LineEndLB( Window* pParent, WinBits aWB );
                 virtual ~LineEndLB (void);

    virtual void Fill( const XLineEndListSharedPtr aList, bool bStart = true );

    void    Append( const XLineEndEntry& rEntry, const Bitmap& rBitmap, bool bStart = true );
    void    Modify( const XLineEndEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap, bool bStart = true );
};

//////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrModel;

class SvxPreviewBase : public Control
{
private:
    SdrModel*                                       mpModel;
    VirtualDevice*                                  mpBufferDevice;

protected:
    void InitSettings(bool bForeground, bool bBackground);

    // prepare buffered paint
    void LocalPrePaint();

    // end and output buffered paint
    void LocalPostPaint();

public:
    SvxPreviewBase( Window* pParent, const ResId& rResId );
    virtual ~SvxPreviewBase();

    // change support
    virtual void StateChanged(StateChangedType nStateChange);
    virtual void DataChanged(const DataChangedEvent& rDCEvt);

    // dada read access
    SdrModel& getModel() const { return *mpModel; }
    OutputDevice& getBufferDevice() const { return *mpBufferDevice; }
};

/*************************************************************************
|*
|* SvxLinePreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXLinePreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpLineObjA;
    SdrObject*                                      mpLineObjB;
    SdrObject*                                      mpLineObjC;

    //#58425# Symbole auf einer Linie (z.B. StarChart)
    Graphic*                                        mpGraphic;
    sal_Bool                                        mbWithSymbol;
    Size                                            maSymbolSize;

public:
    SvxXLinePreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXLinePreview();

    void SetLineAttributes(const SfxItemSet& rItemSet);

    void ShowSymbol( sal_Bool b ) { mbWithSymbol = b; };
    void SetSymbol( Graphic* p, const Size& s );
    void ResizeSymbol( const Size& s );

    virtual void Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXRectPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXRectPreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpRectangleObject;

public:
    SvxXRectPreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXRectPreview();

    void SetAttributes(const SfxItemSet& rItemSet);

    virtual void    Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXShadowPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXShadowPreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpRectangleObject;
    SdrObject*                                      mpRectangleShadow;

public:
    SvxXShadowPreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXShadowPreview();

    void SetRectangleAttributes(const SfxItemSet& rItemSet);
    void SetShadowAttributes(const SfxItemSet& rItemSet);
    void SetShadowPosition(const Point& rPos);

    virtual void    Paint( const Rectangle& rRect );
};

#endif // _SVX_DLG_CTRL_HXX

