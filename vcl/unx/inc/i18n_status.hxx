/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18n_status.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-01 10:36:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SAL_I18N_STATUS_HXX
#define _SAL_I18N_STATUS_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

#ifndef _SV_SALIMESTATUS_HXX
#include <salimestatus.hxx>
#endif

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
