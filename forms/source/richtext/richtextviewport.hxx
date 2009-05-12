/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: richtextviewport.hxx,v $
 * $Revision: 1.5 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX

#include <vcl/ctrl.hxx>

class EditView;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= RichTextViewPort
    //====================================================================
    class RichTextViewPort : public Control
    {
    private:
        EditView*   m_pView;
        Link        m_aInvalidationHandler;
        bool        m_bHideInactiveSelection;

    public:
        RichTextViewPort( Window* _pParent );

        void    setView( EditView& _rView );

        inline void    setAttributeInvalidationHandler( const Link& _rHandler ) { m_aInvalidationHandler = _rHandler; }

        void    SetHideInactiveSelection( bool _bHide );
        bool    GetHideInactiveSelection() const;

    protected:
        virtual void        Paint( const Rectangle& rRect );
        virtual void        GetFocus();
        virtual void        LoseFocus();
        virtual void        KeyInput( const KeyEvent& _rKEvt );
        virtual void        MouseMove( const MouseEvent& _rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& _rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& _rMEvt );

    private:
        inline void implInvalidateAttributes() const
        {
            if ( m_aInvalidationHandler.IsSet() )
                m_aInvalidationHandler.Call( NULL );
        }
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX

