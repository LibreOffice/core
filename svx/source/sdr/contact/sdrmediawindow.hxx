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

#include <avmedia/mediawindow.hxx>

namespace sdr { namespace contact {


// - SdrMediaWindow -


class ViewObjectContactOfSdrMediaObj;

class SdrMediaWindow : public ::avmedia::MediaWindow
{
public:

                            SdrMediaWindow( Window* pParent, ViewObjectContactOfSdrMediaObj& rViewObjContact );
                            virtual ~SdrMediaWindow();

        virtual void        MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;

        virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void        KeyUp( const KeyEvent& rKEvt ) SAL_OVERRIDE;

        virtual void        Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

private:

    ViewObjectContactOfSdrMediaObj& mrViewObjectContactOfSdrMediaObj;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
