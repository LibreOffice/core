/*===========================================================================
    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/linguistic/inc/supplang.hxx,v 1.2 2004-11-27 13:20:05 kz Exp $

    status:

    (c) 1998-1999, Star Office Entwicklungs GmbH, Hamburg
===========================================================================*/

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
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <tools/isolang.hxx>
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
}}}};

namespace com { namespace sun { namespace star { namespace frame {
    class XDesktop;
}}}};


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
    ~SuppLanguages();

    BOOL    HasLanguage( INT16 nLanguage ) const;
    const com::sun::star::uno::Sequence< INT16 >
            GetLanguages() const;
};

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

