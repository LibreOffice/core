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
#ifndef INCLUDED_SVTOOLS_HYPERLABEL_HXX
#define INCLUDED_SVTOOLS_HYPERLABEL_HXX

#include <memory>


#include <vcl/fixed.hxx>

#define LABELBASEMAPHEIGHT      8


namespace svt
{
    class HyperLabelImpl;

    class HyperLabel final : public FixedText
    {
        std::unique_ptr<HyperLabelImpl>     m_pImpl;
        Link<HyperLabel*,void>  maClickHdl;

        virtual void        MouseMove( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        GetFocus() override;
        virtual void        LoseFocus() override;

        void                implInit();

        using FixedText::CalcMinimumSize;

    public:
        HyperLabel( vcl::Window* _pParent, WinBits _nWinStyle );
        virtual ~HyperLabel( ) override;
        virtual void dispose() override;

        virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
        virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

        void                SetID( sal_Int16 ID );
        sal_Int16           GetID() const;

        void                SetIndex( sal_Int32 Index );
        sal_Int32           GetIndex() const;

        void                SetLabel( const OUString& _rText );

        void                ToggleBackgroundColor( const Color& _rGBColor );
        void                SetInteractive( bool _bInteractive );

        void                SetClickHdl( const Link<HyperLabel*,void>& rLink ) { maClickHdl = rLink; }

        Size const &        CalcMinimumSize( long nMaxWidth ) const;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
