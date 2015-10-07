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
#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX

#include <vcl/ctrl.hxx>

class EditView;

namespace frm
{
    class RichTextViewPort : public Control
    {
    private:
        EditView*   m_pView;
        Link<LinkParamNone*,void> m_aInvalidationHandler;
        bool        m_bHideInactiveSelection;

    public:
        explicit RichTextViewPort( vcl::Window* _pParent );

        void    setView( EditView& _rView );

        inline void    setAttributeInvalidationHandler( const Link<LinkParamNone*,void>& _rHandler ) { m_aInvalidationHandler = _rHandler; }

        void    SetHideInactiveSelection( bool _bHide );
        bool    GetHideInactiveSelection() const { return m_bHideInactiveSelection; }

    protected:
        virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;
        virtual void        GetFocus() SAL_OVERRIDE;
        virtual void        LoseFocus() SAL_OVERRIDE;
        virtual void        KeyInput( const KeyEvent& _rKEvt ) SAL_OVERRIDE;
        virtual void        MouseMove( const MouseEvent& _rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonDown( const MouseEvent& _rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonUp( const MouseEvent& _rMEvt ) SAL_OVERRIDE;

    private:
        inline void implInvalidateAttributes() const
        {
            if ( m_aInvalidationHandler.IsSet() )
                m_aInvalidationHandler.Call( NULL );
        }
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
