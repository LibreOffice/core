/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
