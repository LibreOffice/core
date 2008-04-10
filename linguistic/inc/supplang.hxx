/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: supplang.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _LINGUISTIC_SUPPLANG_HXX_
#define _LINGUISTIC_SUPPLANG_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <tools/table.hxx>
#include <vcl/svapp.hxx>
#include <osl/thread.h>
#include <unotools/charclass.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySet;
    class XFastPropertySet;
}}}}

namespace com { namespace sun { namespace star { namespace frame {
    class XDesktop;
}}}}


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

// defines for actual state (availability) of languages
#define LANG_NOT_AVAIL  ((short) 0x0303)
#define LANG_QUERY      ((short) 0x5555)
#define LANG_FAILED     ((short) 0)
#define LANG_OK         ((short) 1)

enum ModuleName { MOD_SPELL, MOD_HYPH, MOD_THES };

struct LanguageState
{
    INT16   nLanguage;
    INT16   nSpellState;
    INT16   nHyphState;
    INT16   nThesState;

    INT16 & GetState( ModuleName eModule );
};

LanguageState * GetLanguageState( INT16 nLanguage, LanguageState *pArray );

///////////////////////////////////////////////////////////////////////////

DECLARE_TABLE( SuppLangTable, INT16 * )

class SuppLanguages
{
    SuppLangTable   aLanguages;

protected:
    virtual INT16   CheckLanguage( INT16 nLanguage ) const = 0;

public:
    SuppLanguages( LanguageState *pState, USHORT nCount );
    virtual ~SuppLanguages();

    BOOL    HasLanguage( INT16 nLanguage ) const;
    const com::sun::star::uno::Sequence< INT16 >
            GetLanguages() const;
};

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

