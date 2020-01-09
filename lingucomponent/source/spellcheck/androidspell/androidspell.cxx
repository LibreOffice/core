/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include <lingutil.hxx>
#include "androidspell.hxx"

#include <linguistic/lngprops.hxx>
#include <linguistic/spelldta.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/strings.hrc>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/log.hxx>

#include <utility>
#include <vector>
#include <set>
#include <string.h>
#include <o3tl/make_unique.hxx>

#include <osl/detail/android-bootstrap.h>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

namespace
{
/// List of locales that we have got via the initSpellCheckingNative JNI call.
std::vector<OUString> gLocales;

/// The gLocales converted to a Sequence of Locales - we cannot do that right away.
Sequence<Locale> gLocalesSequence;

/// The LOActivity class - for calling back.
jclass loActivityClz = nullptr;

/// The LOActivity object - for calling back.
jobject loActivityObj = nullptr;

/// To manage attaching and detaching the thread.
// FIXME TODO some management / remembering of the env maybe? OTOH - this is
// called from the main thread anyway, so we are never doing the
// AttachCurrentThread()...
JNIEnv* getEnv()
{
    JNIEnv* env;

    assert(lo_get_javavm() != nullptr);
    jint res = lo_get_javavm()->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (res == JNI_EDETACHED)
    {
        SAL_INFO("lingucomponent.android", "Attach worker thread");
        res = lo_get_javavm()->AttachCurrentThread(&env, nullptr);
        if (JNI_OK != res)
            SAL_INFO("lingucomponent.android", "Failed to AttachCurrentThread");
    }
    else if (res == JNI_EVERSION)
    {
        SAL_INFO("lingucomponent.android", "GetEnv version not supported");
        return nullptr;
    }
    else if (res != JNI_OK)
    {
        SAL_INFO("lingucomponent.android", "GetEnv another error " << res);
        return nullptr;
    }

    return env;
}
}

/// This has to be called from the Java code before the AndroidSpellChecker
/// initialization - namely to have the JNIEnv to be able to perform calls
/// back to the Java code.
extern "C" JNIEXPORT void JNICALL libreofficekit_spell_checking_initialize(JNIEnv* env,
                                                                           jobject instance,
                                                                           jobjectArray locales)
{
    //SAL-DEBUG("AndroidSpellChecker - libreoffice_spell_checking_initialize() called");
    MutexGuard aGuard(GetLinguMutex());

    int count = env->GetArrayLength(locales);
    for (int i = 0; i < count; ++i)
    {
        jstring jstr = (jstring)(env->GetObjectArrayElement(locales, i));
        const char* localeUtf8 = env->GetStringUTFChars(jstr, nullptr);

        gLocales.push_back(OUString::fromUtf8(localeUtf8).replace('_', '-'));

        env->ReleaseStringUTFChars(jstr, localeUtf8);
    }

    // trigger re-generation
    gLocalesSequence.realloc(0);

    // remember these for calling back later
    jclass clz = env->GetObjectClass(instance);
    loActivityClz = (jclass)env->NewGlobalRef(clz);
    loActivityObj = env->NewGlobalRef(instance);
}

/// We have to switch to the new LOActivity instance when another document is loaded.
extern "C" JNIEXPORT void JNICALL libreofficekit_spell_checking_destroy(JNIEnv* env)
{
    //SAL-DEBUG("AndroidSpellChecker - libreoffice_spell_checking_destroy() called");
    MutexGuard aGuard(GetLinguMutex());

    if (loActivityClz)
    {
        env->DeleteGlobalRef(loActivityClz);
        loActivityClz = nullptr;
    }

    if (loActivityObj)
    {
        env->DeleteGlobalRef(loActivityObj);
        loActivityObj = nullptr;
    }
}

AndroidSpellChecker::AndroidSpellChecker()
    : m_aEvtListeners(GetLinguMutex())
    , m_bDisposing(false)
{
}

AndroidSpellChecker::~AndroidSpellChecker()
{
    if (m_pPropHelper)
        m_pPropHelper->RemoveAsPropListener();
}

PropertyHelper_Spelling& AndroidSpellChecker::GetPropHelper_Impl()
{
    if (!m_pPropHelper)
    {
        Reference<XLinguProperties> xPropSet(GetLinguProperties(), UNO_QUERY);

        m_pPropHelper.reset(
            new PropertyHelper_Spelling(static_cast<XSpellChecker*>(this), xPropSet));
        m_pPropHelper->AddAsPropListener(); // after a reference is established
    }
    return *m_pPropHelper;
}

Sequence<Locale> SAL_CALL AndroidSpellChecker::getLocales()
{
    //SAL-DEBUG("AndroidSpellChecker::getLocales()");
    MutexGuard aGuard(GetLinguMutex());

    if (gLocales.empty())
        return Sequence<Locale>();

    if (gLocalesSequence.hasElements())
        return gLocalesSequence;

    gLocalesSequence.realloc(gLocales.size());
    int i = 0;
    for (const OUString& aLocale : gLocales)
    {
        //SAL-DEBUG("Spell locale: " << aLocale);
        // FIXME don't store invalid locales
        // FIXME make sure we 'languagetag' dir content is added to assets
        gLocalesSequence[i++] = LanguageTag::convertToLocale(aLocale);
        //SAL-DEBUG("Spell locale stored as: " << gLocalesSequence[i - 1].Language << '-' << gLocalesSequence[i - 1].Country);
    }

    return gLocalesSequence;
}

