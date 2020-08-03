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
#pragma once

#include <vcl/ctrl.hxx>
#include "rtattributes.hxx"
#include "textattributelistener.hxx"
#include <memory>

class EditView;
class EditEngine;
class SfxItemSet;

namespace frm
{

    class RichTextControlImpl;
    class RichTextEngine;

    class RichTextControl : public Control, public IMultiAttributeDispatcher
    {
    private:
        std::unique_ptr<RichTextControlImpl>    m_pImpl;

    public:
        RichTextControl(
            RichTextEngine* _pEngine,
            vcl::Window* _pParent,
            WinBits _nStyle,
            ITextAttributeListener* _pTextAttribListener,
            ITextSelectionListener* _pSelectionListener
        );

        virtual ~RichTextControl( ) override;
        virtual void dispose() override;

        /* enables the change notifications for a particular attribute

           If you want to be notified of any changes in the state of an attribute, you need to call enableAttributeNotification.

           If you provide a dedicated listener for this attribute, this listener is called for every change in the state of
           the attribute.

           No matter whether you provide such a dedicated listener, the "global" listener which you specified
           in the constructor of the control is also called for all changes in the attribute state.

           If you previously already enabled the notification for this attribute, and specified a different listener,
           then the previous listener will be replaced with the new listener, provided the latter is not <NULL/>.
        */
        void        enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener );

        /** disables the change notifications for a particular attribute

            If there was a listener dedicated to this attribute, it will not be referenced and used anymore
            after this method had been called
        */
        void        disableAttributeNotification( AttributeId _nAttributeId );

        /** determines whether a given slot can be mapped to an aspect of an attribute of the EditEngine

            E.g. SID_ATTR_PARA_ADJUST_LEFT can, though it's not part of the EditEngine pool, be mapped
            to the SID_ATTR_PARA_ADJUST slot, which in fact *is* usable with the EditEngine.
        */
        static bool isMappableSlot( SfxSlotId _nSlotId );

        // IMultiAttributeDispatcher
        virtual AttributeState  getState( AttributeId _nAttributeId ) const override;
        virtual void            executeAttribute( AttributeId _nAttributeId, const SfxPoolItem* _pArgument ) override;

        void    SetBackgroundColor( );
        void    SetBackgroundColor( const Color& _rColor );

        void    SetReadOnly( bool _bReadOnly );
        bool    IsReadOnly() const;

        void    SetHideInactiveSelection( bool _bHide );
        bool    GetHideInactiveSelection() const;

        const EditView& getView() const;
              EditView& getView();

        // Window overridables
        virtual void        Draw( OutputDevice* _pDev, const Point& _rPos, DrawFlags _nFlags ) override;

    protected:
        // Window overridables
        virtual void        Resize() override;
        virtual void        GetFocus() override;
        virtual void        StateChanged( StateChangedType nStateChange ) override;
        virtual bool        PreNotify( NotifyEvent& _rNEvt ) override;
        virtual bool        EventNotify( NotifyEvent& _rNEvt ) override;

    private:
                void    applyAttributes( const SfxItemSet& _rAttributesToApply );
                void    implInit( RichTextEngine* _pEngine, ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener );
        static  WinBits implInitStyle( WinBits nStyle );

    private:
        EditEngine&  getEngine() const;
    };


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
