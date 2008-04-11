/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontdialog.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_

#include <sfx2/tabdlg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

class SvxFontListItem;
class FontList;

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= ControlCharacterDialog
    //========================================================================
    class ControlCharacterDialog : public SfxTabDialog
    {
    public:
        ControlCharacterDialog(Window* _pParent, const SfxItemSet& _rCoreSet);
        ~ControlCharacterDialog();

        /// creates an item set to be used with this dialog
        static SfxItemSet*  createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

        /// destroys an item previously created with <method>createItemSet</method>
        static void         destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

        /// fills the given item set with values obtained from the given property set
        static void         translatePropertiesToItems(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel,
            SfxItemSet* _pSet);

        /** fills the given property set with values obtained from the given item set
        */
        static void translateItemsToProperties(
            const SfxItemSet& _rSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel);

        /** fills the given property set with values obtained from the given item set
        */
        static void translateItemsToProperties(
            const SfxItemSet& _rSet,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _out_properties );

    protected:
        virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage);
    };
//............................................................................
}   // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_