sal_Bool SAL_CALL AndroidSpellChecker::hasLocale(const Locale& rLocale)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!gLocalesSequence.hasElements())
        return false;

    for (const Locale& locale : gLocalesSequence)
    {
        if (locale == rLocale)
            return true;
    }

    return false;
}

sal_Bool SAL_CALL AndroidSpellChecker::isValid(const OUString& rWord, const Locale& rLocale,
                                               const PropertyValues& /*rProperties*/)
{
    MutexGuard aGuard(GetLinguMutex());

    //SAL-DEBUG("AndroidSpellChecker::isValid(): " << rWord);

    if (rLocale == Locale() || rWord.isEmpty() || !loActivityClz || !loActivityObj)
        return false;

    JNIEnv* env = getEnv();
    if (!env)
        return false;

    jstring jstr = env->NewStringUTF(rWord.toUtf8().getStr());
    jmethodID getSpellCheckingSuggestions
        = env->GetMethodID(loActivityClz, "getSpellCheckingSuggestions", "(Ljava/lang/String;)V");
    env->CallVoidMethod(loActivityObj, getSpellCheckingSuggestions, jstr);

    if (env->ExceptionCheck())
        env->ExceptionDescribe();

    // FIXME TODO wait for the suggestions coming back

    return false;
}

Reference<XSpellAlternatives>
    SAL_CALL AndroidSpellChecker::spell(const OUString& rWord, const Locale& rLocale,
                                        const PropertyValues& /*rProperties*/)
{
    MutexGuard aGuard(GetLinguMutex());

    //SAL-DEBUG("AndroidSpellChecker::spell()");

    if (rLocale == Locale() || rWord.isEmpty())
        return nullptr;

    if (!hasLocale(rLocale))
        return nullptr;

    Reference<XSpellAlternatives> xAlt;

    // FIXME TODO fill in the alternatives

    return xAlt;
}

static Reference<XInterface>
SpellChecker_CreateInstance(const Reference<XMultiServiceFactory>& /*rSMgr*/)
{
    Reference<XInterface> xService = static_cast<cppu::OWeakObject*>(new AndroidSpellChecker);
    return xService;
}

sal_Bool SAL_CALL AndroidSpellChecker::addLinguServiceEventListener(
    const Reference<XLinguServiceEventListener>& rxLstnr)
{
    MutexGuard aGuard(GetLinguMutex());

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener(rxLstnr);
    }
    return bRes;
}

sal_Bool SAL_CALL AndroidSpellChecker::removeLinguServiceEventListener(
    const Reference<XLinguServiceEventListener>& rxLstnr)
{
    MutexGuard aGuard(GetLinguMutex());

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener(rxLstnr);
    }
    return bRes;
}

OUString SAL_CALL AndroidSpellChecker::getServiceDisplayName(const Locale& rLocale)
{
    std::locale loc(Translate::Create("svt", LanguageTag(rLocale)));
    return Translate::get(STR_DESCRIPTION_ANDROID_SPELLCHECKER, loc);
}

void SAL_CALL AndroidSpellChecker::initialize(const Sequence<Any>& rArguments)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference<XLinguProperties> xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;
            // rArguments.getConstArray()[1] >>= xDicList;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is no longer used.
            m_pPropHelper.reset(
                new PropertyHelper_Spelling(static_cast<XSpellChecker*>(this), xPropSet));
            m_pPropHelper->AddAsPropListener(); //! after a reference is established
        }
        else
        {
            OSL_FAIL("wrong number of arguments in sequence");
        }
    }
}

void SAL_CALL AndroidSpellChecker::dispose()
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing)
    {
        m_bDisposing = true;
        EventObject aEvtObj(static_cast<XSpellChecker*>(this));
        m_aEvtListeners.disposeAndClear(aEvtObj);
        if (m_pPropHelper)
        {
            m_pPropHelper->RemoveAsPropListener();
            m_pPropHelper.reset();
        }
    }
}

void SAL_CALL AndroidSpellChecker::addEventListener(const Reference<XEventListener>& rxListener)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.addInterface(rxListener);
}

void SAL_CALL AndroidSpellChecker::removeEventListener(const Reference<XEventListener>& rxListener)
{
    MutexGuard aGuard(GetLinguMutex());

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.removeInterface(rxListener);
}

// Service specific part
OUString SAL_CALL AndroidSpellChecker::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL AndroidSpellChecker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL AndroidSpellChecker::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Sequence<OUString> AndroidSpellChecker::getSupportedServiceNames_Static() throw()
{
    Sequence<OUString> aSNS{ SN_SPELLCHECKER };
    return aSNS;
}

extern "C" {
SAL_DLLPUBLIC_EXPORT void* AndroidSpell_component_getFactory(const sal_Char* pImplName,
                                                             void* pServiceManager,
                                                             void* /*pRegistryKey*/)
{
    void* pRet = nullptr;
    if (AndroidSpellChecker::getImplementationName_Static().equalsAscii(pImplName))
    {
        Reference<XSingleServiceFactory> xFactory = cppu::createOneInstanceFactory(
            static_cast<XMultiServiceFactory*>(pServiceManager),
            AndroidSpellChecker::getImplementationName_Static(), SpellChecker_CreateInstance,
            AndroidSpellChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
