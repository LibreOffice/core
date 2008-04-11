/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sjapplet.cxx,v $
 * $Revision: 1.16 $
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

#include "sal/config.h"

#include "sjapplet.hxx"

#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"

#include "sjapplet_impl.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

SjApplet2::SjApplet2()
  : _pImpl(new SjApplet2_Impl())
{
}

SjApplet2::~SjApplet2()
{
    delete _pImpl;
}

//=========================================================================
void SjApplet2::Init(
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > const & context,
    Window * pParentWin, const INetURLObject & rDocBase,
    const SvCommandList & rCmdList)
{
    try {
        if(_pImpl)
            _pImpl->init(pParentWin, context, rDocBase, rCmdList);
    }
    catch(RuntimeException & runtimeException) {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("sjapplet.cxx: SjApplet2::Init - exception occurred: %s\n", message.getStr());
#else
        (void) runtimeException; // avoid warning
#endif

        delete _pImpl;
        _pImpl = 0;
    }
}

//=========================================================================
void SjApplet2::setSizePixel( const Size & rSize )
{
    if(_pImpl)
        _pImpl->setSize(rSize);
}

void SjApplet2::appletRestart()
{
    if(_pImpl)
        _pImpl->restart();
}

void SjApplet2::appletReload()
{
    if(_pImpl)
        _pImpl->reload();
}

void SjApplet2::appletStart()
{
    if(_pImpl)
        _pImpl->start();
}

void SjApplet2::appletStop()
{
    if(_pImpl)
        _pImpl->stop();
}

void SjApplet2::appletClose()
{
    if(_pImpl)
        _pImpl->close();
}

// Fuer SO3, Wrapper fuer Applet liefern
SjJScriptAppletObject * SjApplet2::GetJScriptApplet()
{
    OSL_TRACE("SjApplet2::GetJScriptApplet\n");

    return NULL;
}

// Settings are detected by the JavaVM service
// This function is not necessary anymore
void SjApplet2::settingsChanged(void)
{}
