/*************************************************************************
 *
 *  $RCSfile: ItemConverter.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CHART_ITEMCONVERTER_HXX
#define CHART_ITEMCONVERTER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
// #ifndef _SVX_UNOIPSET_HXX_
// #include <svx/unoipset.hxx>
// #endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

class SfxItemPropertyMap;

namespace chart
{
namespace wrapper
{

/** This class serves for conversion between properties of an XPropertySet and
    SfxItems in SfxItemSets.

    <p>With this helper classes, you can feed dialogs with XPropertySets and let
    those modify by the dialogs.</p>

    <p>You must implement GetWhichPairs() such that an SfxItemSet created with
    CreateEmptyItemSet() is able to hold all items that may be mapped.</p>

    <p>You also have to implement GetItemPropertyName(), in order to return the
    property name for a given which-id.</p>

    <p>FillSpecialItem and ApplySpecialItem may be used for special handling of
    individual item, e.g. if you need member-ids in QueryValue/PutValue</p>
 */
class ItemConverter :
        public ::cppu::WeakImplHelper1<
            ::com::sun::star::lang::XEventListener
        >
{
public:
    /** Construct an item converter that uses the given property set for
        reading/writing converted items
     */
    ItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet ,
            SfxItemPool& rItemPool );
    virtual ~ItemConverter();

    /** applies all properties that can be mapped to items into the given item
        set.

        <p>Call this method before opening a dialog.</p>

        @param rOutItemSet the SfxItemSet is filled with all items that are a
               result of a conversion from a property of the internal
               XPropertySet.
     */
    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;

    /** applies all properties that are results of a conversion from all items
        in rItemSet to the internal XPropertySet.

        <p>Call this method after a dialog was closed with OK</p>

        @return true, if any properties have been changed, false otherwise.
     */
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

    /** creates an empty item set using the given pool
        or a common pool if empty (see GetItemPool) and
        allowing all items given in the ranges returned by GetWhichPairs.
     */
    SfxItemSet CreateEmptyItemSet() const;

    /** States whether conversion is still likely to work.

        <p>In particular, it is checked if the XPropertySet given in the CTOR is
        still valid, i.e. not disposed.  It is assumed that the XPropertySet is
        valid when the converter is constructed.</p>

        <p>This only works if the XPropertySet given in the CTOR supports the
        interface ::com::sun::star::lang::XComponent.</p>
     */
    bool IsValid() const;

    /// creates a pure chart item pool
    static SfxItemPool* CreateSchItemPool();

    static void InvalidateUnequalItems( SfxItemSet &rDestSet, const SfxItemSet &rSourceSet );

protected:
    // ________

    /** implement this method to provide an array of which-ranges of the form:

       <pre>
        const USHORT aMyPairs[] =
        {
          from_1, to_1,
          from_2, to_2,
          ...
          from_n, to_n,
          0
        };
        </pre>
    */
    virtual const USHORT * GetWhichPairs() const = 0;

    /** implement this method to return a Property object for a given which id.

        @param rOutName If </TRUE> is returned, this contains the property name.

        @return </TRUE>, if the item can be mapped to a property.
     */
    virtual bool GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const;

    /** for items that can not be mapped directly to a property.

        This method is called from FillItemSet(), if GetItemPropertyName() returns
        false.
     */
    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const;

    /** for items that can not be mapped directly to a property.

        This method is called from ApplyItemSet(), if GetItemPropertyName() returns
        false.

        @return </TRUE> if the item changed a property, </FALSE> otherwise.
     */
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet ) const;

    // ________

    /// Returns the pool
    SfxItemPool & GetItemPool() const;

    /** Returns the XPropertySet that was given in the CTOR and is used to apply
        items in ApplyItemSet().
     */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >  GetPropertySet() const;

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >     m_xPropertySet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo > m_xPropertySetInfo;

    SfxItemPool&                                    m_rItemPool;
    bool                                            m_bIsValid;
};

} //  namespace wrapper
} //  namespace chart

// CHART_ITEMCONVERTER_HXX
#endif
