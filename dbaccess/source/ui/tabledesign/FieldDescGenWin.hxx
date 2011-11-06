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


#ifndef DBAUI_TABLEFIELDDESCGENPAGE_HXX
#define DBAUI_TABLEFIELDDESCGENPAGE_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif

namespace dbaui
{
    class OTableDesignHelpBar;
    class OFieldDescription;
    class OTableFieldControl;
    class OTableEditorCtrl;
    //==================================================================
    class OFieldDescGenWin : public TabPage
                            ,public IClipboardTest
    {

        OTableFieldControl  *m_pFieldControl;
    protected:
        virtual void Resize();

    public:
        OFieldDescGenWin( Window* pParent, OTableDesignHelpBar* pHelpBar );
        virtual ~OFieldDescGenWin();

        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetControlText( sal_uInt16 nControlId, const String& rText );
        String GetControlText( sal_uInt16 nControlId );
        void SetReadOnly( sal_Bool bReadOnly );
#if OSL_DEBUG_LEVEL > 0
        OTableEditorCtrl* GetEditorCtrl();
#endif
        //  short GetFormatCategory(OFieldDescription* pFieldDescr);
            // liefert zum am Feld eingestellten Format einen der CAT_xxx-Werte (CAT_NUMBER, CAT_DATE ...)

        String BoolStringPersistent(const String& rUIString) const;
        String BoolStringUI(const String& rPersistentString) const;

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();

        inline OTableFieldControl*  getFieldControl() const { return m_pFieldControl; }
    };
}
#endif // DBAUI_TABLEFIELDDESCGENPAGE_HXX

