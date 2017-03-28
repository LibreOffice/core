/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_COMPHELPER_PROFILEZONE_HXX
#define INCLUDED_COMPHELPER_PROFILEZONE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/comphelperdllapi.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <vector>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{

namespace ProfileRecording
{

COMPHELPER_DLLPUBLIC void startRecording(bool bRecording = true);

COMPHELPER_DLLPUBLIC long long addRecording(const char * aProfileId, long long aCreateTime);

COMPHELPER_DLLPUBLIC css::uno::Sequence<OUString> getRecordingAndClear();

} // namespace ProfileRecording

class COMPHELPER_DLLPUBLIC ProfileZone
{
private:
    const char * m_sProfileId;
    long long m_aCreateTime;
public:
    ProfileZone(const char * sProfileId);
    ~ProfileZone();
};

} // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROFILEZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
