/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <iostream>

#include <tools/debug.hxx>

#include <sal/config.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase2.hxx>
#include <tools/string.hxx>

#include <simpleguesser.hxx>
#include <guess.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <unotools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <osl/thread.h>

#include <sal/macros.h>

#include <libexttextcat/textcat.h>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace css = ::com::sun::star;

//==================================================================================================

#define A2OU(x) ::rtl::OUString::createFromAscii( x )

#define SERVICENAME     "com.sun.star.linguistic2.LanguageGuessing"
#define IMPLNAME        "com.sun.star.lingu2.LanguageGuessing"

static Sequence< OUString > getSupportedServiceNames_LangGuess_Impl()
{
    Sequence<OUString> names(1);
    names[0] = A2OU( SERVICENAME );
    return names;
}

static OUString getImplementationName_LangGuess_Impl()
{
    return A2OU( IMPLNAME );
}

static osl::Mutex &  GetLangGuessMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}


class LangGuess_Impl :
    public ::cppu::WeakImplHelper2<
        XLanguageGuessing,
        XServiceInfo >
{
    SimpleGuesser   m_aGuesser;
    bool            m_bInitialized;
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;

    LangGuess_Impl( const LangGuess_Impl & ); // not defined
    LangGuess_Impl & operator =( const LangGuess_Impl & ); // not defined

    virtual ~LangGuess_Impl() {}
    void    EnsureInitialized();

public:
    explicit LangGuess_Impl(css::uno::Reference< css::uno::XComponentContext > const & rxContext);

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XLanguageGuessing implementation
    virtual ::com::sun::star::lang::Locale SAL_CALL guessPrimaryLanguage( const ::rtl::OUString& aText, ::sal_Int32 nStartPos, ::sal_Int32 nLen ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disableLanguages( const ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >& aLanguages ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL enableLanguages( const ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >& aLanguages ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getAvailableLanguages(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getEnabledLanguages(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getDisabledLanguages(  ) throw (::com::sun::star::uno::RuntimeException);

    // implementation specific
    void SetFingerPrintsDB( const rtl::OUString &fileName ) throw (RuntimeException);

    static const OUString & SAL_CALL getImplementationName_Static() throw();

};

//*************************************************************************

LangGuess_Impl::LangGuess_Impl(css::uno::Reference< css::uno::XComponentContext > const & rxContext) :
    m_bInitialized( false ),
    m_xContext( rxContext )
{
}

//*************************************************************************

void LangGuess_Impl::EnsureInitialized()
{
    if (!m_bInitialized)
    {
        // set this to true at the very start to prevent loops because of
        // implicitly called functions below
        m_bInitialized = true;

        // set default fingerprint path to where those get installed
        rtl::OUString aPhysPath;
        rtl::OUString aURL( SvtPathOptions().GetFingerprintPath() );
        utl::LocalFileHelper::ConvertURLToPhysicalName( aURL, aPhysPath );
#ifdef WNT
        aPhysPath = aPhysPath + rtl::OUString(static_cast<sal_Unicode>('\\'));
#else
        aPhysPath = aPhysPath + rtl::OUString(static_cast<sal_Unicode>('/'));
#endif

        SetFingerPrintsDB( aPhysPath );

#if !defined(EXTTEXTCAT_VERSION_MAJOR)
        //
        // disable currently not functional languages...
        //
        struct LangCountry
        {
            const char *pLang;
            const char *pCountry;
        };
        LangCountry aDisable[] =
        {
            // not functional in modified libtextcat, but fixed in >= libexttextcat 3.1.0
            // which is the first with EXTTEXTCAT_VERSION_MAJOR defined

            {"sco", ""}, {"zh", "CN"}, {"zh", "TW"}, {"ja", ""}, {"ko", ""},
            {"ka", ""}, {"hi", ""}, {"mr", ""}, {"ne", ""}, {"sa", ""},
            {"ta", ""}, {"th", ""}, {"qu", ""}, {"yi", ""}
        };
        sal_Int32 nNum = SAL_N_ELEMENTS(aDisable);
        Sequence< Locale > aDisableSeq( nNum );
        Locale *pDisableSeq = aDisableSeq.getArray();
        for (sal_Int32 i = 0;  i < nNum;  ++i)
        {
            Locale aLocale;
            aLocale.Language = OUString::createFromAscii( aDisable[i].pLang );
            aLocale.Country  = OUString::createFromAscii( aDisable[i].pCountry );
            pDisableSeq[i] = aLocale;
        }
        disableLanguages( aDisableSeq );
        DBG_ASSERT( nNum == getDisabledLanguages().getLength(), "size mismatch" );
#endif
    }
}

//*************************************************************************

Locale SAL_CALL LangGuess_Impl::guessPrimaryLanguage(
        const ::rtl::OUString& rText,
        ::sal_Int32 nStartPos,
        ::sal_Int32 nLen )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    lang::Locale aRes;
    if (nStartPos >=0 && nLen >= 0 && nStartPos + nLen <= rText.getLength())
    {
        OString o( OUStringToOString( rText.copy(nStartPos, nLen), RTL_TEXTENCODING_UTF8 ) );
        Guess g = m_aGuesser.GuessPrimaryLanguage(o.getStr());
        aRes.Language   = OUString::createFromAscii(g.GetLanguage().c_str());
        aRes.Country    = OUString::createFromAscii(g.GetCountry().c_str());
    }
    else
        throw lang::IllegalArgumentException();

    return aRes;
}

//*************************************************************************
#define DEFAULT_CONF_FILE_NAME "fpdb.conf"

void LangGuess_Impl::SetFingerPrintsDB(
        const rtl::OUString &filePath )
    throw (RuntimeException)
{
    //! text encoding for file name / path needs to be in the same encoding the OS uses
    OString path = OUStringToOString( filePath, osl_getThreadTextEncoding() );
    OString conf_file_name( DEFAULT_CONF_FILE_NAME );
    OString conf_file_path(path);
    conf_file_path += conf_file_name;

    m_aGuesser.SetDBPath((const char*)conf_file_path.getStr(), (const char*)path.getStr());
}

//*************************************************************************
uno::Sequence< Locale > SAL_CALL LangGuess_Impl::getAvailableLanguages(  )
        throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    Sequence< com::sun::star::lang::Locale > aRes;
    vector<Guess> gs = m_aGuesser.GetAllManagedLanguages();
    aRes.realloc(gs.size());

    com::sun::star::lang::Locale *pRes = aRes.getArray();

    for(size_t i = 0; i < gs.size() ; i++ ){
        com::sun::star::lang::Locale current_aRes;
        current_aRes.Language   = A2OU( gs[i].GetLanguage().c_str() );
        current_aRes.Country    = A2OU( gs[i].GetCountry().c_str() );
        pRes[i] = current_aRes;
    }

    return aRes;
}

//*************************************************************************
uno::Sequence< Locale > SAL_CALL LangGuess_Impl::getEnabledLanguages(  )
        throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    Sequence< com::sun::star::lang::Locale > aRes;
    vector<Guess> gs = m_aGuesser.GetAvailableLanguages();
    aRes.realloc(gs.size());

    com::sun::star::lang::Locale *pRes = aRes.getArray();

    for(size_t i = 0; i < gs.size() ; i++ ){
        com::sun::star::lang::Locale current_aRes;
        current_aRes.Language   = A2OU( gs[i].GetLanguage().c_str() );
        current_aRes.Country    = A2OU( gs[i].GetCountry().c_str() );
        pRes[i] = current_aRes;
    }

    return aRes;
}

//*************************************************************************
uno::Sequence< Locale > SAL_CALL LangGuess_Impl::getDisabledLanguages(  )
        throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    Sequence< com::sun::star::lang::Locale > aRes;
    vector<Guess> gs = m_aGuesser.GetUnavailableLanguages();
    aRes.realloc(gs.size());

    com::sun::star::lang::Locale *pRes = aRes.getArray();

    for(size_t i = 0; i < gs.size() ; i++ ){
        com::sun::star::lang::Locale current_aRes;
        current_aRes.Language   = A2OU( gs[i].GetLanguage().c_str() );
        current_aRes.Country    = A2OU( gs[i].GetCountry().c_str() );
        pRes[i] = current_aRes;
    }

    return aRes;
}

//*************************************************************************
void SAL_CALL LangGuess_Impl::disableLanguages(
        const uno::Sequence< Locale >& rLanguages )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    sal_Int32 nLanguages = rLanguages.getLength();
    const Locale *pLanguages = rLanguages.getConstArray();

    for (sal_Int32 i = 0;  i < nLanguages;  ++i)
    {
        string language;

        OString l = OUStringToOString( pLanguages[i].Language, RTL_TEXTENCODING_ASCII_US );
        OString c = OUStringToOString( pLanguages[i].Country, RTL_TEXTENCODING_ASCII_US );

        language += l.getStr();
        language += "-";
        language += c.getStr();
        m_aGuesser.DisableLanguage(language);
    }
}

