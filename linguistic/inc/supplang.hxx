/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: supplang.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-25 12:20:34 $
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

#ifndef _LINGUISTIC_SUPPLANG_HXX_
#define _LINGUISTIC_SUPPLANG_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include <com/sun/star/uno/Sequence.h>
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
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

