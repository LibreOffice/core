/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basic/global.hxx"
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/instance.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vcl/svapp.hxx>

namespace
{
    class lclTransliterationWrapper
    {
    private:
        utl::TransliterationWrapper m_aTransliteration;
    public:
        lclTransliterationWrapper()
            : m_aTransliteration(
                comphelper::getProcessComponentContext(),
                com::sun::star::i18n::TransliterationModules_IGNORE_CASE )
        {
            const LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
            m_aTransliteration.loadModuleIfNeeded( eOfficeLanguage );
        }
        utl::TransliterationWrapper& getTransliteration() { return m_aTransliteration; }
    };

    class theTransliterationWrapper : public rtl::Static<lclTransliterationWrapper, theTransliterationWrapper> {};
}

utl::TransliterationWrapper& SbGlobal::GetTransliteration()
{
    return theTransliterationWrapper::get().getTransliteration();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
