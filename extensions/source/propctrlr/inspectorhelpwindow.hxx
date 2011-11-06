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


#ifndef INSPECTORHELPWINDOW_HXX
#define INSPECTORHELPWINDOW_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= InspectorHelpWindow
    //====================================================================
    class InspectorHelpWindow : public Window
    {
    private:
        FixedLine       m_aSeparator;
        MultiLineEdit   m_aHelpText;

        sal_Int32       m_nMinLines;
        sal_Int32       m_nMaxLines;

    public:
        InspectorHelpWindow( Window* _pParent );

        virtual void    SetText( const XubString& rStr );

        void            SetLimits( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );
        long            GetMinimalHeightPixel();
        long            GetOptimalHeightPixel();

    protected:
        // Window overridables
        virtual void    Resize();

    private:
        long            impl_getMinimalTextWindowHeight();
        long            impl_getMaximalTextWindowHeight();
        long            impl_getHelpTextBorderHeight();
        long            impl_getSpaceAboveTextWindow();
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // HELPWINDOW_HXX

