/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_RECEIVER_HXX
#define _SD_IMPRESSREMOTE_RECEIVER_HXX

#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <osl/socket.hxx>
#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

using namespace com::sun::star::presentation;
using namespace com::sun::star::uno;
namespace sd
{

class Receiver
{
public:
    Receiver();
    ~Receiver();
    void parseCommand( const char* aCommand, sal_Int32 size, osl::StreamSocket &aSocket );


private:
    void executeCommand( JsonObject *aObject, Reference<XSlideShowController> aController );

};

}

Sequence<sal_Int8> preparePreview(sal_uInt32 aSlideNumber, Reference<XSlideShowController> xSlideShowController, sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &aSize );

void sendPreview(sal_uInt32 aSlideNumber, Reference<XSlideShowController> xSlideShowController, osl::StreamSocket &mStreamSocket );

#endif // _SD_IMPRESSREMOTE_RECEIVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */