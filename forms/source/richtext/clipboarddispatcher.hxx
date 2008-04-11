/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: clipboarddispatcher.hxx,v $
 * $Revision: 1.4 $
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

#ifndef FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

#include "featuredispatcher.hxx"
#include <tools/link.hxx>

class TransferableClipboardListener;
class TransferableDataHelper;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OClipboardDispatcher
    //====================================================================
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
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);

        // ORichTextFeatureDispatcher
        virtual void    invalidateFeatureState_Broadcast();
        virtual ::com::sun::star::frame::FeatureStateEvent
                        buildStatusEvent() const;

    protected:
        /** determines whether our functionality is currently available
            to be overridden for ePaste
        */
        virtual sal_Bool implIsEnabled( ) const;
    };

    //====================================================================
    //= OPasteClipboardDispatcher
    //====================================================================
    class OPasteClipboardDispatcher : public OClipboardDispatcher
    {
    private:
        TransferableClipboardListener*  m_pClipListener;
        sal_Bool                        m_bPastePossible;

    public:
        OPasteClipboardDispatcher( EditView& _rView );

    protected:
        ~OPasteClipboardDispatcher();

        // OClipboardDispatcher
        virtual sal_Bool    implIsEnabled( ) const;

        // ORichTextFeatureDispatcher
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify );

    private:
        DECL_LINK( OnClipboardChanged, TransferableDataHelper* );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

