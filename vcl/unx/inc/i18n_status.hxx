/*************************************************************************
 *
 *  $RCSfile: i18n_status.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2001-08-30 09:53:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#include <hash_map>

class SalFrame;
class WorkWindow;
class ListBox;
class FixedText;
class PushButton;
class SalI18N_InputContext;

namespace vcl
{

class StatusWindow;

class I18NStatus
{
    SalFrame*               m_pParent;
    StatusWindow*           m_pStatusWindow;
    SalI18N_InputContext*   m_pInputContext;
    String                  m_aCurrentIM;

    ::std::hash_map< ::rtl::OUString, void*, ::rtl::OUStringHash >
                            m_aChoices;

    I18NStatus();
    ~I18NStatus();

    static I18NStatus* pInstance;

public:
    static I18NStatus& get();
    static void free();

    void setParent( SalFrame* pParent );
    SalFrame* getParent() const { return  m_pParent; }
    SalFrame* getStatusFrame() const;

    void setStatusText( const String& rText );
    const String& getStatusText() const;

    void show( bool bShow = true );

    const ::std::hash_map< ::rtl::OUString, void*, ::rtl::OUStringHash >& getChoices() const { return m_aChoices; }
    void clearChoices();
    void addChoice( const String&, void* pData );
    SalI18N_InputContext* getInputContext( bool& bDeleteAfterUse );

    void toTop() const;

    // for SwitchIMCallback
    void changeIM( const String& );
};

} // namespace

#endif // _SAL_I18N_STATUS_HXX
