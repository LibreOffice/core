/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <vcl/vclmain.hxx>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/pngread.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/button.hxx>
#include <vcl/floatwin.hxx>

#if 0
#  define FIXME_SELF_INTERSECTING_WORKING
#endif

using namespace css;

class DemoBase :
        public WorkWindow // hide OutputDevice if necessary
{
public:
    DemoBase() : WorkWindow( NULL, WB_APP | WB_STDWORK)
    {
    }
    OutputDevice &getOutDev() { return *this; }
};

class DemoWin : public DemoBase
{
    Bitmap   maIntroBW;
    BitmapEx maIntro;

public:
    DemoWin() : DemoBase()
              , mpButton(NULL)
              , mpButtonWin(NULL)
    {
        if (!Application::LoadBrandBitmap("intro", maIntro))
            Application::Abort("Failed to load intro image");
        maIntroBW = maIntro.GetBitmap();
        maIntroBW.Filter( BMP_FILTER_EMBOSS_GREY );
    }

    // Bouncing windows on click ...
    PushButton     *mpButton;
    FloatingWindow *mpButtonWin;
    AutoTimer       maBounce;
    int             mnBounceX, mnBounceY;
    DECL_LINK(BounceTimerCb, void *);

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;

    void drawToDevice(OutputDevice &r, bool bVdev);

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE
    {
        fprintf(stderr, "DemoWin::Paint(%ld,%ld,%ld,%ld)\n", rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight());
        drawToDevice(getOutDev(), false);
    }

    std::vector<Rectangle> partitionAndClear(OutputDevice &rDev,
                                             int nX, int nY);

    void drawBackground(OutputDevice &rDev)
    {
        Rectangle r(Point(0,0), rDev.GetOutputSizePixel());
        Gradient aGradient;
        aGradient.SetStartColor(COL_BLUE);
        aGradient.SetEndColor(COL_GREEN);
        aGradient.SetStyle(GradientStyle_LINEAR);
//        aGradient.SetBorder(r.GetSize().Width()/20);
        rDev.DrawGradient(r, aGradient);
    }

    void drawRadialLines(OutputDevice &rDev, Rectangle r)
    {
        rDev.SetFillColor(Color(COL_LIGHTRED));
        rDev.SetLineColor(Color(COL_BLACK));
        rDev.DrawRect( r );

        for(int i=0; i<r.GetHeight(); i+=15)
            rDev.DrawLine( Point(r.Left(), r.Top()+i), Point(r.Right(), r.Bottom()-i) );
        for(int i=0; i<r.GetWidth(); i+=15)
            rDev.DrawLine( Point(r.Left()+i, r.Bottom()), Point(r.Right()-i, r.Top()) );

        // Should draw a white-line across the middle
        Color aLastPixel( COL_WHITE );
        Point aCenter((r.Left() + r.Right())/2 - 4,
                      (r.Top() + r.Bottom())/2 - 4);
        for(int i=0; i<8; i++)
        {
            rDev.DrawPixel(aCenter, aLastPixel);
            aLastPixel = rDev.GetPixel(aCenter);
            aCenter.Move(1,1);
        }
    }

    void drawText(OutputDevice &rDev, Rectangle r)

    {
        rDev.SetTextColor( Color( COL_BLACK ) );
        vcl::Font aFont( OUString( "Times" ), Size( 0, 25 ) );
        rDev.SetFont( aFont );
        rDev.DrawText( r, OUString( "Just a simple text" ) );
    }

    void drawPoly(OutputDevice &rDev, Rectangle r) // pretty
    {
        drawCheckered(rDev, r);

        long nDx = r.GetWidth()/20;
        long nDy = r.GetHeight()/20;
        Rectangle aShrunk(r);
        aShrunk.Move(nDx, nDy);
        aShrunk.SetSize(Size(r.GetWidth()-nDx*2,
                             r.GetHeight()-nDy*2));
        Polygon aPoly(aShrunk);
        tools::PolyPolygon aPPoly(aPoly);
        rDev.SetLineColor(Color(COL_RED));
        rDev.SetFillColor(Color(COL_RED));
        // This hits the optional 'drawPolyPolygon' code-path
        rDev.DrawTransparent(aPPoly, 64);
    }
    void drawEllipse(OutputDevice &rDev, Rectangle r)

    {
        rDev.SetLineColor(Color(COL_RED));
        rDev.SetFillColor(Color(COL_GREEN));
        rDev.DrawEllipse(r);
    }
    void drawCheckered(OutputDevice &rDev, Rectangle r)

    {
        rDev.DrawCheckered(r.TopLeft(), r.GetSize());
    }
    void drawGradient(OutputDevice &rDev, Rectangle r)

    {
        Gradient aGradient;
        aGradient.SetStartColor(COL_YELLOW);
        aGradient.SetEndColor(COL_RED);
//        aGradient.SetAngle(45);
        aGradient.SetStyle(GradientStyle_RECT);
        aGradient.SetBorder(r.GetSize().Width()/20);
        rDev.DrawGradient(r, aGradient);
    }
    void drawBitmap(OutputDevice &rDev, Rectangle r)

