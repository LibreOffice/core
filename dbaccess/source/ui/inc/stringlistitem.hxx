/*************************************************************************
 *
 *  $RCSfile: stringlistitem.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:09:14 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _RTL_USTRING_
#include <rtl/ustring>
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 02.10.00 10:57:48  fs
 ************************************************************************/

