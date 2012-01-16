/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Novell, Inc.
 *   Michael Meeks <michael.meeks@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <android/androidinst.hxx>
#include <headless/svpdummies.hxx>
#include <generic/gendata.hxx>
#include <android/log.h>

AndroidSalInstance::AndroidSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
{
    fprintf (stderr, "created Android Sal Instance\n");
}

AndroidSalInstance::~AndroidSalInstance()
{
    fprintf (stderr, "destroyed Android Sal Instance\n");
}

class AndroidSalSystem : public SvpSalSystem {
public:
    AndroidSalSystem() : SvpSalSystem() {}
    virtual ~AndroidSalSystem() {}
    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
                                  int nDefButton )
    {
        (void)rButtons; (void)nDefButton;
        __android_log_print(ANDROID_LOG_INFO, "LibreOffice - dialog '%s': '%s'",
                            rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
                            rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        return 0;
    }
};

SalSystem *AndroidSalInstance::CreateSalSystem()
{
    return new AndroidSalSystem();
}

class AndroidSalData : public SalGenericData
{
public:
    AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

SalInstance *CreateSalInstance()
{
    SvpSalInstance* pInstance = new SvpSalInstance( new SalYieldMutex() );
    new AndroidSalData( pInstance );
    return pInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
