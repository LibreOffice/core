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


#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#define DBAUI_TABLEDESIGNHELPBAR_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif

class MultiLineEdit;
namespace dbaui
{
    //==================================================================
    // Ableitung von TabPage ist ein Trick von TH,
    // um Aenderungen der Systemfarben zu bemerken (Bug #53905)
    class OTableDesignHelpBar : public TabPage
                                ,public IClipboardTest
    {
    private:
        String          m_sHelpText;
        MultiLineEdit*  m_pTextWin;
        sal_uInt16          m_nDummy;

    protected:
        virtual void Resize();

    public:
        OTableDesignHelpBar( Window* pParent );
        virtual ~OTableDesignHelpBar();

        void SetHelpText( const String& rText );
        String GetHelpText() const { return m_sHelpText; }

        virtual long PreNotify( NotifyEvent& rNEvt );

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();
    };
}
#endif // DBAUI_TABLEDESIGNHELPBAR_HXX

