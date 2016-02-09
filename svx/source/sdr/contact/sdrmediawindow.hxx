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

#ifndef INCLUDED_SVX_SOURCE_SDR_CONTACT_SDRMEDIAWINDOW_HXX
#define INCLUDED_SVX_SOURCE_SDR_CONTACT_SDRMEDIAWINDOW_HXX

#include <avmedia/mediawindow.hxx>

namespace sdr { namespace contact {


class ViewObjectContactOfSdrMediaObj;

class SdrMediaWindow : public ::avmedia::MediaWindow
{
public:

                            SdrMediaWindow( vcl::Window* pParent, ViewObjectContactOfSdrMediaObj& rViewObjContact );
                            virtual ~SdrMediaWindow();

        virtual void        MouseMove( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;

        virtual void        KeyInput( const KeyEvent& rKEvt ) override;
        virtual void        KeyUp( const KeyEvent& rKEvt ) override;

        virtual void        Command( const CommandEvent& rCEvt ) override;

        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

private:

    ViewObjectContactOfSdrMediaObj& mrViewObjectContactOfSdrMediaObj;
};

} }

#endif // INCLUDED_SVX_SOURCE_SDR_CONTACT_SDRMEDIAWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
