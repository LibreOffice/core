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
    virtual sal_uInt32 GetMarkablePointCount() const;
    virtual sal_uInt32 GetMarkedPointCount() const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark = false);
    virtual void CheckPossibilities();
    virtual bool MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark);

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
    basegfx::B2DPoint                               maMouseDownPos;
};

} // end of namespace sd

#endif      // _SD_ANNOTATIONTAG_HXX_

