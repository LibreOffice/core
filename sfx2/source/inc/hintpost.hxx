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
#ifndef INCLUDED_SFX2_HINTPOST_HXX
#define INCLUDED_SFX2_HINTPOST_HXX

#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <functional>
#include <memory>
#include <sfx2/dispatch.hxx>


class SfxRequest;

/**  [Description]

    This class allows sending unique events via VCL's
    Application::PostUserEvent().  When the User-Event is dispatched,
    the handler <Event()> is called, which calls the Link provided with
    SetEventHdl().

    The instances are held via Ref-Count until a possibly sent
    event has arrived.  If the target dies before delivery,
    the connection must be severed with SetEventHdl(Link()).
*/
class SfxHintPoster final : public SvRefBase
{
private:
    class SfxDispatcher *m_Link;

                    DECL_LINK( DoEvent_Impl, void*, void );

    virtual         ~SfxHintPoster() override;

public:
                    SfxHintPoster(SfxDispatcher *aLink);

    void            Post( std::unique_ptr<SfxRequest> pHint );
                    void            ClearLink();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
