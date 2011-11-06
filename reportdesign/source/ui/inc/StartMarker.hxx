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


#ifndef RPTUI_STARTMARKER_HXX
#define RPTUI_STARTMARKER_HXX

#include <osl/interlck.h>
#include <svtools/ruler.hxx>
#include "ColorListener.hxx"
#include <vcl/fixed.hxx>


namespace rptui
{
    class OSectionWindow;
    class OStartMarker : public OColorListener
    {

        Ruler                       m_aVRuler;
        FixedText                   m_aText;
        FixedImage                  m_aImage;
        OSectionWindow*             m_pParent;
        static Image*               s_pDefCollapsed;
        static Image*               s_pDefExpanded;
        static Image*               s_pDefCollapsedHC;
        static Image*               s_pDefExpandedHC;
        static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

        sal_Bool                    m_bShowRuler;

        void changeImage();
        void initDefaultNodeImages();
        void setColor();
        virtual void ImplInitSettings();
        OStartMarker(OStartMarker&);
        void operator =(OStartMarker&);
    public:
        OStartMarker(OSectionWindow* _pParent,const ::rtl::OUString& _sColorEntry);
        virtual ~OStartMarker();

        // SfxListener
        virtual void    Notify(SfxBroadcaster & rBc, SfxHint const & rHint);
        // window overloads
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Resize();
        virtual void    RequestHelp( const HelpEvent& rHEvt );
        using Window::Notify;

        void            setTitle(const String& _sTitle);
        sal_Int32       getMinHeight() const;

        /** shows or hides the ruler.
        */
        void            showRuler(sal_Bool _bShow);

        virtual void    setCollapsed(sal_Bool _bCollapsed);

        /** zoom the ruler and view windows
        */
        void            zoom(const Fraction& _aZoom);
    };
}
#endif // RPTUI_STARTMARKER_HXX

