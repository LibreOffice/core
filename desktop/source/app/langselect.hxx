/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langselect.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 14:44:20 $
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

#include <list>
#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svtools/languageoptions.hxx>

namespace desktop
{

class LanguageSelection
{
private:
    static const rtl::OUString usFallbackLanguage;
    static rtl::OUString aFoundLanguage;
    static sal_Bool bFoundLanguage;

    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
        getConfigAccess(const sal_Char* pPath, sal_Bool bUpdate=sal_False);
    static com::sun::star::uno::Sequence< rtl::OUString > getInstalledLanguages();
    static sal_Bool isInstalledLanguage(rtl::OUString& usLocale, sal_Bool bExact=sal_False);
    static rtl::OUString getFirstInstalledLanguage();
    static rtl::OUString getUserLanguage();
    static rtl::OUString getSystemLanguage();
    static void resetUserLanguage();
    static void setDefaultLocale(const rtl::OUString&);

public:
    static com::sun::star::lang::Locale IsoStringToLocale(const rtl::OUString& str);
    static rtl::OUString getLanguageString();
    static bool prepareLanguage();
};

} //namespace desktop
