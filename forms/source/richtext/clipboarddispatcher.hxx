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

#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

#include "featuredispatcher.hxx"
#include <tools/link.hxx>

class TransferableClipboardListener;
class TransferableDataHelper;

namespace frm
{

    class OClipboardDispatcher : public ORichTextFeatureDispatcher
    {
    public:
        enum ClipboardFunc
        {
            eCut,
            eCopy,
            ePaste
        };

    private:
        ClipboardFunc   m_eFunc;
        bool        m_bLastKnownEnabled;

    public:
        OClipboardDispatcher( EditView& _rView, ClipboardFunc _eFunc );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) throw (css::uno::RuntimeException, std::exception) override;

        // ORichTextFeatureDispatcher
        virtual void    invalidateFeatureState_Broadcast() override;
        virtual css::frame::FeatureStateEvent
                        buildStatusEvent() const override;

    protected:
        /** determines whether our functionality is currently available
            to be overridden for ePaste
        */
        virtual bool implIsEnabled( ) const;
    };

    class OPasteClipboardDispatcher : public OClipboardDispatcher
    {
    private:
        TransferableClipboardListener*  m_pClipListener;
        bool                        m_bPastePossible;

    public:
        explicit OPasteClipboardDispatcher( EditView& _rView );

    protected:
        virtual ~OPasteClipboardDispatcher();

        // OClipboardDispatcher
        virtual bool    implIsEnabled( ) const override;

        // ORichTextFeatureDispatcher
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify ) override;

    private:
        DECL_LINK_TYPED( OnClipboardChanged, TransferableDataHelper*, void );
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
