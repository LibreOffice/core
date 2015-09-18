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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_LISTBOXACCESSIBLE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_LISTBOXACCESSIBLE_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

class SvTreeListBox;
class VclSimpleEvent;
class VclWindowEvent;

namespace accessibility
{

    /** helper class which couples its life time to the life time of an
        SvTreeListBox
    */
    class ListBoxAccessibleBase
    {
    private:
        VclPtr<SvTreeListBox> m_pWindow;

    protected:
        inline SvTreeListBox*       getListBox() const
        {
            return  const_cast< ListBoxAccessibleBase* >( this )->m_pWindow;
        }

        inline  bool                    isAlive() const     { return nullptr != m_pWindow; }

    public:
        ListBoxAccessibleBase( SvTreeListBox& _rWindow );

    protected:
        virtual ~ListBoxAccessibleBase( );

        // own overridables
        /// will be called for any VclWindowEvent events broadcasted by our VCL window
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

        /** will be called when our window broadcasts the VCLEVENT_OBJECT_DYING event

            <p>Usually, you derive your class from both ListBoxAccessibleBase and XComponent,
            and call XComponent::dispose here.</p>
        */
        virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) = 0;

        /// to be called in the dispose method of your derived class
        void disposing();

    private:
        DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

    private:
        ListBoxAccessibleBase( const ListBoxAccessibleBase& ) SAL_DELETED_FUNCTION;
        ListBoxAccessibleBase& operator=( const ListBoxAccessibleBase& ) SAL_DELETED_FUNCTION;
    };


}   // namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_LISTBOXACCESSIBLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
