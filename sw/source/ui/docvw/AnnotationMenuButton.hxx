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



#ifndef _ANNOTATIONMENUBUTTON_HXX
#define _ANNOTATIONMENUBUTTON_HXX

#include <vcl/menubtn.hxx>

namespace sw { namespace sidebarwindows {
    class SwSidebarWin;
} }

namespace sw { namespace annotation {

class AnnotationMenuButton : public MenuButton
{
    public:
        AnnotationMenuButton( sw::sidebarwindows::SwSidebarWin& rSidebarWin );
        ~AnnotationMenuButton();

        // overloaded <MenuButton> methods
        virtual void Select();

        // overloaded <Window> methods
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void KeyInput( const KeyEvent& rKeyEvt );

    private:
        sw::sidebarwindows::SwSidebarWin& mrSidebarWin;
};

} } // end of namespace sw::annotation

#endif
