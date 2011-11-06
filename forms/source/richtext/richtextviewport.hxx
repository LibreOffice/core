/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

