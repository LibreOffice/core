/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_COMPATIBILITYVIEWOPTIONS_HXX
#define INCLUDED_SVTOOLS_COMPATIBILITYVIEWOPTIONS_HXX

#include <svtools/svtdllapi.h>
#include <unotools/options.hxx>
#include <osl/mutex.hxx>

class SvtCompatibilityViewOptions_Impl;

/// Compatibility settings which affects the GUI. These are not document level settings
/// (like SvtCompatibilityOptions), but global options affecting the application's
/// behavior in general.
class UNOTOOLS_DLLPUBLIC SvtCompatibilityViewOptions : public utl::detail::Options
{
public:
    SvtCompatibilityViewOptions();
    virtual ~SvtCompatibilityViewOptions() override;

    bool HasMSOCompatibleFormsMenu() const;
    void SetMSOCompatibleFormsMenu(bool bSet);

private:
    /// Return a reference to a static mutex
    UNOTOOLS_DLLPRIVATE static osl::Mutex& GetOwnStaticMutex();

    std::shared_ptr<SvtCompatibilityViewOptions_Impl> m_pImpl;
};

#endif // #ifndef INCLUDED_SVTOOLS_COMPATIBILITYVIEWOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