//*************************************************************************
void SAL_CALL LangGuess_Impl::enableLanguages(
        const uno::Sequence< Locale >& rLanguages )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );

    EnsureInitialized();

    sal_Int32 nLanguages = rLanguages.getLength();
    const Locale *pLanguages = rLanguages.getConstArray();

    for (sal_Int32 i = 0;  i < nLanguages;  ++i)
    {
        string language;

        OString l = OUStringToOString( pLanguages[i].Language, RTL_TEXTENCODING_ASCII_US );
        OString c = OUStringToOString( pLanguages[i].Country, RTL_TEXTENCODING_ASCII_US );

        language += l.getStr();
        language += "-";
        language += c.getStr();
        m_aGuesser.EnableLanguage(language);
    }
}

//*************************************************************************
OUString SAL_CALL LangGuess_Impl::getImplementationName(  )
    throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );
    return A2OU( IMPLNAME );
}

//*************************************************************************
sal_Bool SAL_CALL LangGuess_Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL LangGuess_Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLangGuessMutex() );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL LangGuess_Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( A2OU( SERVICENAME ) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************


/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL LangGuess_Impl_create(
    Reference< XComponentContext > const & xContext )
    SAL_THROW(())
{
    return static_cast< ::cppu::OWeakObject * >( new LangGuess_Impl(xContext) );
}

//##################################################################################################
//#### EXPORTED ### functions to allow for registration and creation of the UNO component
//##################################################################################################

static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    {
        LangGuess_Impl_create, getImplementationName_LangGuess_Impl,
        getSupportedServiceNames_LangGuess_Impl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL guesslang_component_getFactory(
    sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
    registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(
        implName, xMgr, xRegistry, s_component_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