    {
        Bitmap aBitmap(maIntroBW);
        aBitmap.Scale(r.GetSize(), BMP_SCALE_BESTQUALITY);
        rDev.DrawBitmap(r.TopLeft(), aBitmap);
    }
    void drawBitmapEx(OutputDevice &rDev, Rectangle r)

    {
        drawCheckered(rDev, r);

        BitmapEx aBitmap(maIntro);
        aBitmap.Scale(r.GetSize(), BMP_SCALE_BESTQUALITY);
        AlphaMask aSemiTransp(aBitmap.GetSizePixel());
        aSemiTransp.Erase(64);
        rDev.DrawBitmapEx(r.TopLeft(), BitmapEx(aBitmap.GetBitmap(),
                                                aSemiTransp));
    }
    void drawPolyPolgons(OutputDevice &rDev, Rectangle r)

    {
        struct {
            double nX, nY;
        } aPoints[] = { { 0.1, 0.1 }, { 0.9, 0.9 },
#ifdef FIXME_SELF_INTERSECTING_WORKING
                        { 0.9, 0.1 }, { 0.1, 0.9 },
                        { 0.1, 0.1 } };
#else
                        { 0.1, 0.9 }, { 0.5, 0.5 },
                        { 0.9, 0.1 }, { 0.1, 0.1 } };
#endif

        tools::PolyPolygon aPolyPoly;
        // Render 4x polygons & aggregate into another PolyPolygon
        for (int x = 0; x < 2; x++)
        {
            for (int y = 0; y < 2; y++)
            {
                Rectangle aSubRect(r);
                aSubRect.Move(x * r.GetWidth()/3, y * r.GetHeight()/3);
                aSubRect.SetSize(Size(r.GetWidth()/2, r.GetHeight()/4));
                Polygon aPoly(SAL_N_ELEMENTS(aPoints));
                for (size_t v = 0; v < SAL_N_ELEMENTS(aPoints); v++)
                {
                    aPoly.SetPoint(Point(aSubRect.Left() +
                                         aSubRect.GetWidth() * aPoints[v].nX,
                                         aSubRect.Top() +
                                         aSubRect.GetHeight() * aPoints[v].nY),
                                   v);
                }
                rDev.SetLineColor(Color(COL_YELLOW));
                rDev.SetFillColor(Color(COL_BLACK));
                rDev.DrawPolygon(aPoly);

                // now move and add to the polypolygon
                aPoly.Move(0, r.GetHeight()/2);
                aPolyPoly.Insert(aPoly);
            }
        }
        rDev.SetLineColor(Color(COL_LIGHTRED));
        rDev.SetFillColor(Color(COL_GREEN));
        rDev.DrawTransparent(aPolyPoly, 50);
    }
    void drawToVirtualDevice(OutputDevice &rDev, Rectangle r)
    {
        VirtualDevice aNested(rDev);
        aNested.SetOutputSizePixel(r.GetSize());
        Rectangle aWhole(Point(0,0), r.GetSize());
        // mini me
        drawToDevice(aNested, true);

        Bitmap aBitmap(aNested.GetBitmap(Point(0,0),aWhole.GetSize()));
        rDev.DrawBitmap(r.TopLeft(), aBitmap);
    }

    std::vector<BitmapEx> maIcons;
    void initIcons()
    {
        if (maIcons.size())
            return;

        const char *pNames[] = {
            "cmd/lc_openurl.png",
            "cmd/lc_newdoc.png",
            "cmd/lc_save.png",
            "cmd/lc_saveas.png",
            "cmd/lc_sendmail.png",
            "cmd/lc_editdoc.png",
            "cmd/lc_print.png",
            "cmd/lc_printpreview.png",
            "cmd/lc_cut.png",
            "cmd/lc_copy.png",
            "cmd/lc_paste.png",
            "cmd/lc_formatpaintbrush.png",
            "cmd/lc_undo.png",
            "cmd/lc_redo.png",
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pNames); i++)
            maIcons.push_back(BitmapEx(OUString::createFromAscii(pNames[i])));
    }
    void drawIcons(OutputDevice &rDev, Rectangle r)
    {
        initIcons();

        Rectangle p(r);
        for (size_t i = 0; i < maIcons.size(); i++)
        {
            Size aSize(maIcons[i].GetSizePixel());
            rDev.DrawBitmapEx(p.TopLeft(), maIcons[i]);
            p.Move(aSize.Width(), 0);
            if (p.Left() >= r.Right())
                break;
        }
    }

    void fetchDrawBitmap(OutputDevice &rDev, Rectangle r)
    {
        Bitmap aBitmap(rDev.GetBitmap(Point(0,0),rDev.GetOutputSizePixel()));
        aBitmap.Scale(r.GetSize(), BMP_SCALE_BESTQUALITY);
        rDev.DrawBitmap(r.TopLeft(), aBitmap);
    }
};

