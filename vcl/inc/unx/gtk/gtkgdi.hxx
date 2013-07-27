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



#ifndef _VCL_GTKGDI_HXX
#define _VCL_GTKGDI_HXX

#include <tools/prex.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <tools/postx.h>

#include <unx/salgdi.h>

class GtkSalGraphics : public X11SalGraphics
{
    GtkWidget           *m_pWindow;
    Region               m_aClipRegion;

public:
                        GtkSalGraphics( GtkWidget *window )
                            : m_pWindow( window ),
                              m_aClipRegion(true)
                              {}
    virtual             ~GtkSalGraphics();

    inline GtkWidget*  GetGtkWidget() const { return m_pWindow; }
    inline GdkWindow*  GetGdkWindow() const { return m_pWindow->window; }
    inline GtkSalFrame* GetGtkFrame() const { return static_cast<GtkSalFrame*>(m_pFrame); }
    void SetWindow( GtkWidget* window ) { m_pWindow = window; }


    // will be set when UI theme was changed
    static  sal_Bool        bThemeChanged;
    static  sal_Bool        bNeedPixmapPaint;
    static  sal_Bool        bGlobalNeedPixmapPaint;
    static  sal_Bool        bToolbarGripWorkaround;
    static  sal_Bool        bNeedButtonStyleAsEditBackgroundWorkaround;

    // native widget methods
    virtual sal_Bool        IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual sal_Bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const rtl::OUString& rCaption );
    virtual sal_Bool        drawNativeControlText( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const rtl::OUString& rCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const rtl::OUString& rCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    //helper methods for frame's UpdateSettings
    void updateSettings( AllSettings& rSettings );

    virtual bool            setClipRegion( const Region& );
    virtual void            ResetClipRegion();

    // some themes set the background pixmap of our window EVERY time
    // a control is painted; but presentation effects need
    // the background set to None; workaround: set the background
    // before copyBits
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics );

protected:
    typedef std::list< Rectangle > clipList;

    GdkPixmap* NWGetPixmapFromScreen( Rectangle srcRect );
    sal_Bool NWRenderPixmapToScreen( GdkPixmap* pPixmap, Rectangle dstRect );

    sal_Bool NWPaintGTKButton( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKRadio( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    sal_Bool NWPaintGTKCheck( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                          const Rectangle& rControlRectangle,
                          const clipList& rClipList,
                          ControlState nState, const ImplControlValue& aValue,
                          const OUString& rCaption );
    sal_Bool NWPaintGTKScrollbar( ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    sal_Bool NWPaintGTKEditBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKSpinBox( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKComboBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                             const Rectangle& rControlRectangle,
                             const clipList& rClipList,
                             ControlState nState, const ImplControlValue& aValue,
                             const OUString& rCaption );
    sal_Bool NWPaintGTKTabItem( ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKListBox( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );

    sal_Bool NWPaintGTKToolbar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKMenubar( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKPopupMenu( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                              const Rectangle& rControlRectangle,
                              const clipList& rClipList,
                              ControlState nState, const ImplControlValue& aValue,
                              const OUString& rCaption );
    sal_Bool NWPaintGTKTooltip( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKProgress( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
    sal_Bool NWPaintGTKSlider( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                           const Rectangle& rControlRectangle,
                           const clipList& rClipList,
                           ControlState nState, const ImplControlValue& aValue,
                           const OUString& rCaption );
    sal_Bool NWPaintGTKListNode( GdkDrawable* gdkDrawable, ControlType nType, ControlPart nPart,
                            const Rectangle& rControlRectangle,
                            const clipList& rClipList,
                            ControlState nState, const ImplControlValue& aValue,
                            const OUString& rCaption );
};

#endif // _VCL_GTKGDI_HXX
