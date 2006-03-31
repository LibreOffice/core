/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontdialog.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-31 12:19:15 $
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

#ifndef _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_FONTDIALOG_HXX_

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

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