IMPL_LINK_NOARG(DemoWin,BounceTimerCb)
{
    mpButton->Check(mnBounceX>0);
    mpButton->SetPressed(mnBounceY>0);

    Point aCur = mpButtonWin->GetPosPixel();
    static const int nMovePix = 10;
    aCur.Move(mnBounceX * nMovePix, mnBounceX * nMovePix);
    Size aWinSize = GetSizePixel();
    if (aCur.X() <= 0 || aCur.X() >= aWinSize.Width())
        mnBounceX *= -1;
    if (aCur.Y() <= 0 || aCur.Y() >= aWinSize.Height())
        mnBounceX *= -1;
    mpButtonWin->SetPosPixel(aCur);

    // All smoke and mirrors to test sub-region invalidation underneath
    Rectangle aRect(aCur, mpButtonWin->GetSizePixel());
    Invalidate(aRect);
    return 0;
}

void DemoWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    (void) rMEvt;
    if (!mpButton)
    {
        mpButtonWin = new FloatingWindow(this);
        mpButton = new PushButton(mpButtonWin);
        mpButton->SetSymbol(SymbolType::HELP);
        mpButton->SetText("PushButton demo");
        mpButton->SetPosSizePixel(Point(0,0), mpButton->GetOptimalSize());
        mpButton->Show();
        mpButtonWin->SetPosSizePixel(Point(0,0), mpButton->GetOptimalSize());
        mpButtonWin->Show();
        mnBounceX = 1; mnBounceX = 1;
        maBounce.SetTimeoutHdl(LINK(this,DemoWin,BounceTimerCb));
        maBounce.SetTimeout(55);
        maBounce.Start();
    }
    else
    {
        maBounce.Stop();
        delete mpButtonWin;
        mpButtonWin = NULL;
        mpButton = NULL;
    }
}

std::vector<Rectangle> DemoWin::partitionAndClear(OutputDevice &rDev, int nX, int nY)
{
    Rectangle r;
    std::vector<Rectangle> aRegions;

    // Make small cleared area for these guys
    Size aSize(rDev.GetOutputSizePixel());
    long nBorderSize = aSize.Width() / 32;
    long nBoxWidth = (aSize.Width() - nBorderSize*(nX+1)) / nX;
    long nBoxHeight = (aSize.Height() - nBorderSize*(nY+1)) / nY;
    for (int y = 0; y < nY; y++ )
    {
        for (int x = 0; x < nX; x++ )
        {
            r.SetPos(Point(nBorderSize + (nBorderSize + nBoxWidth) * x,
                           nBorderSize + (nBorderSize + nBoxHeight) * y));
            r.SetSize(Size(nBoxWidth, nBoxHeight));

            // knock up a nice little border
            rDev.SetLineColor(COL_GRAY);
            rDev.SetFillColor(COL_LIGHTGRAY);
            if ((x + y) % 2)
                rDev.DrawRect(r, nBorderSize, nBorderSize);
            else
                rDev.DrawRect(r);

            aRegions.push_back(r);
        }
    }

    return aRegions;
}

void DemoWin::drawToDevice(OutputDevice &rDev, bool bVdev)
{
    drawBackground(rDev);

    std::vector<Rectangle> aRegions(partitionAndClear(rDev, 4, 3));

    drawRadialLines(rDev, aRegions[0]);
    drawText(rDev, aRegions[1]);
    drawPoly(rDev, aRegions[2]);
    drawEllipse(rDev, aRegions[3]);
    drawCheckered(rDev, aRegions[4]);
    drawBitmapEx(rDev, aRegions[5]);
    drawBitmap(rDev, aRegions[6]);
    drawGradient(rDev, aRegions[7]);
    drawPolyPolgons(rDev, aRegions[8]);
    if (!bVdev)
        drawToVirtualDevice(rDev, aRegions[9]);
    drawIcons(rDev, aRegions[10]);
    // last - thumbnail all the above
    fetchDrawBitmap(rDev, aRegions[11]);
}

class DemoApp : public Application
{
public:
    DemoApp() {}

    virtual int Main() SAL_OVERRIDE
    {
        DemoWin aMainWin;
        aMainWin.SetText( "Interactive VCL demo" );
        aMainWin.Show();
        Application::Execute();
        return 0;
    }

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() SAL_OVERRIDE
    {
        try
        {
            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            xMSF = uno::Reference<lang::XMultiServiceFactory>
                ( xComponentContext->getServiceManager(), uno::UNO_QUERY );
            if( !xMSF.is() )
                Application::Abort("Bootstrap failure - no service manager");

            ::comphelper::setProcessServiceFactory( xMSF );
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }
    void DeInit() SAL_OVERRIDE
    {
        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW )-> dispose();
        ::comphelper::setProcessServiceFactory( NULL );
    }
};

void vclmain::createApplication()
{
    static DemoApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
