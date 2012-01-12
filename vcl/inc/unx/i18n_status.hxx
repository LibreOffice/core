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



#ifndef _SAL_I18N_STATUS_HXX
#define _SAL_I18N_STATUS_HXX

#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>

#include <rtl/ustring.hxx>

#include <salimestatus.hxx>

#include <vector>

class SalFrame;
class WorkWindow;
class ListBox;
class FixedText;
class PushButton;
class SalI18N_InputContext;

namespace vcl
{

class StatusWindow;

class X11ImeStatus : public SalI18NImeStatus
{
public:
    X11ImeStatus() {}
    virtual ~X11ImeStatus();

    virtual bool canToggle();
    virtual void toggle();
};

class I18NStatus
{
public:
    struct ChoiceData
    {
        String  aString;
        void*   pData;
    };
private:
    SalFrame*                       m_pParent;
    StatusWindow*                   m_pStatusWindow;
    String                          m_aCurrentIM;
    ::std::vector< ChoiceData >     m_aChoices;

    I18NStatus();
    ~I18NStatus();

    static I18NStatus* pInstance;

    static bool getStatusWindowMode();

public:
    static I18NStatus& get();
    static bool exists();
    static void free();

    void setParent( SalFrame* pParent );
    SalFrame* getParent() const { return  m_pParent; }
    SalFrame* getStatusFrame() const;

    void setStatusText( const String& rText );
    String getStatusText() const;

    enum ShowReason { focus, presentation, contextmap };
    void show( bool bShow, ShowReason eReason );

    const ::std::vector< ChoiceData >& getChoices() const { return m_aChoices; }
    void clearChoices();
    void addChoice( const String&, void* pData );

    void toTop() const;

    // for SwitchIMCallback
    void changeIM( const String& );

    // External Control:

    /** Return true if the status window can be toggled on and off externally.
     */
    bool canToggleStatusWindow() const;

    /** Toggle the status window on or off.

        This only works if canToggleStatusWindow returns true (otherwise, any
        calls of this method are ignored).
     */
    void toggleStatusWindow();
};

} // namespace

#endif // _SAL_I18N_STATUS_HXX
