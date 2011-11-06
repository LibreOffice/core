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


#ifndef _DBUI_HXX
#define _DBUI_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>


class SW_DLLPUBLIC PrintMonitor: public ModelessDialog
{
public:
    enum PrintMonitorType
    {
        MONITOR_TYPE_PRINT,
        MONITOR_TYPE_MAIL,
        MONITOR_TYPE_SAVE
    };
    FixedText       aDocName;
    FixedText       aPrinting;
    FixedText       aPrinter;
    FixedText       aPrintInfo;
    CancelButton    aCancel;

    PrintMonitor( Window *pParent, PrintMonitorType eType );

    void ResizeControls();
};

class CreateMonitor : public ModelessDialog
{
public:
    CreateMonitor( Window *pParent );

    void SetTotalCount( sal_Int32 nTotal );
    void SetCurrentPosition( sal_Int32 nCurrent );

    void SetCancelHdl( const Link& rLink );

private: //methods
    void UpdateCountingText();

private: //member
    FixedText       m_aStatus;
    FixedText       m_aProgress;
    FixedText       m_aCreateDocuments;
    FixedText       m_aCounting;
    CancelButton    m_aCancelButton;

    String          m_sCountingPattern;
    String          m_sVariable_Total;
    String          m_sVariable_Position;
    sal_Int32       m_nTotalCount;
    sal_Int32       m_nCurrentPosition;
};

#endif

