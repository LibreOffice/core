/*===========================================================================
    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/linguistic/source/supplang.cxx,v 1.2 2004-11-27 13:21:11 kz Exp $

    status:

    (c) 1998-1999, Star Office Entwicklungs GmbH, Hamburg
===========================================================================*/

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif


#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include "misc.hxx"
#include "lngprops.hxx"
#include "supplang.hxx"

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

BOOL FileExists( const String &rMainURL )
{
    BOOL bExists = FALSE;
    if (rMainURL.Len())
    {
        try
        {
            ::ucb::Content aContent( rMainURL,
                    Reference< ::com::sun::star::ucb::XCommandEnvironment >());
            bExists = aContent.isDocument();
        }
        catch(Exception &)
        {
        }
    }
    return bExists;
}

String GetFileURL( SvtPathOptions::Pathes ePath, const String &rFileName )
{
    String aURL;
    if (rFileName.Len())
    {
        INetURLObject aURLObj;
        aURLObj.SetSmartProtocol( INET_PROT_FILE );
        aURLObj.SetSmartURL( GetModulePath(ePath) );
        DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
        aURLObj.Append( rFileName );
        DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
        aURL = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }
    return aURL;
}

///////////////////////////////////////////////////////////////////////////

INT16 & LanguageState::GetState( ModuleName eModule )
{
    if ( MOD_SPELL == eModule )
        return nSpellState;
    else if ( MOD_HYPH == eModule )
        return nHyphState;
    else
    {
        DBG_ASSERT( MOD_THES == eModule, "unexpected argument" );
        return nThesState;
    }
}


LanguageState * GetLanguageState( INT16 nLanguage, LanguageState *pArray )
{
    if (!pArray)
        return 0;

    LanguageState   *pState = pArray;
    INT16 nLang;
    while ((nLang = pState->nLanguage) != LANGUAGE_NONE  &&  nLang != nLanguage)
        ++pState;
    return nLang == LANGUAGE_NONE ? 0 : pState;
}

///////////////////////////////////////////////////////////////////////////

SuppLanguages::SuppLanguages( LanguageState *pState, USHORT nCount)
{
    for (USHORT i = 0;  i < nCount;  ++i)
        aLanguages.Insert( pState[i].nLanguage, new INT16( LANG_QUERY ) );
}


SuppLanguages::~SuppLanguages()
{
    INT16 *pItem = aLanguages.First();
    while (pItem)
    {
        INT16 *pTmp = pItem;
        pItem = aLanguages.Next();
        delete pTmp;
    }
}


BOOL SuppLanguages::HasLanguage( INT16 nLanguage ) const
{
    BOOL bRes = FALSE;
    SuppLanguages *pThis = (SuppLanguages *) this;
    INT16 *pEntry = pThis->aLanguages.Seek( nLanguage );
    if (pEntry)
    {
        INT16 nState = *pEntry;
        if (LANG_QUERY == nState)
        {
            nState = CheckLanguage( nLanguage );
            pThis->aLanguages.Replace( nLanguage, new INT16( nState ) );
        }
        bRes = LANG_OK == nState;
    }
    return bRes;
}


const Sequence< INT16 > SuppLanguages::GetLanguages() const
{
    INT32 nLen = aLanguages.Count();
    Sequence< INT16 > aRes( nLen );
    INT16 *pRes = aRes.getArray();
    USHORT nCnt = 0;
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        INT16 nLanguage = aLanguages.GetObjectKey( i );
        if (HasLanguage( nLanguage ))
            pRes[ nCnt++ ] = nLanguage;
    }
    aRes.realloc( nCnt );
    return aRes;
}

///////////////////////////////////////////////////////////////////////////

String  GetModulePath( SvtPathOptions::Pathes ePath, BOOL bAddAccessDelim  )
{
    String aRes;

    SvtPathOptions  aPathOpt;
    switch (ePath)
    {
        case SvtPathOptions::PATH_MODULE :
            aRes = aPathOpt.GetModulePath();
            break;
        case SvtPathOptions::PATH_LINGUISTIC :
        {
            String aTmp( aPathOpt.GetLinguisticPath() );
            LocalFileHelper::ConvertURLToPhysicalName( aTmp, aRes );
            break;
        }
        default:
            DBG_ERROR( "unexpected argument (path)" );
    }
    if (bAddAccessDelim && aRes.Len())
    {
#ifdef WNT
        aRes += '\\';
#else
        aRes += '/';
#endif
    }

    return aRes;
}

///////////////////////////////////////////////////////////////////////////

OUString StripTrailingChars( OUString &rTxt, sal_Unicode cChar )
{
    sal_Int32 nTrailing = 0;
    sal_Int32 nTxtLen = rTxt.getLength();
    sal_Int32 nIdx = nTxtLen - 1;
    while (nIdx >= 0 && rTxt[ nIdx-- ] == cChar)
        ++nTrailing;

    OUString aRes( rTxt.copy( nTxtLen - nTrailing ) );
    rTxt = rTxt.copy( 0, nTxtLen - nTrailing );
    return aRes;
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

