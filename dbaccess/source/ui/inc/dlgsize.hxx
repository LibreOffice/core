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


#ifndef _DBAUI_DLGSIZE_HXX
#define _DBAUI_DLGSIZE_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    class DlgSize : public ModalDialog
    {
    private:
        sal_Int32       m_nPrevValue, m_nStandard;
        void            SetValue( sal_Int32 nVal );

    protected:
        DECL_LINK( CbClickHdl, Button * );

        FixedText       aFT_VALUE;
        MetricField     aMF_VALUE;
        CheckBox        aCB_STANDARD;
        OKButton        aPB_OK;
        CancelButton    aPB_CANCEL;
        HelpButton      aPB_HELP;

    public:
        DlgSize( Window * pParent, sal_Int32 nVal, sal_Bool bRow, sal_Int32 _nAlternativeStandard = -1 );
        ~DlgSize();

        sal_Int32 GetValue();
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DLGSIZE_HXX

