/*************************************************************************
 *
 *  $RCSfile: scripttypedetector.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:06:35 $
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

#include <com/sun/star/i18n/CTLScriptType.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>

//  ----------------------------------------------------
//  class ScriptTypeDetector
//  ----------------------------------------------------;

using namespace com::sun::star::i18n;

ScriptTypeDetector::ScriptTypeDetector()
{
}

ScriptTypeDetector::~ScriptTypeDetector()
{
}

static sal_Int16 scriptDirection[] = {
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_LEFT_TO_RIGHT = 0,
    ScriptDirection::RIGHT_TO_LEFT, // DirectionProperty_RIGHT_TO_LEFT = 1,
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_EUROPEAN_NUMBER = 2,
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_EUROPEAN_NUMBER_SEPARATOR = 3,
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_EUROPEAN_NUMBER_TERMINATOR = 4,
    ScriptDirection::RIGHT_TO_LEFT, // DirectionProperty_ARABIC_NUMBER = 5,
    ScriptDirection::NEUTRAL,       // DirectionProperty_COMMON_NUMBER_SEPARATOR = 6,
    ScriptDirection::NEUTRAL,       // DirectionProperty_BLOCK_SEPARATOR = 7,
    ScriptDirection::NEUTRAL,       // DirectionProperty_SEGMENT_SEPARATOR = 8,
    ScriptDirection::NEUTRAL,       // DirectionProperty_WHITE_SPACE_NEUTRAL = 9,
    ScriptDirection::NEUTRAL,       // DirectionProperty_OTHER_NEUTRAL = 10,
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_LEFT_TO_RIGHT_EMBEDDING = 11,
    ScriptDirection::LEFT_TO_RIGHT, // DirectionProperty_LEFT_TO_RIGHT_OVERRIDE = 12,
    ScriptDirection::RIGHT_TO_LEFT, // DirectionProperty_RIGHT_TO_LEFT_ARABIC = 13,
    ScriptDirection::RIGHT_TO_LEFT, // DirectionProperty_RIGHT_TO_LEFT_EMBEDDING = 14,
    ScriptDirection::RIGHT_TO_LEFT, // DirectionProperty_RIGHT_TO_LEFT_OVERRIDE = 15,
    ScriptDirection::NEUTRAL,       // DirectionProperty_POP_DIRECTIONAL_FORMAT = 16,
    ScriptDirection::NEUTRAL,       // DirectionProperty_DIR_NON_SPACING_MARK = 17,
    ScriptDirection::NEUTRAL,       // DirectionProperty_BOUNDARY_NEUTRAL = 18,
};

sal_Int16 SAL_CALL
ScriptTypeDetector::getScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int16 dir = scriptDirection[unicode::getUnicodeDirection(Text[nPos])];
    return (dir == ScriptDirection::NEUTRAL) ? defaultScriptDirection : dir;
}

// return value '-1' means either the direction on nPos is not same as scriptDirection or nPos is out of range.
sal_Int32 SAL_CALL
ScriptTypeDetector::beginOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 cPos = nPos;

    if (cPos < Text.getLength()) {
        for (; cPos >= 0; cPos--) {
        if (scriptDirection != getScriptDirection(Text, cPos, scriptDirection))
            break;
        }
        return cPos == nPos ? -1 : cPos + 1;
    }
}

sal_Int32 SAL_CALL
ScriptTypeDetector::endOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 cPos = nPos;
    sal_Int32 len = Text.getLength();

    if (cPos >=0) {
        for (; cPos < len; cPos++) {
        if (scriptDirection != getScriptDirection(Text, cPos, scriptDirection))
            break;
        }
    }
    return cPos == nPos ? -1 : cPos;
}

sal_Int16 SAL_CALL
ScriptTypeDetector::getCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    static ScriptTypeList typeList[] = {
    { UnicodeScript_kHebrew,    CTLScriptType::CTL_HEBREW },    // 10
    { UnicodeScript_kArabic,    CTLScriptType::CTL_ARABIC },    // 11
    { UnicodeScript_kDevanagari,    CTLScriptType::CTL_INDIC }, // 14
    { UnicodeScript_kThai,      CTLScriptType::CTL_THAI },  // 24
    { UnicodeScript_kScriptCount,   CTLScriptType::CTL_UNKNOWN }    // 88
    };

    return unicode::getUnicodeScriptType(Text[nPos], typeList, CTLScriptType::CTL_UNKNOWN);
}

// Begin of Script Type is inclusive.
sal_Int32 SAL_CALL
ScriptTypeDetector::beginOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    if (nPos < 0)
    return 0;
    else if (nPos >= Text.getLength())
    return Text.getLength();
    else {
    sal_Int16 cType = getCTLScriptType(Text, nPos);
    for (nPos--; nPos >= 0; nPos--) {
        if (cType != getCTLScriptType(Text, nPos))
        break;
    }
    return nPos + 1;
    }
}

// End of the Script Type is exclusive, the return value pointing to the begin of next script type
sal_Int32 SAL_CALL
ScriptTypeDetector::endOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    if (nPos < 0)
    return 0;
    else if (nPos >= Text.getLength())
    return Text.getLength();
    else {
    sal_Int16 cType = getCTLScriptType(Text, nPos);
    sal_Int32 len = Text.getLength();
    for (nPos++; nPos < len; nPos++) {
        if (cType != getCTLScriptType(Text, nPos))
        break;
    }
    return nPos;
    }
}

const sal_Char sDetector[] = "draft.com.sun.star.i18n.ScriptTypeDetector";

rtl::OUString SAL_CALL
ScriptTypeDetector::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii(sDetector);
}

sal_Bool SAL_CALL
ScriptTypeDetector::supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException )
{
    return !ServiceName.compareToAscii(sDetector);
}

::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
ScriptTypeDetector::getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString::createFromAscii(sDetector);
    return aRet;
}

