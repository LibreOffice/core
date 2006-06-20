/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringlistitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:19:12 $
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

#ifndef _DBAUI_STRINGLISTITEM_HXX_
#define _DBAUI_STRINGLISTITEM_HXX_

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OStringListItem
//=========================================================================
/** <type>SfxPoolItem</type> which transports a sequence of <type scope="rtl">OUString</type>'s
*/
class OStringListItem : public SfxPoolItem
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >      m_aList;

public:
    TYPEINFO();
    OStringListItem(sal_Int16 nWhich, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rList);
    OStringListItem(const OStringListItem& _rSource);

    virtual int              operator==(const SfxPoolItem& _rItem) const;
    virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  getList() const { return m_aList; }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_STRINGLISTITEM_HXX_

