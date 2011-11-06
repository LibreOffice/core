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



#ifndef _ANNOTATIONWIN_HXX
#define _ANNOTATIONWIN_HXX

#include <SidebarWin.hxx>

class PopupMenu;
class OutlinerParaObject;

namespace sw { namespace annotation {

class SwAnnotationWin : public sw::sidebarwindows::SwSidebarWin
{
    public:
        SwAnnotationWin( SwEditWin& rEditWin,
                         WinBits nBits,
                         SwPostItMgr& aMgr,
                         SwPostItBits aBits,
                         SwSidebarItem& rSidebarItem,
                         SwFmtFld* aField );
        virtual ~SwAnnotationWin();

        virtual void    UpdateData();
        virtual void    SetPostItText();
        virtual void    Delete();
        virtual void    GotoPos();

        virtual String  GetAuthor();
        virtual Date    GetDate();
        virtual Time    GetTime();

        virtual sal_uInt32 MoveCaret();

        virtual bool    CalcFollow();
        void            InitAnswer(OutlinerParaObject* pText);

        virtual bool    IsProtected();

    protected:
        virtual MenuButton* CreateMenuButton();

    private:
        virtual SvxLanguageItem GetLanguage(void);
        sal_uInt32 CountFollowing();

        SwFmtFld*       mpFmtFld;
        SwPostItField*  mpFld;
        PopupMenu*      mpButtonPopup;

};

} } // end of namespace sw::annotation
#endif
