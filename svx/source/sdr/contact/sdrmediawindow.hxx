/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SDR_CONTACT_SDRMEDIAWIMNDOW_HXX

#include <avmedia/mediawindow.hxx>

namespace sdr { namespace contact {

// ------------------
// - SdrMediaWindow -
// ------------------

class ViewObjectContactOfSdrMediaObj;

class SdrMediaWindow : public ::avmedia::MediaWindow
{
public:

                            SdrMediaWindow( Window* pParent, ViewObjectContactOfSdrMediaObj& rViewObjContact );
                            ~SdrMediaWindow();

        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& rMEvt );

        virtual void        KeyInput( const KeyEvent& rKEvt );
        virtual void        KeyUp( const KeyEvent& rKEvt );

        virtual void        Command( const CommandEvent& rCEvt );

        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

private:

    ViewObjectContactOfSdrMediaObj& mrViewObjectContactOfSdrMediaObj;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
