/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: specialdispatchers.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:31 $
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

#ifndef FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX
#define FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX

#ifndef FORMS_SOURCE_RICHTEXT_PARAMETRIZEDATTRIBUTEDISPATCHER_HXX
#include "parametrizedattributedispatcher.hxx"
#endif

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
