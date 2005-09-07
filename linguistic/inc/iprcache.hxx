/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iprcache.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:46:22 $
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

#ifndef _LINGUISTIC_IPRCACHE_HXX_
#define _LINGUISTIC_IPRCACHE_HXX_


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx> // helper for implementations

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif


namespace linguistic
{

class IPRCachedWord;

///////////////////////////////////////////////////////////////////////////

class Flushable
{
public:
    virtual void    Flush() = 0;
};

///////////////////////////////////////////////////////////////////////////

class FlushListener :
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XDictionaryListEventListener,
        ::com::sun::star::beans::XPropertyChangeListener
    >
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >    xDicList;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >             xPropSet;
    Flushable                                              *pFlushObj;

    // don't allow to use copy-constructor and assignment-operator
    FlushListener(const FlushListener &);
    FlushListener & operator = (const FlushListener &);

public:
    FlushListener( Flushable *pFO );
    virtual ~FlushListener();

    inline void SetFlushObj( Flushable *pFO)    { pFlushObj = pFO; }

    void        SetDicList(
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XDictionaryList > &rDL );
    void        SetPropSet(
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > &rPS );

    //XEventListener
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& rSource )
            throw(::com::sun::star::uno::RuntimeException);

    // XDictionaryListEventListener
    virtual void SAL_CALL
        processDictionaryListEvent(
                const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent )
            throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException);
};

///////////////////////////////////////////////////////////////////////////

class IPRSpellCache :
    public Flushable
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryListEventListener >
                        xFlushLstnr;
    FlushListener      *pFlushLstnr;

    IPRCachedWord   **ppHash;
    IPRCachedWord    *pFirst;
    IPRCachedWord    *pLast;
    IPRCachedWord    *pRun;
    IPRCachedWord    *pInput;
    ULONG             nIndex;
    ULONG             nCount;
    ULONG             nInputPos;
    ULONG             nInputValue;
    ULONG             nTblSize;
#ifdef DBG_STATISTIC
    ULONG nMaxInput;
    ULONG nMax;
    ULONG nFound;
    ULONG nLost;
#endif

    // don't allow to use copy-constructor and assignment-operator
    IPRSpellCache(const IPRSpellCache &);
    IPRSpellCache & operator = (const IPRSpellCache &);

public:
    IPRSpellCache( ULONG nSize );
    virtual ~IPRSpellCache();

    // Flushable
    virtual void    Flush();

    void    AddWord( const String& rWord, INT16 nLang );
    BOOL    CheckWord( const String& rWord, INT16 nLang, BOOL bAllLang );
};

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

