/*************************************************************************
 *
 *  $RCSfile: clipboarddispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:11:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

