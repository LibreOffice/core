/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <embeddedobj/embobjdllapi.h>

namespace com
{
namespace sun
{
namespace star
{
namespace embed
{
/** This class is only used to tell the OCommonEmbeddedObject class
    that the following call is an Link- / Ole-refresh.

    @since LibreOffice 7.4
 */
class EMBOBJ_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") EmbeddedUpdate
{
    /** By "Refresh all" is used to perform an OLE update and a link update.
        In the case of a link update, the class OCommonEmbeddedObject is
        informed with true that a link update will take place next

        @param bIsOleUpdate
            what kind of update, when true  is OLE-Object
                                 when false is Link-Object
     */
public:
    virtual void SetOleState(bool bIsOleUpdate) = 0;

    EmbeddedUpdate() = default;
    virtual ~EmbeddedUpdate() = default;

private:
    EmbeddedUpdate(const EmbeddedUpdate&) = delete;
    EmbeddedUpdate& operator=(const EmbeddedUpdate&) = delete;
};
};
};
};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
