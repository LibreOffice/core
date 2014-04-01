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



    //= OClipboardDispatcher

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
        sal_Bool        m_bLastKnownEnabled;

    public:
        OClipboardDispatcher( EditView& _rView, ClipboardFunc _eFunc );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ORichTextFeatureDispatcher
        virtual void    invalidateFeatureState_Broadcast() SAL_OVERRIDE;
        virtual ::com::sun::star::frame::FeatureStateEvent
                        buildStatusEvent() const SAL_OVERRIDE;

    protected:
        /** determines whether our functionality is currently available
            to be overridden for ePaste
        */
        virtual sal_Bool implIsEnabled( ) const;
    };


    //= OPasteClipboardDispatcher

    class OPasteClipboardDispatcher : public OClipboardDispatcher
    {
    private:
        TransferableClipboardListener*  m_pClipListener;
        sal_Bool                        m_bPastePossible;

    public:
        OPasteClipboardDispatcher( EditView& _rView );

    protected:
        virtual ~OPasteClipboardDispatcher();

        // OClipboardDispatcher
        virtual sal_Bool    implIsEnabled( ) const SAL_OVERRIDE;

        // ORichTextFeatureDispatcher
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify ) SAL_OVERRIDE;

    private:
        DECL_LINK( OnClipboardChanged, TransferableDataHelper* );
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
