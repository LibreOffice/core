/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: specialdispatchers.hxx,v $
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

#ifndef FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX
#define FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX

#include "parametrizedattributedispatcher.hxx"

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OSelectAllDispatcher
    //====================================================================
    class OSelectAllDispatcher : public ORichTextFeatureDispatcher
    {
    public:
        OSelectAllDispatcher( EditView& _rView, const ::com::sun::star::util::URL&  _rURL );

    protected:
        ~OSelectAllDispatcher();

        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);

        // ORichTextFeatureDispatcher
        virtual ::com::sun::star::frame::FeatureStateEvent  buildStatusEvent() const;
    };

    //====================================================================
    //= OParagraphDirectionDispatcher
    //====================================================================
    class OParagraphDirectionDispatcher : public OAttributeDispatcher
    {
    public:
        OParagraphDirectionDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const ::com::sun::star::util::URL&  _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        // ORichTextFeatureDispatcher
        virtual ::com::sun::star::frame::FeatureStateEvent  buildStatusEvent() const;
    };

    //====================================================================
    //= OTextDirectionDispatcher
    //====================================================================
    class OTextDirectionDispatcher : public ORichTextFeatureDispatcher
    {
    public:
        OTextDirectionDispatcher( EditView& _rView, const ::com::sun::star::util::URL&  _rURL );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);

        // ORichTextFeatureDispatcher
        virtual ::com::sun::star::frame::FeatureStateEvent  buildStatusEvent() const;
    };

    //====================================================================
    //= OAsianFontLayoutDispatcher
    //====================================================================
    class OAsianFontLayoutDispatcher : public OParametrizedAttributeDispatcher
    {
    public:
        OAsianFontLayoutDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const ::com::sun::star::util::URL&  _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        // OParametrizedAttributeDispatcher
        virtual const SfxPoolItem* convertDispatchArgsToItem(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX
