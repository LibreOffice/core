/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clipboarddispatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:04:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_CLIPBOARDDISPATCHER_HXX

#ifndef FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX
#include "featuredispatcher.hxx"
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

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

