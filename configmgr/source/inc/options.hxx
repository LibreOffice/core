/*************************************************************************
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2000-11-13 13:14:07 $
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

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#define CONFIGMGR_MISC_OPTIONS_HXX_

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "hashhelper.hxx"

#ifndef ASCII
#define ASCII(x) OUString::createFromAscii(x)
#endif

#include <vos/ref.hxx>

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;

    class OptionValue {
        rtl::OUString m_aName;
        uno::Any m_aValue;
    public:
        OptionValue(const rtl::OUString &_aName, const uno::Any& _aValue)
                : m_aName(_aName),
                  m_aValue(_aValue)
            {}

        rtl::OUString getName() {return m_aName;}
        uno::Any getValue() {return m_aValue;}

        void setName(const rtl::OUString &_aName) {m_aName = _aName;}
        void setValue(const uno::Any &_aAny) {m_aValue = _aAny;}
    };

    /**
       class OOptions is created one time per Configuration[update]Access
       all important options should stored in this class.
       The object will be [weiterleiten] to all other objects so we only
       need to extend this classobject and all other class can work with
       the new options or important options etc.
    */

    // jb won't take a PropertyValue so we must [nachbilden] something like that.

    class OOptions : public vos::OReference
    {
        HashMapAny m_aHashMap;
    public:
        OOptions()
            {}

        void add(const rtl::OUString &_aName, const uno::Any& _aValue)
            {
                m_aHashMap[_aName] = _aValue;
            }
        uno::Any getValue(const rtl::OUString &_aName)
            {
                return m_aHashMap[_aName];
            }

        // Some Helperfunctions, to esaier setting Values
        void add(const sal_Char* _pChar, const rtl::OUString &_aValueAsString)
            {
                uno::Any aAny;
                aAny <<= _aValueAsString;
                m_aHashMap[ASCII(_pChar)] = aAny;
            }

        rtl::OUString getLocale()
            {
                rtl::OUString aLocalStr;
                uno::Any aAny;
                aAny = m_aHashMap[ASCII("Locale")];
                if (aAny >>= aLocalStr)
                {
                    return aLocalStr;
                }
                return ASCII("en-US"); // Fallback Language
            }

        // vos::OReference implements acquire and release
        // ...
    };
} // namespace

#endif
