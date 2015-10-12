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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PRIVSPLT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PRIVSPLT_HXX

#include <vcl/ctrl.hxx>

class ScPrivatSplit : public Control
{
private:

        Link<ScPrivatSplit&,void> aCtrModifiedLink;
        bool                aMovingFlag;
        Pointer             aWinPointer;
        short               nOldX;
        short               nOldY;
        short               nNewX;
        short               nNewY;
        Range               aXMovingRange;
        Range               aYMovingRange;
        short               nDeltaY;

        using Control::ImplInitSettings;
        void                ImplInitSettings( bool bFont, bool bForeground, bool bBackground );

protected:
        virtual void        MouseMove( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt) override;

public:

        ScPrivatSplit(vcl::Window* pWindow, const ResId& rResId);

        short           GetDeltaY();

        void            CtrModified();

        void            SetYRange(Range cRgeY);

        void            MoveSplitTo(Point aPos);

        virtual void    StateChanged( StateChangedType nType ) override;
        virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

        void            SetCtrModifiedHdl( const Link<ScPrivatSplit&,void>& rLink ) { aCtrModifiedLink = rLink; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
