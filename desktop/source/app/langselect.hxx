/*************************************************************************
 *
 *  $RCSfile: langselect.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 12:24:58 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <list>
#include <tools/string.hxx>
#include <tools/lang.hxx>
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/lang/Locale.hpp>

namespace desktop
{

//typedef std::list<LanguageType> LangList;
typedef std::list<rtl::OUString> IsoList;
typedef std::list<String> StrList;

class LanguageSelection;

class LanguageSelectionDialog : public  ModalDialog
{
friend class LanguageSelection;
private:
//    LanguageType m_aLanguage;
    FixedText m_aText;
    ListBox m_aListBox;
    OKButton m_aButton;
    ModalDialog *m_pDialog;
    IsoList m_isoLanguages;

public:
    LanguageSelectionDialog(ResMgr* pResMgr);

};

class LanguageSelection
{
private:

    static IsoList m_lLanguages;

//    static LangList getInstalledLanguages();
    static IsoList getInstalledIsoLanguages();
//    static StrList getLanguageStrings(const LangList&);
    static StrList getLanguageStrings(const IsoList&);
    static rtl::OUString getUserLanguage();

public:
    static com::sun::star::lang::Locale IsoStringToLocale(const rtl::OUString& str);
//    static LanguageType getLanguageType();
    static rtl::OUString getLanguageString();
    static void prepareLanguage();
};

} //namespace desktop
